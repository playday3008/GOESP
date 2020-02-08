project(GOESP CXX)

################################################################################
# Source groups
################################################################################
set(Hacks
    "Hacks/ESP.cpp"
    "Hacks/ESP.h"
    "Hacks/Misc.cpp"
    "Hacks/Misc.h"
)
source_group("Hacks" FILES ${Hacks})

set(Header_Files
    "Config.h"
    "fnv.h"
    "GUI.h"
    "Helpers.h"
    "Hooks.h"
    "ImGuiCustom.h"
    "Interfaces.h"
    "Memory.h"
    "resource.h"
)
source_group("Header Files" FILES ${Header_Files})

set(SDK
    "SDK/ClassId.h"
    "SDK/Client.h"
    "SDK/ClientClass.h"
    "SDK/ConVar.h"
    "SDK/Cvar.h"
    "SDK/Engine.h"
    "SDK/EngineTrace.h"
    "SDK/Entity.h"
    "SDK/EntityList.h"
    "SDK/GlobalVars.h"
    "SDK/InputSystem.h"
    "SDK/Localize.h"
    "SDK/Matrix3x4.h"
    "SDK/Vector.h"
    "SDK/VirtualMethod.h"
    "SDK/WeaponId.h"
    "SDK/WeaponInfo.h"
)
source_group("SDK" FILES ${SDK})

set(Source_Files
    "Config.cpp"
    "dllmain.cpp"
    "GUI.cpp"
    "Hooks.cpp"
    "ImGuiCustom.cpp"
    "Memory.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(imgui
    "imgui/imconfig.h"
    "imgui/imgui.cpp"
    "imgui/imgui.h"
    "imgui/imgui_demo.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_impl_dx9.cpp"
    "imgui/imgui_impl_dx9.h"
    "imgui/imgui_impl_win32.cpp"
    "imgui/imgui_impl_win32.h"
    "imgui/imgui_internal.h"
    "imgui/imgui_widgets.cpp"
    "imgui/imstb_rectpack.h"
    "imgui/imstb_textedit.h"
    "imgui/imstb_truetype.h"
)
source_group("imgui" FILES ${imgui})

set(nlohmann
    "nlohmann/json.hpp"
)
source_group("nlohmann" FILES ${nlohmann})

set(ALL_FILES
    ${Hacks}
    ${Header_Files}
    ${SDK}
    ${Source_Files}
    ${imgui}
    ${nlohmann}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE GOESP)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "_HAS_EXCEPTIONS=0;"
        "NDEBUG"
    ">"
    "_CRT_SECURE_NO_WARNINGS;"
    "_USE_MATH_DEFINES;"
    "WIN32;"
    "GOESP_EXPORTS;"
    "_WINDOWS;"
    "_USRDLL;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od;
            /sdl;
            ${DEFAULT_CXX_DEBUG_RUNTIME_LIBRARY};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        >
        $<$<CONFIG:Release>:
            /O2;
            /Ob2;
            /Oi;
            /GF;
            /GT;
            /arch:SSE2;
            /Oy;
            ${DEFAULT_CXX_RUNTIME_LIBRARY};
            /Gy;
            /fp:except-;
            /Ot;
            /fp:fast;
            /GS-
        >
        /permissive-;
        /std:c++latest;
        /Gr;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        /GR-;
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /DEBUG;
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /MANIFEST:NO;
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO;
            /EMITPOGOPHASEINFO
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

