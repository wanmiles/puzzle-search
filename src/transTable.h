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
