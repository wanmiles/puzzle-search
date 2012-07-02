##
# Copyright (c) 2010-2012, Ken Anderson
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##

# Compiler settings
set (CMAKE_CXX_FLAGS "-g -Wall")
#set (CMAKE_CXX_FLAGS "-Wall -O3")

# include_directories ("$PROJECT_COURCE_DIR")
#include_directories (src)

#add_library (slidingTile src/slidingTile.hpp src/slidingTile.h src/common.h src/search.h src/transTable.h src/search.h)
add_executable (../Search main.cpp
                domain.h slidingTile.h slidingTile.hpp
                kpancake.h kpancake.hpp
                common.h
                log.h
                searchState.h
                search.h search.hpp
                transTable.h transTable.hpp
                perimeterDB.h perimeterDB.hpp
                search.h)
#target_link_libraries(Search)

#add_definitions (-DINPUT_FILE)

#add_custom_command (OUTPUT run
#                    DEPENDS Search
#                    COMMAND "./Search" )
