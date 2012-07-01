/**
 * Copyright Ken Anderson, 2010-2012
 */

#include <stdio.h>
#include <stdlib.h>	// abs()
#include <string.h>	// memory comparisons

////////////////////////////////
// State ///////////////////////
////////////////////////////////

inline void State::init()
{
  for( int i=0; i<NUM_PANCAKES; i++ ) {
    this->pancakes[i]= NUM_PANCAKES-1-i;
  }
}

inline void State::load( const char* str )
{
  //LOG("offset=");
  int offset = 0;
  for( int i=0; i<NUM_PANCAKES; i++ ) {
    //LOG("%i ",offset);
    int pancakeNum;
    sscanf( str+offset, "%d ", &pancakeNum );
    offset += strspn(str+offset,"1234567890");
    offset += strspn(str+offset," ");
    this->pancakes[i]= pancakeNum;
  }
  //LOG("\n");
}

inline bool State::operator==( const State & state2 ) const
{
  return (
    !memcmp(this->pancakes, state2.pancakes, NUM_PANCAKES*sizeof(this->pancakes[0]))
  );
}


inline int  State::apply( const Operator & op, Heuristic * pHeuristic, Hash * pHash )
{
  const int top = (int)op;
  const int middle = (top+1) / 2;
  for(int i=0; i<middle; i++)
  {
    const int index1 = i;
    const int index2 = top-i;
    std::swap(this->pancakes[index1],this->pancakes[index2]);

#if defined USE_HASH && defined USE_INCREMENTAL_HASH
    if(pHash)
    {
      pHash->incrementHash(*this,index1,index2);
    }
#endif
  }

#if defined USE_HASH && !(defined USE_INCREMENTAL_HASH)
  if(pHash)
  {
    pHash->calculateHash(*this);
  }
#endif


#ifdef USE_HEURISTIC
  if(pHeuristic)
  {
#ifdef USE_INCREMENTAL_HEURISTIC
    pHeuristic->incrementHeuristic(*this,op);
#else
    pHeuristic->calculateHeuristic(*this);
#endif
  }
#endif

  // cost
  return 1;
}


inline void State::print(LogLevel level) const
{
  _LOG( level, "pancakes=" );
  // compact state
  for( int i=0; i<NUM_PANCAKES; i++ ) {
    _LOG( level, "%x", this->pancakes[i] );
  }
}

inline const OpList OpLookupTable::_getValidOperators( const Operator & prevOp )
{
  OpList opList;
  opList.length = 0;
  Operator op;
#ifdef USE_SKIP_TRANS_OP
  Operator transOp = reverse(prevOp);
#endif
  for(int i=(int)NO_OP+1; i<NUM_PANCAKES; i++)
  {
    op = (Operator)i;
#ifdef USE_SKIP_TRANS_OP
    if( op != transOp )	// skip the op that results in a transposition.
#endif
    {
      opList.ops[opList.length] = op;
      opList.length++;
    }
  }

  return opList;
}

void OpList::print(LogLevel level) const
{
  _LOG(level," ops=[");
  for( int i=0; i<this->length; i++)
  {
    _LOG(level," %i", this->ops[i]);
  }
  _LOG(level,"], length=%d", this->length);
}


OpLookupTable::OpLookupTable()
{
  // initialize
#ifdef OP_LOOKUP_TABLE
  for(int prevOp=(int)NO_OP; prevOp<=MAX_NUM_OPS; prevOp++)
  {
    operatorTable[prevOp] = _getValidOperators( (Operator)prevOp );
  }
#endif
}

inline const OpList OpLookupTable::getValidOperators( const Operator & prevOp )
{

#ifdef OP_LOOKUP_TABLE
  return operatorTable[prevOp];
#else
  return _getValidOperators(prevOp);
#endif
}

inline const OpList State::findSuccessorOperators( const Operator & prevOp ) const
{
#ifdef USE_SKIP_TRANS_OP
  return State::operatorTable.getValidOperators(prevOp );
#else
  return State::operatorTable.getValidOperators(NO_OP );
#endif
}

inline const OpList State::findPredecessorOperators( const Operator & prevOp ) const
{
  return findSuccessorOperators( prevOp );
}


