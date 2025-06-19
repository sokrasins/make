# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(wieg_sig_gen C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(wieg_sig_gen
    emulator.c 
    src/wiegand.c
    src/cmd.c
)

pico_set_program_name(wieg_sig_gen "wieg_sig_gen")
pico_set_program_version(wieg_sig_gen "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(wieg_sig_gen 1)
pico_enable_stdio_usb(wieg_sig_gen 0)

# Add the standard library to the build
target_link_libraries(wieg_sig_gen
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(wieg_sig_gen PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Add any user requested libraries
target_link_libraries(wieg_sig_gen 
        hardware_timer
        )

pico_add_extra_outputs(wieg_sig_gen)

