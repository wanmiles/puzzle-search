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
