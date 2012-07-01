/**
 * Copyright Ken Anderson, 2010-2011
 */

#ifndef SEARH_STATE_H
#define SEARH_STATE_H

#include "common.h"
#include "domain.h"

enum PruneStatus
{
  NODE_NEEDS_EXPANSION = 0,	// not in TT and low cost, so expand.
  NODE_PRUNED_BY_COST,		// g+h > depth... therefore pruned.
  NODE_PRUNED_BY_TT,		// Usually gets pruned.
};

enum NodeStatus
{
  SEARCH_FOUND_SOLUTION = 0,	// solution found, overrrides all others
  SEARCH_ALL_CHILDREN_IN_TT,	// all children nodes in trans table
  SEARCH_NODE_IN_TT,		// node in trans table,
          // but don't know about children
  SEARCH_SOME_CHILDREN_LEAF,	// children definatly not in trans table
};

// Most of the information we use during the search is here for easy access.
class SearchState
{
public:
  // Ordered for efficiency (small first)
  State         state;
  int						cost;						// cost of path from start to state
#ifdef USE_SKIP_TRANS_OP
  Operator      prevOp;					// Used for simple cycle detection
#endif
#ifdef USE_HEURISTIC
  Heuristic     incHeuristic;		// Used for incremental heuristics
#endif
#ifdef USE_HASH
  Hash          hash;						// Used for incremental hashing
#endif

public:
  SearchState();
  void init( );	// assumes that state is set.
  void load( const char* str );
  bool operator==( const SearchState & state2 ) const;
  SearchState& operator=( const SearchState &rhs);
  void apply( const Operator & op );
  // CAREFUL - does not unapply the prevOp.
  void unapply( const Operator & op );
  const OpList findSuccessorOperators() const;
  const OpList findPredecessorOperators() const;
  void print( LogLevel level ) const;
  // Take numRandOps random operations away from the current state.
  // These operations had better be reversable,
  // or you may come to an unsolvable state.
  void randomize( const int numRandOps );
private:
  void _init();
  int _apply( const Operator & op );
};


////////////////////////////////
// SearchState ///////////////////////
////////////////////////////////

SearchState::SearchState()
{
  this->state.init();
  _init();
}

// Copy the entire SearchState structure.
// This includes any optional portions.
SearchState& SearchState::operator=(const SearchState &rhs)
{
  memcpy(this, &rhs, sizeof(SearchState) );
  return *this;
}

inline void SearchState::_init()
{
  this->cost = 0;
#ifdef USE_HASH
  this->hash.calculateHash(this->state);
#endif
#ifdef USE_SKIP_TRANS_OP
  this->prevOp = NO_OP;
#endif
#ifdef USE_HEURISTIC
  this->incHeuristic.calculateHeuristic(this->state);
#endif
}

inline void SearchState::init()
{
  _init();
}

inline void SearchState::load( const char* stateDescription )
{
  this->state.load(stateDescription);
  _init();
}

inline void SearchState::print(LogLevel level) const
{
  _LOG(level, "[ ");
  this->state.print(level);
  _LOG(level, ", cost=%i,", this->cost);
#ifdef USE_HEURISTIC
  _LOG(level,  " incHeur=");
  this->incHeuristic.print(level);
  _LOG(level,  ",");
#endif
#ifdef USE_HASH
  _LOG(level,  " hash=");
  this->hash.print(level);
  _LOG(level,  ",");
#endif
#ifdef USE_SKIP_TRANS_OP
  _LOG(level, " prevOp=%i,", this->prevOp );
#endif
  _LOG(level,  "] ");
}

inline bool SearchState::operator==( const SearchState & state2 ) const
{
  return(
#ifdef USE_HASH
    this->hash.value==state2.hash.value &&			// optimization
#endif
    this->state == state2.state
  );
}

inline int SearchState::_apply( const Operator & op )
{
  this->print(VERBOSE);
  LOG_VERBOSE("...applying op=%i... ", op);

  // Change State and modify/increment the hash
  //
#if defined USE_HEURISTIC && defined USE_HASH
  const int cost = this->state.apply(op,&(this->incHeuristic),&(this->hash));
#elif defined USE_HEURISTIC
  const int cost = this->state.apply(op,&(this->incHeuristic),NULL);
#elif defined USE_HASH
  const int cost = this->state.apply(op,NULL,&(this->hash));
#else
  const int cost = this->state.apply(op,NULL,NULL);
#endif

  // Change the rest
  // set the previous operation
#ifdef USE_SKIP_TRANS_OP
  this->prevOp = op;
#endif

  this->print(VERBOSE);
  LOG_VERBOSE("\n");

  // cost
  return cost;
}

inline void SearchState::apply( const Operator & op )
{
  this->cost += _apply(op);
}

inline void SearchState::unapply( const Operator & op )
{
  this->cost -= _apply(reverse(op));
}

inline void SearchState::randomize( const int numRandOps )
{
  for( int i=0; i<numRandOps; i++ )
  {
#ifdef USE_SKIP_TRANS_OP
    const OpList opList = this->state.findPredecessorOperators( this->prevOp );
#else
    const OpList opList = this->state.findPredecessorOperators( NO_OP );
#endif
    //LOG("opListLength=%i\n", opListLength);
    int random = rand() % opList.length;
    apply( opList.ops[random]);
  }
}

inline const OpList SearchState::findSuccessorOperators( ) const
{
#ifdef USE_SKIP_TRANS_OP
  return this->state.findSuccessorOperators(this->prevOp);
#else
  return this->state.findSuccessorOperators(NO_OP);
#endif
}

inline const OpList SearchState::findPredecessorOperators( ) const
{
#ifdef USE_SKIP_TRANS_OP
  return this->state.findPredecessorOperators(this->prevOp);
#else
  return this->state.findPredecessorOperators(NO_OP);
#endif

}

#endif
