/**
 * Copyright Ken Anderson, 2010-2012
 */
/**
 * Frequently changing constants.
 */

#ifndef COMMON_H
#define COMMON_H

#include "log.h"

/////////////////////////////////
// CONSTANTS ////////////////////
/////////////////////////////////

const int SEED = 0;
const int NUM_RANDOM_STEPS = 100;
const int MAX_COST = 150;

/////////////////////////////////
// DOMAIN ///////////////////////
/////////////////////////////////

// 1=slidingTile, 2=kpancake
#define DOMAIN 1

/////////////////////////////////
// OPTIONS //////////////////////
/////////////////////////////////

// Scan through all the nodes on the perimeter and expand them to a depth
// This introduces suboptimality in order to get deeper, high-priority states faster
//#define USE_PERIMETER_SCAN_EXPANSION

// Do a DFS search at the leaf nodes in order to try and propogate backwards a good heuristic value
//#define USE_LOOKAHEAD

// Each state gets a priority
// Used for the replacement policy of trans table and perimeter db
#define USE_PERIMETER_STATE_PRIORITIZATION
//#define USE_TRANS_TABLE_STATE_PRIORITIZATION

/////////////////////////////////
// SEARCH OPTIMIZATIONS /////////
/////////////////////////////////

// This is used to eliminate transpositions caused by reverse operator
// This will slightly change the TT and perimeterDb entries because the order of expansion will change slightly.
#define USE_SKIP_TRANS_OP

/////////////////////////////////
// PERIMETER DB /////////////////
/////////////////////////////////

#define USE_PERIMETER_DB
#define PERIMETER_DEPTH		8	// 24 ~= 40 seconds, 30 = long
#define PERIMETER_DB_SIZE 100 // 1000007
#define USE_LAZY_PERIMETER

/////////////////////////////////
// TRANSPOSITION TABLES /////////
/////////////////////////////////

// The transposition table reduces cycles caused by node re-expansions
// Number of nodes-per-sec should be directly dependent 
// on the size of the table.
// Total number of nodes generated should be inversely dependent 
// on the size of the table.
#define USE_TRANS_TABLE

#define USE_TRANS_TABLE_HEUR_CACHING

// Do NOT use 2^x, use a prime number
//const int TT_SIZE = 40000007;	// large
//const int TT_SIZE = 5000007;	// medium-large
//const int TT_SIZE = 200007;	// medium
//const int TT_SIZE = 10007;	// medium-small
const int TT_SIZE = 107;		// small
//const int TT_SIZE = 15;		// small

// When using a trans table, lazy evaluation can be used 
// to avoid clearing the table every time.
// This also allows for less node expansions.
#define USE_LAZY_TRANS_TABLE

// The new search technique relies on having a transposition table.
// We can scan through the transposition table to restart the search iterations
// instead of starting from the start node each time.
// Note that we are currently requiring LAZY trans table help differentiate
// Which nodes are leaf nodes (using lastIteration).
// Also-- we cannot clear the TT in between iterations
// NOTE-- The (1) number of nodes expanded/ & 
// (2) TT fill will be different 
// because the order of expansion is different.  
//  (the second one is actually quite tricky).
// FIXME - doesn't work for heuristics.
//#define USE_TT_SCAN_EXPANSION


/////////////////////////////////
// HASH /////////////////////////
/////////////////////////////////

#if defined USE_TRANS_TABLE || defined USE_PERIMETER_DB
  #define USE_HASH
#endif

// This is used to turn on incremental hashing (should be on by default)
#define USE_INCREMENTAL_HASH

/////////////////////////////////
// HEURISTIC ////////////////////
/////////////////////////////////

// If heuristic is not used, then 0 is the heuristic
// And the search acts like DFS
// Note that the MD heuristic is incremental
//#define USE_HEURISTIC
#define USE_INCREMENTAL_HEURISTIC
#define USE_BPMX


#endif

