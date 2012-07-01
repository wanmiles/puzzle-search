/**
 * Copyright Ken Anderson, 2010-2012
 */

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