// Call reverse to get the opposite operator
inline Operator const reverse( const Operator & op )
{
  return op;
}

////////////////////////////////
// Heuristic ///////////////////
////////////////////////////////

#ifdef USE_HEURISTIC

inline void Heuristic::print(LogLevel level) const
{
  _LOG(level,"%2i", value);
}

// Non-incremental heuristic calculation
inline void Heuristic::calculateHeuristic(const State& state)
{
  this->value = 0;
  int pancake1;
  int pancake2;

  for( int i=0; i<NUM_PANCAKES; i++ )
  {
    pancake1 = state.pancakes[i];
    if( i> 0 )
    {
      pancake2 = state.pancakes[i-1];
    }
    else
    {
      pancake2 = NUM_PANCAKES;
    }

    if( abs(pancake2-pancake1) > 1 )
    {
      this->value++;
    }
    //state.print(NORMAL);
    //LOG(" p1=%i p2=%i heur=%i\n", pancake1, pancake2, this->value);
  }

  //LOG(" Calculating heuristic = %d \n", this->value);
  //state.print(NORMAL);
  //LOG(" heuristic = %d \n", this->value);
}

// State is AFTER the operation occured
inline void Heuristic::incrementHeuristic( const State& state, const Operator op )
{
  int pancake1;
  int pancake2;
  int index = (int)op;

  LOG_VERBOSE(" incHeur");
  // before flip
  if( abs(state.pancakes[index]-NUM_PANCAKES) > 1 )
  {
    this->value--;
    LOG_VERBOSE("-");
  }

  // after flip
  if( abs(state.pancakes[0]-NUM_PANCAKES) > 1 )
  {
    this->value++;
    LOG_VERBOSE("+");
  }

  if(index<NUM_PANCAKES-1)
  {
    // before flip
    pancake1 = state.pancakes[index];
    pancake2 = state.pancakes[index+1];
    if( abs(state.pancakes[index]-state.pancakes[index+1]) > 1 )
    {
      this->value++;
      LOG_VERBOSE("+");
    }

    // before flip
    if( abs(state.pancakes[0]-state.pancakes[index+1]) > 1 )
    {
      this->value--;
      LOG_VERBOSE("-");
    }
  }
  LOG_VERBOSE("\n");
}

#endif

////////////////////////////////
// Hash ////////////////////////
////////////////////////////////

#ifdef USE_HASH

unsigned int Hash::hashTable[NUM_PANCAKES][NUM_PANCAKES];
bool Hash::tableInitialized = false;

inline void Hash::printTable(LogLevel level) const
{
  // Error check
  for( int i=0; i<NUM_PANCAKES; i++ ) {
    for( int j=0;j<NUM_PANCAKES; j++ ) {
      _LOG(level, " %8x", Hash::hashTable[i][j] );
    }
    _LOG(level,"\n");
  }
}

inline void Hash::initTable()
{
  srandom( HASHSEED );
  // init Hash Table
  for( int i=0; i<NUM_PANCAKES; i++ ) {
    for( int j=0; j<NUM_PANCAKES; j++ ) {
      Hash::hashTable[i][j] = (unsigned int) random();
    }
  }

  Hash::tableInitialized = true;

  // error check
  printTable(DEBUG);
}

inline void Hash::print(LogLevel level) const
{
  _LOG(level, "%8x", value);
}

inline void Hash::calculateHash(const State& state )
{
  // hash function
  this->value = 0;
  for( int location=0; location<NUM_PANCAKES; location++ )
  {
    int pancakeNumber = state.pancakes[location];
    this->value ^= Hash::hashTable[pancakeNumber][location];
  }
}

// Incremental hash calculation
inline void Hash::incrementHash( const State & state, const int & index1, const int & index2 )
{
  const int & pancakeNumber1 = state.pancakes[index1];
  const int & pancakeNumber2 = state.pancakes[index2];

  this->value ^= Hash::hashTable[pancakeNumber1][index1];
  this->value ^= Hash::hashTable[pancakeNumber1][index2];
  this->value ^= Hash::hashTable[pancakeNumber2][index1];
  this->value ^= Hash::hashTable[pancakeNumber2][index2];
}

#endif
