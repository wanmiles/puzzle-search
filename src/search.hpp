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

#include <time.h>

////////////////////////////////
// IDA star search /////////////
////////////////////////////////

inline int IDA::getHeuristic(const SearchState & state) const
{
  int returnVal = 0;
#ifdef USE_HEURISTIC
  returnVal = state.incHeuristic.value;
#endif
#ifdef USE_PERIMETER_DB
  const int perimeterHeuristicVal = this->perimeterDb.getHeuristic(state.state, state.hash);
/*  if( perimeterHeuristicVal > state.incHeuristic.value )
  {
    LOG("in perimeter: ");
    state.print();
    LOG(" md=%i perim=%i\n", state.incHeuristic.value, perimeterHeuristicVal );
  }
*/
  returnVal = std::max( returnVal, perimeterHeuristicVal );
#endif
#if defined USE_TRANS_TABLE && defined USE_TRANS_TABLE_HEUR_CACHING
  const int cachedHeuristicVal = this->transTable.getCachedHeuristic(state.state, state.hash);
  returnVal = std::max(returnVal, cachedHeuristicVal);
#endif

  return returnVal;
}

inline void IDA::checkHeuristic(const SearchState & state, const int & heur)
{
#if defined USE_TRANS_TABLE && defined USE_TRANS_TABLE_HEUR_CACHING
  //const int transTableCachedHeurVal = this->transTable.getCachedHeuristic(state.state, state.hash);
  this->transTable.updateCachedHeuristic(state.state, state.hash, heur );
  //transTableCachedHeurVal = std::max( heur, transTableCachedHeurVal, heur );
#endif
}

inline PruneStatus IDA::prune(
  const SearchState & state,
  const int & costLimit,
  const int & heur
  ) //const
{
  //const int heur = 0;

  if( state.cost + heur > costLimit )
  {
    return NODE_PRUNED_BY_COST;
  }

#ifdef USE_TRANS_TABLE
  if( transTable.pruneState(state.state, state.hash, heur, state.cost, costLimit) )
  {
    return NODE_PRUNED_BY_TT;
  }
  else
  {
    return NODE_NEEDS_EXPANSION;
  }
/*  TransTableEntry * entry = NULL;
  (entry) = transTable.LookupEntry( state.state, state.hash );
  if( !(entry) )
  {
    return NODE_NEEDS_EXPANSION;
  }

  if( !(entry)->UpdateEntry( cost, costLimit ) )
  {
    return NODE_PRUNED_BY_TT;
  }*/
#endif

  return NODE_NEEDS_EXPANSION;
}

void indent(LogLevel level, int num)
{
  for(int i=0; i<num; ++i)
    _LOG(level," ");
}

NodeStatus IDA::idaRecursive( SearchState & state, const int & costLimit, int & prevHeuristic )
{
  generationCount++;

  // find heuristic
  int heuristic = getHeuristic(state);
#ifdef USE_BPMX
  heuristic = std::max(prevHeuristic-1,heuristic);
  prevHeuristic = std::max(prevHeuristic, heuristic-1);
#endif
#if defined USE_TRANS_TABLE && defined USE_TRANS_TABLE_HEUR_CACHING
  checkHeuristic(state, heuristic);
#endif

  // Debugging
  indent(DEBUG,state.cost);
  state.print(DEBUG);
#ifdef USE_HEURISTIC
  LOG_DEBUG(" heur=%i",heuristic);
#endif
  LOG_DEBUG(" costLimit=%i \n",costLimit);

  // Only continue if node needs expansion.
  PruneStatus pruneStatus = prune(state,costLimit,heuristic);
  if( pruneStatus == NODE_PRUNED_BY_TT )
  {
    return SEARCH_NODE_IN_TT;
  }
  else if( pruneStatus == NODE_PRUNED_BY_COST )
  {
#ifdef USE_LOOKAHEAD
    //int heur = heuristic;
    lookaheadRecursive(state,costLimit+3,heuristic);
    //if( heuristic > heur )
    //{
    //  LOG("lookahead improved heuristic value\n");
    //}
#endif
    return SEARCH_SOME_CHILDREN_LEAF;
  }

  // Expanding node
  if( state == m_goal )
  {
    //LOG(" |-- > solution! \n");
    LOG("\n");
    state.print(NORMAL);
    LOG(" \n" );
    return SEARCH_FOUND_SOLUTION;	// Found the solution
  }

  NodeStatus childrenStatus = SEARCH_ALL_CHILDREN_IN_TT;
  const OpList opList = state.findSuccessorOperators();

  // Debug
  indent(DEBUG,state.cost);
  opList.print(DEBUG);
  LOG_DEBUG("\n");

  for( int i=0; i<opList.length; i++ )
  {
    // apply
    state.apply( opList.ops[i] );
    //recurse
    NodeStatus status = idaRecursive( state, costLimit, heuristic );
    // revert
    state.unapply( opList.ops[i] );

    if( status == SEARCH_FOUND_SOLUTION )
    {
      state.print(NORMAL);
      LOG(" op=%i\n", opList.ops[i] );
      return SEARCH_FOUND_SOLUTION;
    } else if ( status == SEARCH_SOME_CHILDREN_LEAF )
    {
      childrenStatus = SEARCH_SOME_CHILDREN_LEAF;
    }

#if defined USE_TRANS_TABLE && defined USE_TRANS_TABLE_HEUR_CACHING
    checkHeuristic(state, heuristic);
#endif
#ifdef USE_BPMX
    if( state.cost + heuristic > costLimit )
    {	// heuristic propogated backwards and caused a parental cutoff
      //LOG(".");
      prevHeuristic = std::max(prevHeuristic, heuristic-1);
      return SEARCH_SOME_CHILDREN_LEAF;
    }
#endif

  }

  return childrenStatus;
}

