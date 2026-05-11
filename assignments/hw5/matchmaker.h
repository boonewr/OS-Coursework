#ifndef __MATCHMAKER_H__
#define __MATCHMAKER_H__

#include <stdbool.h>

/** Interface for a monitor that lets us pair up threads with similar
    skill levels. */

/** Initialize the monitor for skill levels ranging from 0 .. range,
    with gap as the maximum skill difference between compatible players. */
void initMonitor( int range, int gap );

/** Destroy the monitor, freeing any resources it uses. */
void destroyMonitor();

/** Pair up with a compatible opponent for a game.  Returns true if we
    got a match before stopPlaying was called. */
bool findMatch( char const *name, int skill );

/** Called by a player when they are done playing. */
void donePlaying();

/** Called at the end of the day, when its time to stop pairing up
    players. */
void stopMatchmaking();

#endif
