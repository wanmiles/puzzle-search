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
