/**
 * Copyright Ken Anderson, 2010-2012
 */

#ifdef USE_TRANS_TABLE

#ifndef TRANS_TABLE_H
#define TRANS_TABLE_H

#include "common.h"
#include "domain.h"

// TODO - could reduce the entry size if we only store something like (int)(state)%TT_SIZE


class TransTableEntry {
public:
  // Careful with the ordering!
  State   	    state;
  int           cost;
#ifdef USE_TRANS_TABLE_HEUR_CACHING
  Heuristic	    heuristic;
#endif
#ifdef USE_LAZY_TRANS_TABLE
  int           costLimit;
#endif
#ifdef USE_TRANS_TABLE_STATE_PRIORITIZATION
  unsigned int  priority;
#endif

public:
  TransTableEntry();
  void print(LogLevel level) const;
  bool updateEntry(const int & cost, const int & costLimit);
};


class TransTable
{
private:
  TransTableEntry* transTable;

public:
  TransTable() { transTable = new TransTableEntry[TT_SIZE]; }
  ~TransTable() { delete[] transTable; }
  void reset();

  // Adds or updates the state in the trans table,
  // and returns true if the node already exists and should be pruned from the search tree.
  // returns false if the state must be expanded.
  bool pruneState( const State & state, const Hash & hash, const int & heur, const int & cost, const int & costLimit );

  // Returns the cached heuristic value, if the state exists in the table
  // returns 0 otherwise
 // int getCachedHeuristic( const State & state, const Hash & hash ) const;
  int getCachedHeuristic( const State & state, const Hash & hash) const;
  // updates the cached heuristic value if it is large enough
  void updateCachedHeuristic( const State & state, const Hash & hash, const int & heuristic ) const;

  // Stats
  void print(LogLevel level) const;
  void printInfo(LogLevel level) const;
  double percentFull( ) const;

private:
  TransTableEntry &getEntry(const unsigned index);
  long long numEntries( ) const;
  unsigned int calculateIndex( const Hash & hash ) const;
};

// Inline function definintions
#include "transTable.hpp"

#endif	// TRANS_TABLE_H
#endif	// USE_TRANS_TABLE
