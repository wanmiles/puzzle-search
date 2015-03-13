# Puzzle Search #

This project uses heuristic search to find optimal solutions for puzzle domains.  The implemented techniques are as follows:

Domains:
  * Sliding tile puzzle
  * Pancake puzzle

Search Algorithms:
  * IDA`*`
  * IDA`*` with BPMX (bidirectional pathmax)

Heuristics:
  * Manhattan Distance (sliding tile puzzle only)
  * Gap Heuristic (pancake puzzle only)
  * Incremental Heuristics (both MD and Gap)
  * Perimeter Database

Various optimizations:
  * Reverse operator removal.
  * Transposition Table

# Dependencies #
  * git
  * cmake 2.8

# Compilation #

## Windows using Cygwin ##

  * Install cygwin and the gcc and the cmake packages for cygwin.
  * Checkout code
  * Open a cygwin terminal & go to code.
  * cd puzzle-search/build
  * cmake ..
  * make
  * ./Search.exe

## Visual Studio ##

Similar to above.  If visual studio is installed, use cmake to generate the Visual Studio project file instead.

## Linux ##

Very similar to above.