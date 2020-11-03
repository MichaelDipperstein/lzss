/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : hash.c
*   Purpose : Implement hash table optimized matching of uncoded strings
*             for LZSS algorithm.
*   Author  : Michael Dipperstein
*   Date    : February 21, 2004
*
****************************************************************************
*
* Hash: Hash table optimized matching routines used by LZSS
*       Encoding/Decoding Routine
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
*                            TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define NULL_INDEX      (WINDOW_SIZE + 1)
#define HASH_SIZE       (WINDOW_SIZE >> 2)  /* size of hash table */

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
unsigned int hashTable[HASH_SIZE];          /* list head for each hash key */
unsigned int next[WINDOW_SIZE];             /* indices of next in hash list */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : HashKey
*   Description: This function generates a hash key for a (MAX_UNCODED + 1)
*                long string located either in the sliding window or the
*                uncoded lookahead.  The key generation algorithm is
*                supposed to be based on the algorithm used by gzip.  As
*                reported in K. Sadakane, H. Imai. "Improving the Speed of
*                LZ77 Compression by Hashing and Suffix Sorting". IEICE
*                Trans. Fundamentals, Vol. E83-A, No. 12 (December 2000)
*   Parameters : offset - offset into either the uncoded lookahead or the
*                         sliding window.
*                buffer - pointer to either the uncoded lookahead or the
*                         sliding window.
*                bufferLen - the size of the buffer pointed to.
*   Effects    : None
*   Returned   : The hash key for the (MAX_UNCODED + 1) long string located
*                at the start of the buffer.
****************************************************************************/
static unsigned int HashKey(const unsigned int offset,
    unsigned char *buffer,
    const unsigned int bufferLen)
{
    unsigned int i;
    unsigned int hashKey;

    hashKey = 0;

    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        hashKey = (hashKey << 5) ^
            buffer[Wrap((offset + i), bufferLen)];
        hashKey %= HASH_SIZE;
    }

    return hashKey;
}

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of mathcing uncoded strings to strings in the
*                sliding window.  For hashed searches, this means that a
*                hash table pointing to linked lists is initialized.
*   Parameters : buffers - pointer to structure with sliding window and
*                          uncoded lookahead buffers
*   Effects    : The hash table and next array are initialized.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
*
*   NOTE: This function assumes that the sliding window is initially filled
*         with all identical characters.
****************************************************************************/
int InitializeSearchStructures(buffers_t *buffers)
{
    unsigned int i;

    /************************************************************************
    * Since the encode routine only fills the sliding window with one
    * character, there is only one hash key for the entier sliding window.
    * That means all positions are in the same linked list.
    ************************************************************************/
    for (i = 0; i < (WINDOW_SIZE - 1); i++)
    {
        next[i] = i + 1;
    }

    /* there's no next for the last character */
    next[WINDOW_SIZE - 1] = NULL_INDEX;

    /* the only list right now is the "   " list */
    for (i = 0; i < HASH_SIZE; i++)
    {
        hashTable[i] = NULL_INDEX;
    }

    hashTable[HashKey(0, buffers->slidingWindow, WINDOW_SIZE)] = 0;

    return 0;
}

/****************************************************************************
*   Function   : FindMatch
*   Description: This function will search through the slidingWindow
*                dictionary for the longest sequence matching the MAX_CODED
*                long string stored in uncodedLookahead.
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
    unsigned int i;
    unsigned int j;

    /* unwrapped copy of uncoded lookahead */
    unsigned char uncoded[MAX_CODED];

    (void)windowHead;       /* prevents unused variable warning */
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

    /* use hash to find the start of the list that we need to check */
    i = hashTable[HashKey(0, uncoded, MAX_CODED)];
    j = 0;

    while (i != NULL_INDEX)
    {
        if (buffers->slidingWindow[i] == uncoded[0])
        {
            /* we matched one how many more match? */
            j = 1;

            while(buffers->slidingWindow[Wrap((i + j), WINDOW_SIZE)] ==
                uncoded[j])
            {
                j++;

                if (j == uncodedLen)
                {
                    break;
                }
            }

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

        i = next[i];    /* try next in list */
    }

    return matchData;
}

