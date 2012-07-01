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
  for( int i=0; i<NUM_TILES; i++ ) {
    this->tiles[i]= i;
  }
  this->blankLocation = 0;
}

inline void State::load( const char* str )
{
  //LOG("offset=");
  int offset = 0;
  for( int i=0; i<NUM_TILES; i++ ) {
    //LOG("%i ",offset);
    int tileNum;
    sscanf( str+offset, "%d ", &tileNum );
    offset += strspn(str+offset,"1234567890");
    offset += strspn(str+offset," ");
    this->tiles[i]= tileNum;
    if( tileNum == 0 ) {
      this->blankLocation = i;
    }
  }
  //LOG("\n");
}

inline bool State::operator==( const State & state2 ) const
{
  return (
    //this->state.blankLocation==state2.state.blankLocation &&	// optimization
    !memcmp(this->tiles, state2.tiles, NUM_TILES*sizeof(this->tiles[0]))
  );
}

inline int State::getNewBlankLoc(const Operator & op) const
{
  const unsigned int oldBlankLocation = this->blankLocation;
  switch(op) {
    case OP_RIGHT:
      return oldBlankLocation+1;
    case OP_LEFT:
      return oldBlankLocation-1;
    case OP_UP:
      return oldBlankLocation-WIDTH;
    case OP_DOWN:
      return oldBlankLocation+WIDTH;
    default:
      return oldBlankLocation;
  }
}

