#include "AntiDetection.h"

#include <algorithm>
#include <memory>
#include <vector>

// Stored module links
std::vector<UNLINKED_MODULE> UnlinkedModules;
// Stored PE Header
std::unique_ptr<std::byte[]> peHeader = nullptr;
// Stored PE Header Size
DWORD peHeaderSize = NULL;


HMODULE GetSelfModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;

	return VirtualQuery(GetSelfModuleHandle, &mbi, sizeof mbi) != 0
		       ? static_cast<HMODULE>(mbi.AllocationBase) : nullptr;
}

bool AntiDetection::RemovePeHeader(const HMODULE hModule) noexcept
{
	if (peHeader)
		return false;

	// Get DOS Header
	const auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
	// Get NT Header
	const auto pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<PBYTE>(pDosHeader) + pDosHeader->e_lfanew);

	// Check if NT Header is correct
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return false;

	// Return value
	bool result = false;

	// Check if Header exists
	if (pNTHeader->FileHeader.SizeOfOptionalHeader)
	{
		// Store VirtualProtect flags
		DWORD dwMemProtect;
		// Get PE Header size
		peHeaderSize = pNTHeader->FileHeader.SizeOfOptionalHeader;
		// Disable memory protect
		if (VirtualProtect(hModule, peHeaderSize, PAGE_EXECUTE_READWRITE, &dwMemProtect))
		{
			// Allocate memory to PE Header backup
			peHeader = std::make_unique<std::byte[]>(peHeaderSize);
			// Backup PE Header
			if (std::memcpy(peHeader.get(), hModule, peHeaderSize) == hModule)
				// Erease PE Header
				if (SecureZeroMemory(hModule, peHeaderSize))
					result = true;
			// Restore memory protect
			if (!VirtualProtect(hModule, peHeaderSize, dwMemProtect, &dwMemProtect))
				result = false;
		}
	}
	return result;
}

bool AntiDetection::RestorePeHeader(const HMODULE hModule)
{
	if (!peHeader)
		return false;

	// Return value
	bool result = false;
	
	// Store VirtualProtect flags
	DWORD dwMemProtect = NULL;
	// Disable memory protect
	if (VirtualProtect(hModule, peHeaderSize, PAGE_EXECUTE_READWRITE, &dwMemProtect)) {
		// Restore PE Header
		if (std::memcpy(hModule, peHeader.get(), peHeaderSize) == hModule)
			result = true;
		// Restore memory protect
		if (!VirtualProtect(hModule, peHeaderSize, dwMemProtect, &dwMemProtect))
			result = false;
	}
	
	return result;
}

bool AntiDetection::UnlinkModule(const HMODULE hModule)
{
	// Check if module already unlinked
	if (const auto it = std::ranges::find(UnlinkedModules, hModule, &UNLINKED_MODULE::hModule);
		it != UnlinkedModules.end())
		return false;

	// Get ProcessEnvironmentBlock
	const auto pPEB = reinterpret_cast<PPEB>(
#ifdef _WIN64
		__readgsqword(
#else
			__readfsdword(
#endif
				FIELD_OFFSET(TEB, ProcessEnvironmentBlock)
			));

	// Cast PEB Loader Data as custom (PPEB_LDR_DATA_FULL) struct
	auto pLDR = reinterpret_cast<PPEB_LDR_DATA_FULL>(pPEB->Ldr);
	PLIST_ENTRY CurrentEntry = pLDR->InLoadOrderModuleList.Flink;
	// Find entry with needed hModule
	while (CurrentEntry != &pLDR->InLoadOrderModuleList && CurrentEntry != nullptr)
	{
		if (const auto Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY_FULL, InLoadOrderLinks);
			Current->DllBase == hModule)
		{
			// Backup current module state
			const UNLINKED_MODULE CurrentModule{
				hModule,
				Current->InLoadOrderLinks.Blink->Flink,
				Current->InMemoryOrderLinks.Blink->Flink,
				Current->InInitializationOrderLinks.Blink->Flink,
				Current
			};
			// Push backup to vector
			UnlinkedModules.push_back(CurrentModule);

			// Unlink all
			UnlinkLink(&Current->InLoadOrderLinks);
			UnlinkLink(&Current->InMemoryOrderLinks);
			UnlinkLink(&Current->InInitializationOrderLinks);

			return true;
		}

		CurrentEntry = CurrentEntry->Flink;
	}

	return false;
}

bool AntiDetection::RelinkModule(const HMODULE hModule)
{
	// Check if module already linked
	const auto it = std::ranges::find(UnlinkedModules, hModule, &UNLINKED_MODULE::hModule);
	if (it == UnlinkedModules.end())
		return false;

	// Relink all from backup
	const auto& m = it;
	RelinkLink(m->RealInLoadOrderLinks, &m->LdrDataTableEntry->InLoadOrderLinks);
	RelinkLink(m->RealInMemoryOrderLinks, &m->LdrDataTableEntry->InMemoryOrderLinks);
	RelinkLink(m->RealInInitializationOrderLinks, &m->LdrDataTableEntry->InInitializationOrderLinks);
	// Delete backup
	UnlinkedModules.erase(it);
	
	return true;
}
