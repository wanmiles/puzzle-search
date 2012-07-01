/**
 * Copyright Ken Anderson, 2010-2011
 */

#ifdef USE_PERIMETER_DB

/////////////////////////////////////
// PerimeterDbEntry /////////////////
/////////////////////////////////////

#include <math.h>

PerimeterDbEntry::PerimeterDbEntry()
: state(), cost(MAX_COST)
{}

PerimeterDbEntry::~PerimeterDbEntry()
{}

inline void PerimeterDbEntry::print(LogLevel level) const
{
  this->state.print(NORMAL);
  _LOG(level," cost=%i", this->cost );
#ifdef USE_LAZY_PERIMETER
  _LOG(level," iteration=%i", this->iteration );
#endif
  _LOG(level,"\n");
}


// Updates the entry if needed.
// returns true if entry was updated and the node must be expanded.
// returns false if the node has been visited previously
// and doesn't need expanding
inline bool PerimeterDbEntry::updateEntry(const int & cost, const int & iteration )
{

#ifdef USE_LAZY_PERIMETER
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
    if ( iteration == this->iteration )
    {
      // Reached on this iteration.  Prune.
      //LOG("pruned by TT\n");
      return false;
    } else {
      // reached on previous iteration.
      // Update the cost, but do not prune.
      this->iteration = iteration;
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
#ifdef USE_LAZY_PERIMETER
    this->iteration = iteration;
#endif
    //LOG("set cost=%d costLimit=%d\n", cost, costLimit);
    return true;
  }

  return true;
}

/////////////////////////////////////
// PerimeterDb///////////////////////
/////////////////////////////////////

inline void PerimeterDb::print(LogLevel level) const
{
  _LOG(level,"PerimeterDb= [\n");
  for( int i=0; i<PERIMETER_DB_SIZE; i++ )
  {
    const PerimeterDbEntry & entry = perimeterDb[i];
    if( entry.cost != MAX_COST )
    {
      entry.print(level);
    }
  }
  _LOG(level,"]\n");
}

inline void PerimeterDb::printInfo(LogLevel level) const
{
  long long numEntries;
  double percentFull;
  double avgDepth;
  calculate(numEntries, avgDepth, percentFull);
  _LOG(level,"PerimeterDb: size=%i, entries=%12lli, fill=%f avgDepth=%f \n", PERIMETER_DB_SIZE, numEntries, percentFull, avgDepth);
}

// TODO -- not the most efficient...  calculate is called twice...
inline double PerimeterDb::getAvgDepth() const
{
  long long numEntries;
  double percentFull;
  double avgDepth;
  calculate(numEntries, avgDepth, percentFull);
  return avgDepth;
}

// Histogram for distance values and priority values
inline void PerimeterDb::printHistogram(LogLevel level, const unsigned int depth) const
{
  const unsigned int N_BUCKETS = 32;
  unsigned int value_count[MAX_COST];
  unsigned int priority_count[N_BUCKETS];

  // init
  for( int i=0; i<MAX_COST; i++)
  {
    value_count[i]=0;
  }
  for( unsigned int i=0; i<N_BUCKETS; i++)
  {
    priority_count[i]=0;
  }

  // count
  Hash hash;
  for( int i=0; i<PERIMETER_DB_SIZE; i++ )
  {
    const PerimeterDbEntry & entry = perimeterDb[i];
    if( entry.cost != MAX_COST )
    {
      value_count[entry.cost]++;

      hash.calculateHash(entry.state);
      unsigned int priority = getPriority(hash);
      unsigned int bucket = log(priority)/log(2);
      //_LOG(level,"i=%i, cost=%i, hash=%i", i, entry.cost, hash.value );
      //unsigned int bucket = priority/(MAX_PRIORITY/N_BUCKETS);
      //_LOG(level,", priority=%i bucket=%i \n", priority, bucket);
      if(bucket < N_BUCKETS)
      {
        priority_count[bucket]++;
      }
    }
  }

  // print distance histogram
  _LOG(level,"PerimeterDb value histogram= [\n");
  for( unsigned int i=0; i<depth; i++)
  {
    _LOG(level,"%i:%i ", i, value_count[i]);
  }
  _LOG(level,"]\n");

/*
  // print priority histogram
  _LOG(level,"PerimeterDb priority histogram= [\n");
  for( unsigned int i=0; i<N_BUCKETS; i++)
  {
    _LOG(level,"2^%i:%i ", i, priority_count[i]);
  }
  _LOG(level,"]\n");
*/
}