#ifdef USE_LOOKAHEAD
NodeStatus IDA::lookaheadRecursive( SearchState & state, const int & costLimit, int & prevHeuristic )
{
  generationCount++;
  // Debugging
  //print( state );

  // find heuristic
  int heuristic = getHeuristic(state);
#ifdef USE_BPMX
  heuristic = std::max(prevHeuristic-1,heuristic);
  prevHeuristic = std::max(prevHeuristic, heuristic-1);
#endif

  // Only continue if node needs expansion.
  PruneStatus pruneStatus = prune(state,costLimit,heuristic);
  if( pruneStatus == NODE_PRUNED_BY_TT )
  {
    return SEARCH_NODE_IN_TT;
  }
  else if( pruneStatus == NODE_PRUNED_BY_COST )
  {
    return SEARCH_SOME_CHILDREN_LEAF;
  }

/*
  // Expanding node
  if( state == m_goal )
  {
    return SEARCH_FOUND_SOLUTION;	// Found the solution
  }
*/

  NodeStatus childrenStatus = SEARCH_ALL_CHILDREN_IN_TT;
  int highestPriority = -1;
  Operator highestPriorityOp = NO_OP;

  const OpList opList = state.findSuccessorOperators();
  for( int i=0; i<opList.length; i++ )
  {
    // apply
    state.apply( opList.ops[i] );
    generationCount++;
    // get operation with highest state priority
    int priority = getPriority(state.hash);
    if( priority > highestPriority )
    {
      highestPriority = priority;
      highestPriorityOp = opList.ops[i];
    }
    // revert
    state.unapply( opList.ops[i] );

    /*
    if( status == SEARCH_FOUND_SOLUTION )
    {
      state.print(NORMAL);
      LOG(" op=%i\n", opList.ops[i] );
      return SEARCH_FOUND_SOLUTION;
    } else if ( status == SEARCH_SOME_CHILDREN_LEAF )
    {
      childrenStatus = SEARCH_SOME_CHILDREN_LEAF;
    }
    */
  }

  // Use best one
  state.apply( highestPriorityOp );
  /*NodeStatus status =*/ lookaheadRecursive( state, costLimit, heuristic );
  state.unapply( highestPriorityOp );

#ifdef USE_BPMX
    if( state.cost + heuristic > costLimit )
    {	// heuristic propogated backwards and caused a parental cutoff
      //LOG(".");
      prevHeuristic = std::max(prevHeuristic, heuristic-1);
      return SEARCH_SOME_CHILDREN_LEAF;
    }
#endif

  return childrenStatus;
}
#endif

