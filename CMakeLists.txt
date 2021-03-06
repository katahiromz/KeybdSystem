# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# project name and language
project(KeybdSystem CXX)

# check build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# make uppercase string of build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

# check build type again
if (NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if (WIN32)
    # enable Win32 resource
    enable_language(RC)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # using Clang
        set(CMAKE_C_FLAGS "-static -s")
        set(CMAKE_CXX_FLAGS "-static -s")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        set(CMAKE_C_FLAGS "-static -static-libgcc -static-libstdc++ -s")
        set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++ -s")
    elseif (MSVC)
        # replace "/MD" with "/MT" (building without runtime DLLs)
        set(CompilerFlags
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        foreach(CompilerFlags ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
        endforeach()
    endif()
endif()

##############################################################################

subdirs(
    KeybdSystem
    EasyEn
    StdEn
    FullEn
    SimpleJP
    EasyJP
    StdJP
    FullJP
    SimpleNumPad)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_target(
    deploy ALL
    DEPENDS
            $<TARGET_FILE:KeybdSystem>
            $<TARGET_FILE:EasyEn>
            $<TARGET_FILE:StdEn>
            $<TARGET_FILE:FullEn>
            $<TARGET_FILE:SimpleJP>
            $<TARGET_FILE:EasyJP>
            $<TARGET_FILE:StdJP>
            $<TARGET_FILE:FullJP>
            $<TARGET_FILE:SimpleNumPad>
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:KeybdSystem>
            $<TARGET_FILE:EasyEn>
            $<TARGET_FILE:StdEn>
            $<TARGET_FILE:FullEn>
            $<TARGET_FILE:SimpleJP>
            $<TARGET_FILE:EasyJP>
            $<TARGET_FILE:StdJP>
            $<TARGET_FILE:FullJP>
            $<TARGET_FILE:SimpleNumPad>
            ${CMAKE_BINARY_DIR}/bin)

##############################################################################
