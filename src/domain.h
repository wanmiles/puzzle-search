/**
 * Copyright Ken Anderson, 2010-2012
 */

#ifndef DOMAIN_H
#define DOMAIN_H

#include "common.h"

#if DOMAIN == 1
#		include "slidingTile.h"
#elif DOMAIN == 2
#		include "kpancake.h"
#else
#		error no domain specified
#endif


#endif
