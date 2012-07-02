/**
 * Copyright (c) 2010-2012, Ken Anderson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

// FIXME - avoiding reverse operators does not work!!
// FIXME - doesn't check for solution at start of search
// TODO - PDB
// FIXME - optimize the hash function

#include "domain.h"
#include "search.h"
#include "perimeterDB.h"
#include <vector>
#include <fstream>

// Declarations
void initializeStartStates(std::vector<SearchState> & states);

// Main
int main( int argc, const char* argv[]  )
{
  double avgLength = 0.0;
  double avgNodesGen = 0.0;
  int numSearches = 100;

  SearchState goal;
  std::vector<SearchState> startingStates;
  initializeStartStates(startingStates);
  LOG_ERROR("LogLevel =%i\n", g_logLevel);

  // Preprocess the state space
#ifdef USE_PERIMETER_DB
  LOG_ERROR("PerimeterDepth =%i\n", PERIMETER_DEPTH);
  PerimeterDb perimeterDb;
  DFS dfs(perimeterDb);
  dfs.search(goal, PERIMETER_DEPTH);
  perimeterDb.printInfo(ERROR);
  LOG("\n");
#endif

  // Search algorithm
#ifdef USE_PERIMETER_DB
  IDA idaSearch(perimeterDb);
#else
  IDA idaSearch;
#endif

	// Search
  for( int i=0; i<numSearches; i++)
  {
    SearchState & state = startingStates[i];	// copy

    LOG("   start=");
    state.state.print(NORMAL);
    LOG("\n   goal =");
    goal.state.print(NORMAL);
    LOG("\n");

    printTime(WARN);
    int solutionLength = idaSearch.search(state, goal);
    long long nodesGenerated = idaSearch.getNodesGenerated();
    LOG_WARN("SolutionNumber %i Solution length %i Nodes Generated %lli\n", i, solutionLength, nodesGenerated);

    avgLength += solutionLength;
    avgNodesGen += nodesGenerated;

#ifdef USE_PERIMETER_DB
    idaSearch.perimeterDb.printInfo(ERROR);
#endif
#ifdef USE_TRANS_TABLE
    idaSearch.transTable.printInfo(ERROR);
#endif
  }

  LOG_ERROR("\n");
  LOG_ERROR(" avgSolLength %f avgNodesGenerated %f numSearches %i \n", avgLength/numSearches, avgNodesGen/numSearches, numSearches);

	return 0;
}

 void initializeStartStates(std::vector<SearchState> & states)
{
#ifdef INPUT_FILE
  SearchState state;
  LOG("Using input from %s\n", INPUT_FILE);
  std::ifstream input(INPUT_FILE);
  std::string str;
  if(input.is_open())
  {
    while(getline(input,str))
    {
      state.load( str.c_str() );
      //state.print();
      states.push_back(state);
    }
    input.close();
  }

#else
  // Random init
  LOG("Using Random input");
  srand( SEED );
  //state.state = goal;
  //state.init();
  for(int i=0; i<100; i++)
  {
    SearchState state;
    state.init();
    state.randomize( NUM_RANDOM_STEPS );
    state.cost = 0;
#ifdef USE_SKIP_TRANS_OP
    state.prevOp = NO_OP;
#endif
    states.push_back(state);
  }

#endif

}

