/**
 * Copyright Ken Anderson, 2011
 */

#ifndef PERIMETER_DB_H
#define PERIMETER_DB_H

#include "common.h"
#include "domain.h"

class PerimeterDbEntry
{
public:
  State   		  state;
  int 					cost;
#ifdef USE_LAZY_PERIMETER
  int  					iteration;	// The search iteration the node was last expanded on
#endif
#ifdef USE_PERIMETER_STATE_PRIORITIZATION
  unsigned int	priority;
#endif

public:
  PerimeterDbEntry();
  ~PerimeterDbEntry();
  bool updateEntry(const int & cost, const int & iteration);
  void print(LogLevel level) const;
};

class PerimeterDb
{
private:
  PerimeterDbEntry * perimeterDb;
  double avgDepth;

public:
  PerimeterDb() { perimeterDb = new PerimeterDbEntry[PERIMETER_DB_SIZE];}
  ~PerimeterDb() { delete[] perimeterDb; }
  void reset();

  // Adds or updates the state in the trans table,
  // and returns true if the node already exists and should be pruned from the search tree.
  // returns false if the state must be expanded.
  bool pruneState( const State & state, const Hash & hash, const int cost, const int iteration );	// TODO: reference
  // Returns the cost to the goal state, if the state exists in the perimeterDb
  // returns 0 otherwise
  int getHeuristic( const State & state, const Hash & hash ) const;
  // return true if a state exists at this index
  // if true, set the state and the cost
  PerimeterDbEntry * getState( const unsigned & index );

  // Stats
  void print(LogLevel level) const;
  void printInfo(LogLevel level) const;
  void printHistogram(LogLevel level, const unsigned int depth) const;
  double getAvgDepth() const;

private:
  unsigned int calculateIndex( const Hash & hash ) const;
  void calculate(long long & numEntries, double & avgDepth, double & percentFull) const;
};

#include "perimeterDB.hpp"

#endif