/****************************************************************************
*   Function   : AddString
*   Description: This function adds the (MAX_UNCODED + 1) long string
*                starting at slidingWindow[charIndex] to the hash table's
*                linked list associated with its hash key.
*   Parameters : charIndex - sliding window index of the string to be
*                            added to the linked list.
*   Effects    : The string starting at slidingWindow[charIndex] is appended
*                to the end of the appropriate linked list.
*   Returned   : NONE
****************************************************************************/
static void AddString(unsigned char *slidingWindow,
    const unsigned int charIndex)
{
    unsigned int i;
    unsigned int hashKey;

    /* inserted character will be at the end of the list */
    next[charIndex] = NULL_INDEX;

    hashKey = HashKey(charIndex, slidingWindow, WINDOW_SIZE);

    if (hashTable[hashKey] == NULL_INDEX)
    {
        /* this is the only character in it's list */
        hashTable[hashKey] = charIndex;
        return;
    }

    /* find the end of the list */
    i = hashTable[hashKey];

    while(next[i] != NULL_INDEX)
    {
        i = next[i];
    }

    /* add new character to the list end */
    next[i] = charIndex;
}

/****************************************************************************
*   Function   : RemoveString
*   Description: This function removes the (MAX_UNCODED + 1) long string
*                starting at slidingWindow[charIndex] from the hash table's
*                linked list associated with its hash key.
*   Parameters : charIndex - sliding window index of the string to be
*                            removed from the linked list.
*   Effects    : The string starting at slidingWindow[charIndex] is removed
*                from its linked list.
*   Returned   : NONE
****************************************************************************/
static void RemoveString(unsigned char *slidingWindow,
    const unsigned int charIndex)
{
    unsigned int i;
    unsigned int hashKey;
    unsigned int nextIndex;

    nextIndex = next[charIndex];        /* remember where this points to */
    next[charIndex] = NULL_INDEX;

    hashKey = HashKey(charIndex, slidingWindow, WINDOW_SIZE);

    if (hashTable[hashKey] == charIndex)
    {
        /* we're deleting a list head */
        hashTable[hashKey] = nextIndex;
        return;
    }

    /* find character pointing to ours */
    i = hashTable[hashKey];

    while(next[i] != charIndex)
    {
        i = next[i];
    }

    /* point the previous next */
    next[i] = nextIndex;
}

/****************************************************************************
*   Function   : ReplaceChar
*   Description: This function replaces the character stored in
*                slidingWindow[charIndex] with the one specified by
*                replacement.  The hash table entries effected by the
*                replacement are also corrected.
*   Parameters : slidingWindow - pointer to the head of the sliding window.
*                charIndex - sliding window index of the character to be
*                            removed from the linked list.
*                replacement - new character
*   Effects    : slidingWindow[charIndex] is replaced by replacement.  Old
*                hash entries for strings containing slidingWindow[charIndex]
*                are removed and new ones are added.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int ReplaceChar(unsigned char *slidingWindow,
    const unsigned int charIndex,
    const unsigned char replacement)
{
    unsigned int firstIndex;
    unsigned int i;

    if (charIndex < MAX_UNCODED)
    {
        firstIndex = (WINDOW_SIZE + charIndex) - MAX_UNCODED;
    }
    else
    {
        firstIndex = charIndex - MAX_UNCODED;
    }

    /* remove all hash entries containing character at char index */
    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        RemoveString(slidingWindow, Wrap((firstIndex + i), WINDOW_SIZE));
    }

    slidingWindow[charIndex] = replacement;

    /* add all hash entries containing character at char index */
    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        AddString(slidingWindow, Wrap((firstIndex + i), WINDOW_SIZE));
    }

    return 0;
}
