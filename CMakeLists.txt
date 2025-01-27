cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Set Preprocessor Defines
add_compile_definitions(LELY_NO_THREADS) 
add_compile_definitions(LELY_NO_DAEMON) 
add_compile_definitions(USE_NUCLEO_64)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME lely-cia402-tester)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Füge das Include-Verzeichnis für "LibraryA" hinzu, damit #include <LibraryA/...> funktioniert
include_directories(
${CMAKE_SOURCE_DIR}/ThirdParty/Include
${CMAKE_SOURCE_DIR}/Libraries/Include
${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Core/Include/
${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include/
${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/
${CMAKE_SOURCE_DIR}/Drivers/BSP/STM32G4xx_Nucleo/
${CMAKE_SOURCE_DIR}/Core/Inc/
)

# Finde alle Quell-Dateien rekursiv im Source-Verzeichnis
file(GLOB_RECURSE LIBRARY_SOURCES 
    ${CMAKE_SOURCE_DIR}/ThirdParty/Source/lely/*.c
    ${CMAKE_SOURCE_DIR}/ThirdParty/Source/lcd/*.c
    ${CMAKE_SOURCE_DIR}/ThirdParty/Source/ugui/*.c
)

add_library(lely-core STATIC ${LIBRARY_SOURCES})

file(GLOB_RECURSE APP_LIBRARY_SOURCES 
    ${CMAKE_SOURCE_DIR}/Libraries/Source/ui/*.c
    ${CMAKE_SOURCE_DIR}/Libraries/Source/cia_402/*.c
)

add_library(app-core STATIC ${APP_LIBRARY_SOURCES})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths

)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    lely-core
    app-core
    # Add user defined libraries
)