/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : brute.c
*   Purpose : Implement brute force matching of uncoded strings for LZSS
*             algorithm.
*   Author  : Michael Dipperstein
*   Date    : February 18, 2004
*
****************************************************************************
*
* Brute: Brute force matching routines used by LZSS Encoding/Decoding
*        Routine
* Copyright (C) 2004 - 2007, 2014, 2020 by
* Michael Dipperstein (mdipperstein@gmail.com)
*
* This file is part of the lzss library.
*
* The lzss library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The lzss library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include "lzlocal.h"

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of mathcing uncoded strings to strings in the
*                sliding window.  The brute force search doesn't use any
*                special structures, so this function doesn't do anything.
*   Parameters : buffers - pointer to structure with sliding window and
*                          uncoded lookahead buffers
*   Effects    : None
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int InitializeSearchStructures(buffers_t *buffers)
{
    (void)buffers;      /* not used */
    return 0;
}

/****************************************************************************
*   Function   : FindMatch
*   Description: This function will search through the slidingWindow
*                dictionary for the longest sequence matching the MAX_CODED
*                long string stored in uncodedLookahed.
*   Parameters : buffers - pointer to structure with sliding window and
*                          uncoded lookahead buffers
*                windowHead - head of sliding window
*                uncodedHead - head of uncoded lookahead buffer
*                uncodedLen - length of uncoded lookahead buffer
*   Effects    : None
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
encoded_string_t FindMatch(buffers_t *buffers,
    const unsigned int windowHead,
    const unsigned int uncodedHead,
    const unsigned int uncodedLen)
{
    encoded_string_t matchData;
    unsigned int i;     /* current search start offset */
    unsigned int j;     /* current match length */

    /* unwrapped copy of uncoded lookahead */
    unsigned char uncoded[MAX_CODED];

    matchData.length = 0;
    matchData.offset = 0;

    if (uncodedLen <= MAX_UNCODED)
    {
        /* don't even bother, there aren't enough symbols to encode */
        return matchData;
    }

    for (i = 0; i < uncodedLen; i++)
    {
        uncoded[i] =
            buffers->uncodedLookahead[Wrap((uncodedHead + i), MAX_CODED)];
    }

    i = windowHead;  /* start at the beginning of the sliding window */
    j = 0;

    while (1)
    {
        if (buffers->slidingWindow[i] == uncoded[0])
        {
            /* we matched one. how many more match? */
            j = 1;

            while(uncoded[j] ==
                buffers->slidingWindow[Wrap((i + j), WINDOW_SIZE)])
            {
                j++;

                if (j == uncodedLen)
                {
                    break;
                }
            }

            /* end of current match */
            if (j > matchData.length)
            {
                matchData.length = j;
                matchData.offset = i;
            }
        }

        if (j == uncodedLen)
        {
            break;
        }

        CyclicInc(i, WINDOW_SIZE);
        if (i == windowHead)
        {
            /* we're where we started */
            break;
        }
    }

    return matchData;
}

/****************************************************************************
*   Function   : ReplaceChar
*   Description: This function replaces the character stored in
*                slidingWindow[charIndex] with the one specified by
*                replacement.
*   Parameters : slidingWindow - pointer to the head of the sliding window.
*                charIndex - sliding window index of the character to be
*                            removed from the linked list.
*                replacement - new character
*   Effects    : slidingWindow[charIndex] is replaced by replacement.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int ReplaceChar(unsigned char *slidingWindow,
    const unsigned int charIndex,
    const unsigned char replacement)
{
    slidingWindow[charIndex] = replacement;
    return 0;
}
