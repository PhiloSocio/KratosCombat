# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
project(KratosCombat VERSION 2.0.8.9 LANGUAGES CXX)

set(FULL_PROJECT_NAME "Kratos Combat" CACHE STRING "Full project name")
set(ADD_COMMONLIBSSE_PLUGIN_AUTHOR "AnArchos" CACHE STRING "Plugin author")
set(ADD_COMMONLIBSSE_PLUGIN_EMAIL "patreon.com/AnArchos" CACHE STRING "Plugin email")
set(commonlibSSE_min_skse_version "2.0.0.2" CACHE STRING "Minimum SKSE version")


if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${FULL_PROJECT_NAME}")
endif()

if(DEFINED ENV{CommonLibSSEPath})
    set(CommonLibSSEPath "$ENV{CommonLibSSEPath}")
else()
    message(FATAL_ERROR "CommonLibSSEPath could not find!")
endif()

message(STATUS "CommonLibSSE path: ${CommonLibSSEPath}")

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)

add_subdirectory("${CommonLibSSEPath}" commonlibsse_build)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h" "src/*.hpp" "include/*.h" "include/*.hpp")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(xbyak CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(rapidcsv CONFIG REQUIRED)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
        nlohmann_json::nlohmann_json
        fmt::fmt
        spdlog::spdlog
        xbyak::xbyak
        Microsoft::DirectXTK
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    AUTHOR="${ADD_COMMONLIBSSE_PLUGIN_AUTHOR}"
    EMAIL="${ADD_COMMONLIBSSE_PLUGIN_EMAIL}"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    lib/glm
    ${SIMPLEINI_INCLUDE_DIRS}
)
# When your SKSE .dll is compiled, this will automatically copy the .dll into your mods folder.
# Only works if you configure DEPLOY_ROOT above (or set the SKYRIM_MODS_FOLDER environment variable)
if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    file(TO_CMAKE_PATH "${OUTPUT_FOLDER}" SANITIZED_OUTPUT_FOLDER)
    set(DLL_FOLDER "${SANITIZED_OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:RelWithDebInfo>:/OPT:REF>
    )
endif()