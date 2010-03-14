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
*   UPDATES
*
*   $Id: list.c,v 1.1 2004/02/22 17:24:02 michael Exp $
*   $Log: list.c,v $
*   Revision 1.1  2004/02/22 17:24:02  michael
*   Initial revision of linked list search.  Mostly code from lzlist.c.
*
*
****************************************************************************
*
* List: Linked list optimized matching routines used by LZSS
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
#include <limits.h>
#include "lzlocal.h"

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define NULL_INDEX      (WINDOW_SIZE + 1)

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

unsigned int lists[UCHAR_MAX];                    /* heads of linked lists */
unsigned int next[WINDOW_SIZE];             /* indices of next in list */

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of mathcing uncoded strings to strings in the
*                sliding window.  For link list optimized searches, this
*                means that linked lists of strings all starting with
*                the same character are initialized.
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

    for (i = 0; i < WINDOW_SIZE; i++)
    {
        next[i] = i + 1;
    }

    /* there's no next for the last character */
    next[WINDOW_SIZE - 1] = NULL_INDEX;

    /* the only list right now is the ' ' list */
    for (i = 0; i < 256; i++)
    {
        lists[i] = NULL_INDEX;
    }

    lists[' '] = 0;
}

/****************************************************************************
*   Function   : FindMatch
*   Description: This function will search through the slidingWindow
*                dictionary for the longest sequence matching the MAX_CODED
*                long string stored in uncodedLookahed.
*   Parameters : windowHead - head of sliding window
*                uncodedHead - head of uncoded lookahead buffer
*   Effects    : None
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
encoded_string_t FindMatch(int windowHead, int uncodedHead)
{
    encoded_string_t matchData;
    int i, j;

    matchData.length = 0;
    i = lists[uncodedLookahead[uncodedHead]];   /* start of proper list */

    while (i != NULL_INDEX)
    {
        /* the list insures we matched one, how many more match? */
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
*   Function   : AddChar
*   Description: This function adds the character stored in
*                slidingWindow[charIndex] to the linked lists.
*   Parameters : charIndex - sliding window index of the character to be
*                            added to the linked list.
*   Effects    : slidingWindow[charIndex] appended to the end of the
*                appropriate linked list.
*   Returned   : NONE
****************************************************************************/
void AddChar(int charIndex)
{
    int i;

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
*   Parameters : charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] is removed from it's linked list
*                and the list is appropriately reconnected.
*   Returned   : NONE
****************************************************************************/
void RemoveChar(int charIndex)
{
    int i;
    int nextIndex;

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
*                replacement.  The hash table entries effected by the
*                replacement are also corrected.
*   Parameters : charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] is replaced by replacement.  Old
*                hash entries for strings containing slidingWindow[charIndex]
*                are removed and new ones are added.
*   Returned   : None
****************************************************************************/
void ReplaceChar(int charIndex, unsigned char replacement)
{
    RemoveChar(charIndex);
    slidingWindow[charIndex] = replacement;
    AddChar(charIndex);
}