// Applies operator to transform the state.
inline int  State::apply( const Operator & op, Heuristic * pHeuristic, Hash * pHash )
{
  const unsigned int oldBlankLocation = this->blankLocation;
  register unsigned int newBlankLocation = getNewBlankLoc(op);

  // change the rest of the state
  this->tiles[oldBlankLocation] = this->tiles[newBlankLocation];
  this->tiles[newBlankLocation] = 0;
  this->blankLocation = newBlankLocation;

#ifdef USE_HASH
  if(pHash)
  {
#ifdef USE_INCREMENTAL_HASH
    pHash->incrementHash(*this,oldBlankLocation);
#else
    pHash->calculateHash(*this);
#endif
  }
#endif



#ifdef USE_HEURISTIC
  if(pHeuristic)
  {
#ifdef USE_INCREMENTAL_HEURISTIC
    pHeuristic->incrementHeuristic(*this,oldBlankLocation);
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
  _LOG( level, "blankLocation=%2i, ", this->blankLocation);
  _LOG( level, "tiles=" );
  if( g_logLevel > VERBOSE )
  {
    // compact state
    for( int i=0; i<NUM_TILES; i++ ) {
      _LOG( level, "%x", this->tiles[i] );
    }
  }
  else
  {
    // expanded state
    LOG_DEBUG("\n");
    for( int y=0; y<HEIGHT; y++ ) {
      //LOG("");
      for( int x=0; x<WIDTH; x++ ) {
        _LOG( level, "%2i ", this->tiles[y*WIDTH+x] );
      }
      LOG_DEBUG("\n");
    }
  }
}

inline const OpList OpLookupTable::_getValidOperators( const int & blankLoc, const Operator & prevOp )
{
  OpList opList;
  opList.length = 0;

  // Check for move to right
#ifdef USE_SKIP_TRANS_OP
  Operator transOp = reverse(prevOp);	// skip the op that results in a transposition.
  if( transOp != OP_RIGHT ) {
#endif
    if( (blankLoc+1)%WIDTH != 0 )
     opList.ops[opList.length++] = OP_RIGHT;

  // Check for move to left
#ifdef USE_SKIP_TRANS_OP
  } if( transOp != OP_LEFT ) {
#endif
    if( (blankLoc)%WIDTH != 0 )
      opList.ops[opList.length++] = OP_LEFT;

  // Check for move up
#ifdef USE_SKIP_TRANS_OP
  } if( transOp != OP_UP ) {
#endif
    if( (blankLoc)/WIDTH != 0 )
      opList.ops[opList.length++] = OP_UP;

  // Check for move down
#ifdef USE_SKIP_TRANS_OP
  } if( transOp != OP_DOWN ) {
#endif
    if( (blankLoc)/WIDTH != HEIGHT-1 )
        opList.ops[opList.length++] = OP_DOWN;

#ifdef USE_SKIP_TRANS_OP
  }
#endif

  // Debug
  opList.print(DEBUG);
#ifdef USE_SKIP_TRANS_OP
  LOG_DEBUG(" prevOp=%i transOp=%i", prevOp, transOp);
#endif
  LOG_DEBUG("\n");

  return opList;
}

void OpList::print(LogLevel level) const
{
  _LOG(level," ops=[");
  for( int i=0; i<this->length; i++)
    _LOG(level," %i", this->ops[i]);
  _LOG(level,"], length=%d", this->length);
}

OpLookupTable::OpLookupTable()
{
  // initialize
  for(int blankLoc=0; blankLoc<NUM_TILES; blankLoc++)
  {
    for(int prevOp=(int)NO_OP; prevOp<=(int)OP_DOWN; prevOp++)
    {
      operatorTable[blankLoc][prevOp] = _getValidOperators( blankLoc, (Operator)prevOp );
    }
  }
}

inline const OpList OpLookupTable::getValidOperators( const int & blankLoc, const Operator & prevOp )
{

#ifdef OP_LOOKUP_TABLE
  return operatorTable[blankLoc][prevOp];
#else
  return _getValidOperators(blankLoc, prevOp);
#endif
}



inline const OpList State::findSuccessorOperators( const Operator & prevOp ) const
{
#ifdef USE_SKIP_TRANS_OP
  return State::operatorTable.getValidOperators(this->blankLocation, prevOp);
#else
  return State::operatorTable.getValidOperators(this->blankLocation, NO_OP);
#endif
}

inline const OpList State::findPredecessorOperators( const Operator & prevOp ) const
{
  return findSuccessorOperators( prevOp );
}


// Call reverse to get the opposite operator
inline Operator const reverse( const Operator & op )
{
  switch( op )
  {
  case OP_RIGHT: return OP_LEFT;
  case OP_LEFT:  return OP_RIGHT;
  case OP_UP:    return OP_DOWN;
  case OP_DOWN:  return OP_UP;
  default :      return NO_OP;
  }
}

////////////////////////////////
// Heuristic ///////////////////
////////////////////////////////

#ifdef USE_HEURISTIC

bool Heuristic::tableInitialized = false;
tile_t Heuristic::mdTable[NUM_TILES][NUM_TILES];

inline Heuristic::Heuristic() 
{
  if(!tableInitialized) 
    initTable(); 
}

inline void Heuristic::print(LogLevel level) const
{
  _LOG(level,"%3i", value);
}

inline void Heuristic::incrementHeuristic( const State& state, const int & newTileLoc )
{
  const int tileNum = state.tiles[newTileLoc];
  const int oldTileLoc = state.blankLocation;
  this->value -= Heuristic::mdTable[tileNum][oldTileLoc];
  this->value += Heuristic::mdTable[tileNum][newTileLoc];
  //LOG(" tileNum=%i oldTileLoc=%i -=%i +=%i\n",
  //    tileNum,oldTileLoc,Heuristic::mdTable[tileNum][oldTileLoc],Heuristic::mdTable[tileNum][newTileLoc]);

}

// Non-incremental heuristic calculation
inline void Heuristic::calculateHeuristic(const State& state)
{
  //this->print();
  //LOG("\n");
  this->value = 0;
  for( int i=0; i<NUM_TILES; i++ ) {
    const int & tileLoc = i;
    const int & tileNum = state.tiles[i];
    if(tileNum==0)
      continue;
    //state.heuristic += abs(tileLoc/4 - tileNum/4) + abs(tileLoc%4 - tileNum%4);
    this->value += (unsigned int) Heuristic::mdTable[tileNum][tileLoc];

    //LOG( "%2d+", (unsigned int) Heuristic::mdTable[tileNum][tileLoc] );
    //LOG(" tileLoc=%i tileNum=%i deltaX=%i deltaY=%i sum=%i\n", tileLoc,tileNum,
    //  tileLoc%4-tileNum%4,tileLoc/4-tileNum/4,abs(tileLoc%4-tileNum%4)+abs(tileLoc/4-tileNum/4));
  }
  //LOG(" heuristic = %d \n", state.heuristic );
}

inline void Heuristic::printTable(LogLevel level) const
{
  // Error check
  for( int i=0; i<NUM_TILES; i++ ) {
    for( int j=0;j<NUM_TILES; j++ ) {
      _LOG(level, " %2d", (unsigned int)Heuristic::mdTable[i][j] );
    }
    _LOG(level,"\n");
  }
}

inline void Heuristic::initTable()
{
  // init Heuristic Table
  for( int i=0; i<NUM_TILES; i++ ) {
    for( int j=0; j<NUM_TILES; j++ ) {
      Heuristic::mdTable[i][j] = abs(j%WIDTH-i%WIDTH)
                                + abs(j/WIDTH-i/WIDTH);
    }
  }

  Heuristic::tableInitialized = true;

  // error check
  printTable(DEBUG);
}

#else
Heuristic::Heuristic() {}
#endif

////////////////////////////////
// Hash ////////////////////////
////////////////////////////////

#ifdef USE_HASH

unsigned int Hash::hashTable[NUM_TILES][NUM_TILES];
bool Hash::tableInitialized = false;

inline void Hash::printTable(LogLevel level) const
{
  // Error check
  for( int i=0; i<NUM_TILES; i++ ) {
    for( int j=0;j<NUM_TILES; j++ ) {
      _LOG(level, " %8x", Hash::hashTable[i][j] );
    }
    _LOG(level,"\n");
  }
}

inline void Hash::initTable()
{
  srandom( HASHSEED );
  // init Hash Table
  for( int i=0; i<NUM_TILES; i++ ) {
    for( int j=0; j<NUM_TILES; j++ ) {
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
  for( int location=0; location<NUM_TILES; location++ ) {
    int tileNumber = state.tiles[location];
    if( tileNumber != 0 )
    {	// not the blank
      this->value ^= Hash::hashTable[tileNumber][location];
    }
  }
}

// Incremental hash calculation
inline void Hash::incrementHash( const State& state, const int & oldBlankLoc )
{
  const int & newTileLoc = oldBlankLoc;
  const int & oldTileLoc = state.blankLocation;
  const int & tileNumber = state.tiles[newTileLoc];

  this->value ^= Hash::hashTable[tileNumber][oldTileLoc];
  this->value ^= Hash::hashTable[tileNumber][newTileLoc];
}

#endif
