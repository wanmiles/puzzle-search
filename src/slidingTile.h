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

#ifndef SLIDING_TILE_H
#define SLIDING_TILE_H

#include "common.h"
#include <string>

class Hash;
class Heuristic;

// Use this option to speed up operation generation
// The valid operators are generated via a lookup table instead of programatically.
// This should be faster.
#define OP_LOOKUP_TABLE

// Use the input file for the start states
//#define INPUT_FILE "../input/korf_100.txt"

// State-specific constants
static const int MAX_NUM_OPS = 4;
static const int WIDTH =3;
static const int HEIGHT=3;
static const int NUM_TILES=WIDTH*HEIGHT;
typedef unsigned int tile_t;

// operators are ordered for efficiency (independent of goal state)
enum Operator
{
  NO_OP=0,	// used no denote the prev op for the first state
  OP_RIGHT,
  OP_LEFT,
  OP_UP,
  OP_DOWN,
};
const int num_operators = (int)MAX_NUM_OPS+1;
inline Operator const reverse( const Operator & op );
struct OpList
{
  Operator	ops[MAX_NUM_OPS];
  int				length;

  void print(LogLevel level) const;
};
class OpLookupTable
{
  OpList operatorTable[NUM_TILES][num_operators];
public:
  OpLookupTable();
  const OpList getValidOperators( const int & blankLoc, const Operator & prevOp );
private:
  const OpList _getValidOperators( const int & blankLoc, const Operator & prevOp );
};


// State keeps track of the state within the world/environment/domain
// For example- puzzle permutation, grid location on a map, joint angles on a robot, etc.
class State
{
private:
  // Lookup tables for the valid operations given the blank location
  static OpLookupTable operatorTable;

public:
  tile_t tiles[NUM_TILES];
  unsigned int  blankLocation;
public:
  bool operator==( const State & state2 ) const;
  void init();
  void load( const char* str );
  void print(LogLevel level) const;
  const OpList findSuccessorOperators( const Operator & prevOp ) const;
  const OpList findPredecessorOperators( const Operator & prevOp ) const;
  // Pass in Heuristic and/or hash, if they exist.
  int  apply( const Operator & op, Heuristic * heuristic, Hash * hash );
private:
  int getNewBlankLoc(const Operator & op) const;
};
OpLookupTable State::operatorTable;


// Manhattan Distance Heuristic
class Heuristic
{
public:
  int value;
private:
  static tile_t mdTable[NUM_TILES][NUM_TILES];
  static bool tableInitialized;
  
public:
  Heuristic();
  void calculateHeuristic(const State& state);
  void print(LogLevel level) const;

private:
  friend class State;
  void incrementHeuristic( const State& state, const int & oldBlankLoc );

  // Lookup table for manhattan distance between any two tiles
  void initTable();
  void printTable(LogLevel level) const;
};

class Hash
{
public:
  unsigned int value;
  
public:
  Hash() { if(!tableInitialized) initTable(); }
  void calculateHash(const State & state);
  void print(LogLevel level) const;
private:
  friend class State;
  void incrementHash( const State & state, const int & oldBlankLoc );

  // Lookup table for the incremental hash difference
  void initTable();
  void printTable(LogLevel level) const;
  static const int HASHSEED = 1;			//0xA7F3C07B;
  static unsigned int hashTable[NUM_TILES][NUM_TILES];
  static bool tableInitialized;
};

// FIXME -- move to an appropriate location
// FIXME -- not very random
static unsigned int getPriority(const Hash & hash)
{
  // Convert to 32 bit, even if int is 63 bit
  unsigned int priority = 0xFFFFFFFF / ( hash.value & 0xFFFFFFFF );
  //priority &= 0x55555555;
  return priority;
}


/////////////////////////////////////// INLINE DEFINITIONS ///////////////////////////////////////////////////

#include "slidingTile.hpp"

#endif