inline int IDA::search(const SearchState & start, const SearchState & goal)
{
  int depth = 0;
  generationCount = 0;
  clock_t totalClockTicks = 0;
  int oldNodeCount = 0;
  double time;
  m_goal = goal;
  //m_goal.print(NORMAL);
  //LOG("\n");
  NodeStatus status = SEARCH_SOME_CHILDREN_LEAF;

#ifdef USE_TRANS_TABLE
  transTable.reset();
#endif

  SearchState state;
  while( status != SEARCH_FOUND_SOLUTION && depth < MAX_COST )
  {
    state = start;
    depth++;
    oldNodeCount = 0;
#ifdef USE_TRANS_TABLE
#ifndef USE_LAZY_TRANS_TABLE
//#ifndef USE_TT_SCAN_EXPANSION
    transTable.resetTT();
//#endif
#endif
#endif
    clock_t startClock = clock();

#ifndef USE_TT_SCAN_EXPANSION
    int heur = 0;
    status = idaRecursive(state, depth, heur);
#else
/*
    // TODO: clean this up.
    //int startingCost = depth-1;
    if( depth <= 1 ) {
      status = IdaRecursive(state, startingCost, depth);
    } else {
      for(int i=0; i<TT_SIZE; i++)
      {
        TransTableEntry & entry = transTable[i];
        //LOG( "entry.cost=%i, startCost=%i\n", entry.cost, startingCost );

        if( entry.cost != MAX_COST ) // valid node
        {
          copy( state, entry.state ); // Absolutely necessary!
          status = IdaRecursive(state, entry.cost, depth);
        }

        else if( status == SEARCH_FOUND_SOLUTION ) {
          LOG("Not implemented yet. exit(4)\n");
          exit(4);  // Not implemented.
        }
      }
    }
*/
#endif
    totalClockTicks += clock() - startClock;
    time = (double)totalClockTicks/CLOCKS_PER_SEC;
    LOG("depth=%2i genCnt=%13lld time=%6.2fsec nps=%9.f ",
      depth, generationCount,
      time, generationCount/time );
#ifdef USE_TRANS_TABLE
    LOG("fill=%.3f", transTable.percentFull() );
    //LOG("\n");
    //printTT( transTable );
    //printTTInfo( transTable );
#endif
    LOG("\n");
  }

  // If didn't find solution, then we went up to our maximum depth.  May want to increase MAX_DEPTH.
  if( status != SEARCH_FOUND_SOLUTION)
    depth = -1;

  return depth;
}

///////////////////////////////
// DFS ////////////////////////
///////////////////////////////

DFS::DFS(PerimeterDb & _perimeterDb) : generationCount(0), perimeterDb(_perimeterDb)
{}

inline void DFS::search( const SearchState & goal, const int & depth)
{
  SearchState state;
  LOG("Populating PerimeterDb\n");
  for( int d=0; d<depth; ++d )
  {
    state = goal;
    dfsRecursive( state, d, d);
    LOG("\n");
    printTime(NORMAL);
    LOG("depth=%3.i ", d);
    perimeterDb.printInfo(NORMAL);
    perimeterDb.printHistogram(DEBUG,d);
  }

#ifdef USE_PERIMETER_SCAN_EXPANSION
    // Error limited to 5
    // This isn't the most efficient way to do this, but it the easiest
    //double avgDepth;
    //do
    //{
    //  avgDepth = perimeterDb.getAvgDepth();
      // TODO-- we're not storing the prevOp, so there's a big inefficiency there!
  //for( int d=0; d<5; ++d )
  const int ERROR = 5;
  const int iteration = 2;
  for( int it=0; it<iteration; ++it)
  {
      for( unsigned int i=0; i<PERIMETER_DB_SIZE; ++i)
      {
        PerimeterDbEntry* entry = perimeterDb.getState(i);
        if( entry )
        {
          // reset entry so that search doesn't get cutoff at 1st node
          //entry->iteration = 0;
          // init
          state.state = entry->state;
          state.init();
          state.cost = entry->cost;
          // search
          dfsRecursive( state, state.cost+ERROR*it, depth+it );
        }
      }
      LOG("depth=%3.i ", depth+ERROR*it);
      perimeterDb.printInfo(NORMAL);
    //} while( avgDepth < perimeterDb.getAvgDepth() );
  }
#endif

}

inline void DFS::dfsRecursive( SearchState & state, const int & costLimit, const int & iteration )
{
  generationCount++;
  // Debugging
  state.print(DEBUG);
  LOG_DEBUG(" costLimit=%i iteration=%i\n",costLimit,iteration);

  // Only continue if node needs expansion.
  if( prune(state,costLimit, iteration) )
  {
    return;
  }

  // Expanding node
/*  if( state == m_start)
  {
    return SEARCH_FOUND_SOLUTION;	// Found the solution
  }
*/

  //NodeStatus childrenStatus = SEARCH_ALL_CHILDREN_IN_TT;

  const OpList opList = state.findPredecessorOperators();
  for( int i=0; i<opList.length; i++ )
  {
    state.apply( opList.ops[i] );
    /*NodeStatus status =*/ dfsRecursive( state, costLimit, iteration );
    state.unapply( opList.ops[i] );
  }

  return;// childrenStatus;
}

// return true if pruned
// return false if needs expansion
inline bool DFS::prune(
  const SearchState & state,
  const int & costLimit,
  const int & iteration
  )
{
  if( state.cost > costLimit )
  { // Pruned by cost
    //LOG("pruning by cost=%i limit=%i iteration=%i\n",state.cost,costLimit,iteration);
    return true;
  }

#ifdef USE_PERIMETER_DB
  if( perimeterDb.pruneState(state.state, state.hash, state.cost, iteration) )
  {
    //LOG("pruning by PerimeterDb=%i limit=%i iteration=%i\n",state.cost,costLimit,iteration);
    return true;
  }
#endif
  //LOG("updating entry in DB\n");
  return false;
}
