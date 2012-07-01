/**
 * Copyright Ken Anderson, 2010-2011
 */

/////////////////////////////////
// TransTableEntry //////////////
/////////////////////////////////

TransTableEntry::TransTableEntry()
: cost(MAX_COST)
#ifdef USE_LAZY_TRANS_TABLE
, costLimit(-1)
#endif
{
}

inline TransTableEntry &TransTable::getEntry(const unsigned index)
{
  return transTable[index];
}

/////////////////////////////////
// TransTable ///////////////////
/////////////////////////////////

// Use this function if you want to initialize the TT as well.
inline void TransTable::reset()
{
  TransTableEntry entry;
  for( int i=0; i<TT_SIZE; i++)
  {
    transTable[i] = entry;
  }
  //memset( transTable, 0, sizeof(SearchState)*TT_SIZE );
}

inline unsigned int TransTable::calculateIndex( const Hash & hash ) const
{
  return hash.value%TT_SIZE;
}

// Updates the entry if needed.
// returns true if entry was updated and the node must be expanded.
// returns false if the node has been visited previously
// and doesn't need expanding
inline bool TransTableEntry::updateEntry(const int & cost, const int & costLimit )
{

#ifdef USE_LAZY_TRANS_TABLE
  if( cost > this->cost )
  {
    // Cost higher than cached state.
    // This is a suboptimal path. Prune node.
    //LOG("pruned by TT\n");
    return false;
  } else if ( cost == this->cost ) {
    // Cost same as cache.
    // This is potentially an optimal path.
    // (technically- this is a transposition)
    // Check if was reached on the current search iteration.
    if ( costLimit == this->costLimit )
    {
      // Reached on this iteration.  Prune.
      //LOG("pruned by TT\n");
      return false;
    } else {
      // reached on previous iteration.
      // Update the cost, but do not prune.
      this->costLimit = costLimit;
      return true;
    }
  }
#else
  if( cost >= this->cost )
  {
    // Reached previously.  Prune.
    //LOG("pruned by TT\n");
    return false;
  }
#endif
  else
  {
    // Node not pruned.
    // and cost is lower than the current entry.
    // This means we we need to update the entry.
    this->cost = cost;
#ifdef USE_LAZY_TRANS_TABLE
    this->costLimit = costLimit;
#endif
    //LOG("set cost=%d costLimit=%d\n", cost, costLimit);
    return true;
  }

  return true;
}

inline int TransTable::getCachedHeuristic( const State & state, const Hash & hash ) const
{
#ifdef USE_TRANS_TABLE_HEUR_CACHING
  // Calculate index and lookup entry
  unsigned int index = calculateIndex(hash);
  TransTableEntry & entry = transTable[index];

  if( entry.state == state )
  { // found the node
    //LOG("perimeter heuristic value=%i\n",entry.cost);
    return entry.heuristic.value;
  }
#endif
  return 0;
}

// TODO- potential optimization-- only lookup once.  But update a couple times.
inline void TransTable::updateCachedHeuristic( const State & state, const Hash & hash, const int & heur ) const
{
#ifdef USE_TRANS_TABLE_HEUR_CACHING
  // Calculate index and lookup entry
  unsigned int index = calculateIndex(hash);
  TransTableEntry & entry = transTable[index];

  if( entry.state == state )
  { // found the node
    //LOG("perimeter heuristic value=%i\n",entry.cost);
    entry.heuristic.value = std::max(entry.heuristic.value, heur);
  }
#endif
}

inline bool TransTable::pruneState( const State & state, const Hash & hash, const int & heur, const int & cost, const int & costLimit )
{
  // Calculate index and lookup entry
  unsigned int index = calculateIndex(hash);
  TransTableEntry & entry = transTable[index];
  //LOG("looked at TT: hash=%x index=%i\n", state.hash, index);
#ifdef USE_TRANS_TABLE_STATE_PRIORITIZATION
  unsigned int priority = getPriority(hash);
#endif

  if( entry.state == state )
  { // found the node
#ifdef USE_TRANS_TABLE_HEUR_CACHING
    if( entry.heuristic.value < heur )
    {
      entry.heuristic.value = heur;
    }
#endif

    // Check whether an update is required
    if( entry.updateEntry( cost, costLimit ) )
    {	// Needs updating
      return false;
    }
    else
    { // does not need updating
      return true;
    }

  // No node in the table at this location.
  // So just add the node in.
  } else if ( entry.cost == MAX_COST ) {
    // Did not find the state.
    // Add the state if there is not an entry already there.
    //LOG("Adding state to TT\n");
    entry.state = state;
    entry.cost = cost;
#ifdef USE_LAZY_TRANS_TABLE
    entry.costLimit = costLimit;
#endif
#ifdef USE_TRANS_TABLE_STATE_PRIORITIZATION
    entry.priority = priority;
#endif
    return false;
  }
#ifdef USE_TRANS_TABLE_STATE_PRIORITIZATION
  else if ( priority > entry.priority )
  {	// higher priority node-- just replace the entry
    entry.state = state;
    entry.cost = cost;
#ifdef USE_LAZY_TRANS_TABLE
    entry.costLimit = costLimit;
#endif
    entry.priority = priority;
  }
#endif
  // entry occupied by another state-- the state isn't actually updated, but we say it is because it must be expanded.
  return false;
}

inline long long TransTable::numEntries() const
{
  long long fill = 0;
  for(int i=0; i<TT_SIZE; i++) {
    if( transTable[i].cost != MAX_COST ) {
      fill++;
    }
  }
  return fill;
}

inline double TransTable::percentFull( ) const
{
  long long fill = numEntries( );
  return (double)fill/(double)TT_SIZE;
}

inline void TransTableEntry::print(LogLevel level ) const
{
  this->state.print(level);
  _LOG(level," cost=%i", this->cost );
#ifdef USE_LAZY_TRANS_TABLE
  _LOG(level," costLim=%i", this->costLimit );
#endif
  _LOG(level,"\n");
}

inline void TransTable::print(LogLevel level) const
{
  _LOG(level,"TransTable= [\n");
  for( int i=0; i<TT_SIZE; i++ )
  {
    const TransTableEntry & entry = transTable[i];
    if( entry.cost != MAX_COST )
    {
      entry.print(level);
    }
  }
  _LOG(level,"]\n");
}

inline void TransTable::printInfo(LogLevel level) const
{
  _LOG(level,"TransTable: size=%i, entries=%12lli, fill=%f \n", TT_SIZE, numEntries(), percentFull());
}
