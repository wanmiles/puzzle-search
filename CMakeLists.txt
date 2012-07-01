# Cmake params
cmake_minimum_required (VERSION 2.4.6)

# Windows/cygwin-specific
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

# Project info
project (Search)
set (Search_VERSION_MAJOR_0)
set (Search_VERSION_MINOR_1)


# Compiler files
add_subdirectory(src)
add_subdirectory(input)

add_custom_command (OUTPUT run
                    DEPENDS Search
                    COMMAND "./Search" )
