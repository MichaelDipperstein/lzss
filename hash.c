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
*   UPDATES
*   $Id: hash.c,v 1.1 2004/02/22 17:23:02 michael Exp $
*   $Log: hash.c,v $
*   Revision 1.1  2004/02/22 17:23:02  michael
*   Initial revision of hash table search.  Mostly code from lzhash.c.
*
*
****************************************************************************
*
* Hash: Hash table optimized matching routines used by LZSS
*       Encoding/Decoding Routine
* Copyright (C) 2004 by Michael Dipperstein (mdipper@cs.ucsb.edu)
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#define HASH_SIZE       1024     /* size of hash table */

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

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
*                uncoded lookahead.  The key generation algorithm is supposed
*                identical to the algorithm used by gzip.  As reported in
*                K. Sadakane, H. Imai. "Improving the Speed of LZ77
*                Compression by Hashing and Suffix Sorting". IEICE Trans.
*                Fundamentals, Vol. E83-A, No. 12 (December 2000)
*   Parameters : offset - offset into either the uncoded lookahead or the
*                         sliding window.
*                lookahead - TRUE iff offset is an offset into the uncoded
*                            lookahead buffer.
*   Effects    : NONE
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
int HashKey(int offset, char lookahead)
{
    int i, hashKey;

    hashKey = 0;

    if (lookahead)
    {
        /* string is in the lookahead buffer */
        for (i = 0; i < (MAX_UNCODED + 1); i++)
        {
            hashKey = (hashKey << 5) ^ (uncodedLookahead[offset]);
            hashKey %= HASH_SIZE;
            offset = (offset + 1) % MAX_CODED;
        }
    }
    else
    {
        /* string is in the sliding window */
        for (i = 0; i < (MAX_UNCODED + 1); i++)
        {
            hashKey = (hashKey << 5) ^ (slidingWindow[offset]);
            hashKey %= HASH_SIZE;
            offset = (offset + 1) % WINDOW_SIZE;
        }
    }

    return hashKey;
}

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of mathcing uncoded strings to strings in the
*                sliding window.  For hashed searches, this means that a
*                hash table pointing to linked lists is initialized.
*   Parameters : None
*   Effects    : None
*   Returned   : None
*
*   NOTE: This function assumes that the sliding window is initially filled
*         with all identical characters.
****************************************************************************/
void InitializeSearchStructures()
{
    int i;

    /************************************************************************
    * Since the encode routine only fills the sliding window one character,
    * there is only one hash key for the entier sliding window.  That means
    * all positions are in the same linked list.
    ************************************************************************/
    for (i = 0; i < WINDOW_SIZE; i++)
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

    hashTable[HashKey(0, FALSE)] = 0;
}

/****************************************************************************
*   Function   : FindMatch
*   Description: This function will search through the slidingWindow
*                dictionary for the longest sequence matching the MAX_CODED
*                long string stored in uncodedLookahead.
*   Parameters : uncodedHead - head of uncoded lookahead buffer
*   Effects    : NONE
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
encoded_string_t FindMatch(int windowHead, int uncodedHead)
{
    encoded_string_t matchData;
    int i, j;

    matchData.length = 0;
    i = hashTable[HashKey(uncodedHead, TRUE)];  /* start of proper list */
    j = 0;

    while (i != NULL_INDEX)
    {
        if (slidingWindow[i] == uncodedLookahead[uncodedHead])
        {
            /* we matched one how many more match? */
            j = 1;

            while(slidingWindow[(i + j) % WINDOW_SIZE] ==
                uncodedLookahead[(uncodedHead + j) % MAX_CODED])
            {
                if (j >= MAX_CODED)
                {
                    break;
                }
                j++;
            };

            if (j > matchData.length)
            {
                matchData.length = j;
                matchData.offset = i;
            }
        }

        if (j >= MAX_CODED)
        {
            matchData.length = MAX_CODED;
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
void AddString(int charIndex)
{
    int i, hashKey;

    /* inserted character will be at the end of the list */
    next[charIndex] = NULL_INDEX;

    hashKey = HashKey(charIndex, FALSE);

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
void RemoveString(int charIndex)
{
    int i, hashKey;
    int nextIndex;

    nextIndex = next[charIndex];        /* remember where this points to */
    next[charIndex] = NULL_INDEX;

    hashKey = HashKey(charIndex, FALSE);

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
*   Parameters : charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] is replaced by replacement.  Old
*                hash entries for strings containing slidingWindow[charIndex]
*                are removed and new ones are added.
*   Returned   : NONE
****************************************************************************/
void ReplaceChar(int charIndex, unsigned char replacement)
{
    int firstIndex, i;

    firstIndex = charIndex - (MAX_UNCODED + 1);

    if (firstIndex < 0)
    {
        firstIndex = WINDOW_SIZE + firstIndex;
    }

    /* remove all hash entries containing character at char index */
    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        RemoveString((firstIndex + i) % WINDOW_SIZE);
    }

    slidingWindow[charIndex] = replacement;

    /* add all hash entries containing character at char index */
    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        AddString((firstIndex + i) % WINDOW_SIZE);
    }
}
