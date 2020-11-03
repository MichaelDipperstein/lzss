/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : list.c
*   Purpose : Implement linked list optimized matching of uncoded strings
*             for LZSS algorithm.
*   Author  : Michael Dipperstein
*   Date    : February 18, 2004
*
****************************************************************************
*
* List: Linked list optimized matching routines used by LZSS
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
*                                CONSTANTS
***************************************************************************/
#define NULL_INDEX      (WINDOW_SIZE + 1)

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
unsigned int lists[UCHAR_MAX + 1];          /* heads of linked lists */
unsigned int next[WINDOW_SIZE];             /* indices of next in list */

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of mathcing uncoded strings to strings in the
*                sliding window.  For link list optimized searches, this
*                means that linked lists of strings all starting with
*                the same character are initialized.
*   Parameters : buffers - pointer to structure with sliding window and
*                          uncoded lookahead buffers
*   Effects    : Initializes lists and next array
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
*
*   NOTE: This function assumes that the sliding window is initially filled
*         with all identical characters.
****************************************************************************/
int InitializeSearchStructures(buffers_t *buffers)
{
    unsigned int i;

    for (i = 0; i < WINDOW_SIZE - 1; i++)
    {
        next[i] = i + 1;
    }

    /* there's no next for the last character */
    next[WINDOW_SIZE - 1] = NULL_INDEX;

    /* the only list right now is the slidingWindow[0] list */
    for (i = 0; i < UCHAR_MAX + 1; i++)
    {
        lists[i] = NULL_INDEX;
    }

    lists[buffers->slidingWindow[0]] = 0;

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

    /* get the index for strings starting with the first character */
    i = lists[buffers->uncodedLookahead[uncodedHead]];

    while (i != NULL_INDEX)
    {
        /* the list insures we matched one, how many more match? */
        j = 1;

        while(buffers->slidingWindow[Wrap((i + j), WINDOW_SIZE)] == uncoded[j])
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

        if (j == uncodedLen)
        {
            break;
        }

        i = next[i];    /* try next in list */
    }

    return matchData;
}

/****************************************************************************
*   Function   : AddChar
*   Description: This function adds the character stored in
*                slidingWindow[charIndex] to the linked lists.
*   Parameters : slidingWindow - pointer to the head of the sliding window.
*                charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] appended to the end of the
*                appropriate linked list.
*   Returned   : NONE
****************************************************************************/
static void AddChar(unsigned char *slidingWindow,
    const unsigned int charIndex)
{
    unsigned int i;

    /* inserted character will be at the end of the list */
    next[charIndex] = NULL_INDEX;

    if (lists[slidingWindow[charIndex]] == NULL_INDEX)
    {
        /* this is the only character in it's list */
        lists[slidingWindow[charIndex]] = charIndex;
        return;
    }

    /* find the end of the list */
    i = lists[slidingWindow[charIndex]];
    while(next[i] != NULL_INDEX)
    {
        i = next[i];
    }

    /* add new character to the list end */
    next[i] = charIndex;
}

/****************************************************************************
*   Function   : RemoveChar
*   Description: This function removes the character stored in
*                slidingWindow[charIndex] from the linked lists.
*   Parameters : slidingWindow - pointer to the head of the sliding window.
*                charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] is removed from it's linked list
*                and the list is appropriately reconnected.
*   Returned   : NONE
****************************************************************************/
static void RemoveChar(unsigned char *slidingWindow,
    const unsigned int charIndex)
{
    unsigned int i;
    unsigned int nextIndex;

    nextIndex = next[charIndex];        /* remember where this points to */
    next[charIndex] = NULL_INDEX;

    if (lists[slidingWindow[charIndex]] == charIndex)
    {
        /* we're deleting a list head */
        lists[slidingWindow[charIndex]] = nextIndex;
        return;
    }

    /* find character pointing to ours */
    i = lists[slidingWindow[charIndex]];

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
*                replacement.  The linked list entries effected by the
*                replacement are also corrected.
*   Parameters : slidingWindow - pointer to the head of the sliding window.
*                charIndex - sliding window index of the character to be
*                            removed from the linked list.
*                replacement - new character
*   Effects    : slidingWindow[charIndex] is replaced by replacement.  Old
*                list entries for strings containing slidingWindow[charIndex]
*                are removed and new ones are added.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int ReplaceChar(unsigned char *slidingWindow,
    const unsigned int charIndex,
    const unsigned char replacement)
{
    RemoveChar(slidingWindow, charIndex);
    slidingWindow[charIndex] = replacement;
    AddChar(slidingWindow, charIndex);

    return 0;
}
