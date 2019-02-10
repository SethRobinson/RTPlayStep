cmake_minimum_required(VERSION 2.6)

ENABLE_LANGUAGE(C)

include(CheckIncludeFile)
set (CMAKE_REQUIRED_INCLUDES "/opt/vc/include")
CHECK_INCLUDE_FILE(bcm_host.h ISRASPBERRYPIE)

if(ISRASPBERRYPIE)
OPTION(RASPBERRYPI_GLES11 "Compile for Raspberry PI GLES" ON)
message(STATUS "Detected as PI")
add_definitions(-DRT_RASPBERRY)
endif(ISRASPBERRYPIE)

#// -DRT_CONSOLE_SDL
add_definitions(-DRT_LINUX)

set(CMAKE_BUILD_TYPE "Release") #use Release or Debug

project(RTPlayStep)
include(../../shared/linux/Proton.cmake)

proton_include_all_components()
#proton_include_components(FocusInputComponent)
#proton_include_components(CustomInputComponent ArcadeInputComponent)
proton_use_sdl_audio()
set(APP "../source")
proton_set_sources("${APP}/App.cpp" "${APP}/GameLogicComponent.cpp" "${APP}/BlockerManager.cpp" "../../shared/Manager/WiringPiManager.cpp")
target_link_libraries(RTPlayStep wiringPi wiringPiDev)