// Use this function if you want to initialize the TT as well.
inline void PerimeterDb::reset()
{
  PerimeterDbEntry entry;
  for( int i=0; i<PERIMETER_DB_SIZE; i++)
  {
    perimeterDb[i] = entry;
  }
}

inline void PerimeterDb::calculate(long long & numEntries, double & avgDepth, double & percentFull) const
{
  numEntries = 0;
  avgDepth = 0;
  for(int i=0; i<PERIMETER_DB_SIZE; i++) {
    PerimeterDbEntry & entry = perimeterDb[i];
    if( entry.cost != MAX_COST ) {
      numEntries++;
      avgDepth = (avgDepth/numEntries)*(numEntries-1) + (double)entry.cost/numEntries;
    }
  }
  percentFull = (double)numEntries/PERIMETER_DB_SIZE;
}

inline int PerimeterDb::getHeuristic( const State & state, const Hash & hash ) const
{
  // Calculate index and lookup entry
  unsigned int index = calculateIndex(hash);
  PerimeterDbEntry & entry = perimeterDb[index];

  if( entry.state == state )
  { // found the node
    //LOG("perimeter heuristic value=%i\n",entry.cost);
    return entry.cost;
  }

  return 0;
}

inline PerimeterDbEntry * PerimeterDb::getState( const unsigned & index )
{
  PerimeterDbEntry & entry = perimeterDb[index];
  if( entry.cost != MAX_COST )
  {
    return &entry;
    //state = entry.state;
    //cost = entry.cost;
    //return true;
  }

  //return false;
  return NULL;
}


inline bool PerimeterDb::pruneState( const State & state, const Hash & hash, const int cost, const int iteration )
{
  // Calculate index and lookup entry
  unsigned int index = calculateIndex(hash);
  PerimeterDbEntry & entry = perimeterDb[index];
  //LOG("looked at TT: hash=%x index=%i\n", state.hash, index);
#ifdef USE_PERIMETER_STATE_PRIORITIZATION
  unsigned int priority = getPriority(hash);
#endif

  if( entry.state == state )
  { // found the node
    if( entry.updateEntry( cost, iteration ) )
    {	// Needs updating
      return false;
    }
    else
    { // does not need updating
      return true;
    }
  }
  else if ( entry.cost == MAX_COST )
  { // No node in the table at this location.
    // So just add the node in.
    //LOG("Adding state to TT\n");
    entry.state = state;
    entry.cost = cost;
#ifdef USE_LAZY_PERIMETER
    entry.iteration = iteration;
#endif
#ifdef USE_PERIMETER_STATE_PRIORITIZATION
    entry.priority = priority;
#endif
    return false;
  }
#ifdef USE_PERIMETER_STATE_PRIORITIZATION
  else if ( priority > entry.priority )
  {	// higher priority node-- just replace the entry
/*    LOG("replacing node: state=");
    entry.state.print();
    LOG(" cost=%i priority=%i\n", entry.cost, entry.priority);
    LOG("     with node: state=");
    state.print();
    LOG(" cost=%i priority=%i\n", cost, priority);
*/
    entry.state = state;
    entry.cost = cost;
#ifdef USE_LAZY_PERIMETER
    entry.iteration = iteration;
#endif
    entry.priority = priority;
  }
#endif

  // entry occupied by another state-- the state isn't actually updated, but we say it is because it must be expanded.
  return false;
}

inline unsigned int PerimeterDb::calculateIndex( const Hash & hash ) const
{
  return hash.value%PERIMETER_DB_SIZE;
}

#endif
