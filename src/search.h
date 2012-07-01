/**
 * Copyright Ken Anderson, 2010-2011
 */

#ifndef SEARH_H
#define SEARH_H

#include "domain.h"
#include "searchState.h"
#include "transTable.h"
#include "perimeterDB.h"
#include "common.h"

// This class is currently only intended to fill the PerimeterDB.
// Might be extended later for more general purpose.
class DFS
{
public:
  long long generationCount;
  //SearchState m_goal;
  //SearchState m_start;
  PerimeterDb & perimeterDb;

public:
  DFS(PerimeterDb & _perimeterDb);

  // Main search function
  // Searches from the goal outwards
  void search(const SearchState & goal, const int & depth);
  long long getNodesGenerated() { return generationCount; }

private:
  // Used to create perimeter DB
  void dfsRecursive( SearchState & state, const int & costLimit, const int & iteration );
  // returns true if the state should be pruned off the search tree
  bool prune( const SearchState & state, const int & costLimit, const int & iteration ) ;
};

class IDA
{
public:
  long long generationCount;
  SearchState m_goal;
  SearchState m_start;

#ifdef USE_TRANS_TABLE
  TransTable transTable;
#endif
#ifdef USE_PERIMETER_DB
  PerimeterDb & perimeterDb;
#endif

public:
#ifdef USE_PERIMETER_DB
  IDA(PerimeterDb & _perimeterDb) : perimeterDb(_perimeterDb) {}
#else
  IDA() {}
#endif
  ~IDA() {}

  // Main search function
  int search(const SearchState & start, const SearchState & goal);
  long long getNodesGenerated() { return generationCount; }

private:
  // prune the node if necessary, and update tables if necessary.
  // return 0 if
  // 1) not over the depth bound
  // 2) not in transposition table and already visited with smaller (or equal) g-value on this iteration.
  PruneStatus prune( const SearchState & state, const int & costLimit, const int & heuristic ) ;//const;

  int getHeuristic(const SearchState & state) const;
  void checkHeuristic(const SearchState & state, const int & heur);

  // returns 0 if found a solution
  // returns 1 if all children (or children's children) are in the TT
  // returns 2 if a child (or child's child) is a leaf node.
  NodeStatus idaRecursive( SearchState & state, const int & costLimit, int & prevHeuristic );

  // lookahead past the frontier in the hopes that we can propogate a high heuristic backwards
  NodeStatus lookaheadRecursive( SearchState & state, const int & costLimit, int & prevHeuristic );
};


#include "search.hpp"


#endif //SEARH_H
