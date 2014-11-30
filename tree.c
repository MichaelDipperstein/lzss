/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : tree.c
*   Purpose : Implement a binary tree used by the LZSS algorithm to search
*             for matching of uncoded strings in the sliding window.
*   Author  : Michael Dipperstein
*   Date    : August 16, 2010
*
****************************************************************************
*
* Tree: Tree table optimized matching routines used by LZSS
*       Encoding/Decoding Routine
* Copyright (C) 2010, 2014 by
* Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
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
#include <stdio.h>
#include <ctype.h>
#include "lzlocal.h"

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define ROOT_INDEX      (WINDOW_SIZE + 1)
#define NULL_INDEX      (ROOT_INDEX + 1)

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
* This data structure represents the node of a binary search tree.  The
* leftChild, rightChild, and parent fields should contain the index of the
* slidingWindow dictionary where the left child, right child, and parent
* strings begin.
***************************************************************************/
typedef struct tree_node_t
{
    unsigned int leftChild;
    unsigned int rightChild;
    unsigned int parent;
} tree_node_t;

/***************************************************************************
*                                 MACROS
***************************************************************************/

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

tree_node_t tree[WINDOW_SIZE];      /* tree[n] is node for slidingWindow[n] */
unsigned int treeRoot;              /* index of the root of the tree */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
static void ClearNode(const unsigned int index);

/* add/remove strings starting at slidingWindow[charIndex] too/from tree */
static void AddString(const unsigned int charIndex);
static void RemoveString(const unsigned int charIndex);

/* debugging functions not used by algorithm */
static void PrintLen(const unsigned int charIndex, const unsigned int len);
static void DumpTree(const unsigned int root);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of matching uncoded strings to strings in the
*                sliding window.  For binary searches, this means that the
*                last MAX_CODED length string in the sliding window will be
*                made the root of the tree, and have no children.  This only
*                works if the sliding window is filled with identical
*                symbols.
*   Parameters : None
*   Effects    : A tree consisting of just a root node is created.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
*
*   NOTE: This function assumes that the sliding window is initially filled
*         with all identical characters.
****************************************************************************/
int InitializeSearchStructures(void)
{
    unsigned int i;

    /* clear out all tree node pointers */
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        ClearNode(i);
    }

    /************************************************************************
    * Since the encode routine only fills the sliding window with one
    * character, there are only possible MAX_CODED length strings in the
    * tree.  Use the newest of those strings at the tree root.
    ************************************************************************/
    treeRoot = (WINDOW_SIZE - MAX_CODED) - 1;
    tree[treeRoot].parent = ROOT_INDEX;

    if (0)
    {
        /* get rid of unused warning for DumpTree */
        DumpTree(NULL_INDEX);
    }

    return 0;
}

/****************************************************************************
*   Function   : FindMatch
*   Description: This function will search through the slidingWindow
*                dictionary for the longest sequence matching the MAX_CODED
*                long string stored in uncodedLookahead.
*   Parameters : windowHead - not used
*                uncodedHead - head of uncoded lookahead buffer
*   Effects    : NONE
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
encoded_string_t FindMatch(const unsigned int windowHead,
    const unsigned int uncodedHead)
{
    encoded_string_t matchData;
    unsigned int i;
    unsigned int j;
    int compare;

    (void)windowHead;       /* prevents unused variable warning */
    matchData.length = 0;
    matchData.offset = 0;

    i = treeRoot;       /* start at root */
    j = 0;

    while (i != NULL_INDEX)
    {
        compare = slidingWindow[i] - uncodedLookahead[uncodedHead];

        if (0 == compare)
        {
            /* we matched the first symbol, how many more match? */
            j = 1;

            while((compare = slidingWindow[Wrap((i + j), WINDOW_SIZE)] -
                uncodedLookahead[Wrap((uncodedHead + j), MAX_CODED)]) == 0)
            {
                if (j >= MAX_CODED)
                {
                    break;
                }
                j++;
            }

            if (j > matchData.length)
            {
                matchData.length = j;
                matchData.offset = i;
            }
        }

        if (j >= MAX_CODED)
        {
            /* we found the largest allowed match */
            matchData.length = MAX_CODED;
            break;
        }

        if (compare > 0)
        {
            /* branch left and look for a longer match */
            i = tree[i].leftChild;
        }
        else
        {
            /* branch right and look for a longer match */
            i = tree[i].rightChild;
        }
    }

    return matchData;
}

/****************************************************************************
*   Function   : CompareString
*   Description: This function will compare two MAX_CODED long strings in
*                the slidingWindow dictionary.
*   Parameters : index1 - slidingWindow index where the first string starts
*                index2 - slidingWindow index where the second string starts
*   Effects    : NONE
*   Returned   : 0 if first string equals second string.
*                < 0 if first string is less than second string.
*                > 0 if first string is greater than second string.
****************************************************************************/
static int CompareString(const unsigned int index1, const unsigned int index2)
{
    unsigned int offset;
    int result = 0;

    for (offset = 0; offset < MAX_CODED; offset++)
    {
        result = slidingWindow[Wrap((index1 + offset), WINDOW_SIZE)] -
            slidingWindow[Wrap((index2 + offset), WINDOW_SIZE)];

        if (result != 0)
        {
            break;      /* we have a mismatch */
        }
    }

    return result;
}

/****************************************************************************
*   Function   : FixChildren
*   Description: This function reattaches the children to a parent node
*                after it has been inserted.
*   Parameters : index - sliding window index of the parent node.
*   Effects    : The .parent fields for the children of a newly attached
*                node are made to point to the newly attached node.
*   Returned   : NONE
****************************************************************************/
static void FixChildren(const unsigned int index)
{
    if (tree[index].leftChild != NULL_INDEX)
    {
        tree[tree[index].leftChild].parent = index;
    }
    
    if (tree[index].rightChild != NULL_INDEX)
    {
        tree[tree[index].rightChild].parent = index;
    }
}

/****************************************************************************
*   Function   : AddString
*   Description: This function adds the MAX_UNCODED long string starting at
*                slidingWindow[charIndex] to the binary tree.
*   Parameters : charIndex - sliding window index of the string to be
*                            added to the binary tree list.
*   Effects    : The string starting at slidingWindow[charIndex] is inserted
*                into the sorted binary tree.
*   Returned   : NONE
****************************************************************************/
static void AddString(const unsigned int charIndex)
{
    int compare;
    unsigned int here;

    compare = CompareString(charIndex, treeRoot);

    if (0 == compare)
    {
        /* make start the new root, because it's newer identical */
        tree[charIndex].leftChild = tree[treeRoot].leftChild;
        tree[charIndex].rightChild = tree[treeRoot].rightChild;
        tree[charIndex].parent = ROOT_INDEX;
        FixChildren(charIndex);

        /* remove old root from the tree */
        ClearNode(treeRoot);

        treeRoot = charIndex;
        return;
    }

    here = treeRoot;

    while(1)
    {
        if (compare < 0)
        {
            /* branch left for < */
            if (tree[here].leftChild != NULL_INDEX)
            {
                here = tree[here].leftChild;
            }
            else
            {
                /* we've hit the bottom */
                tree[here].leftChild = charIndex;
                tree[charIndex].leftChild = NULL_INDEX;
                tree[charIndex].rightChild = NULL_INDEX;
                tree[charIndex].parent = here;
                FixChildren(charIndex);
                return;
            }
        }
        else if (compare > 0)
        {
            /* branch right for > */
            if (tree[here].rightChild != NULL_INDEX)
            {
                here = tree[here].rightChild;
            }
            else
            {
                /* we've hit the bottom */
                tree[here].rightChild = charIndex;
                tree[charIndex].leftChild = NULL_INDEX;
                tree[charIndex].rightChild = NULL_INDEX;
                tree[charIndex].parent = here;
                FixChildren(charIndex);
                return;
            }
        }
        else
        {
            /* identical strings.  replace old with new */
            tree[charIndex].leftChild = tree[here].leftChild;
            tree[charIndex].rightChild = tree[here].rightChild;
            tree[charIndex].parent = tree[here].parent;
            FixChildren(charIndex);

            if (tree[tree[here].parent].leftChild == here)
            {
                tree[tree[here].parent].leftChild = charIndex;
            }
            else
            {
                tree[tree[here].parent].rightChild = charIndex;
            }

            /* remove old node from the tree */
            ClearNode(here);
            return;
        }

        compare = CompareString(charIndex, here);
    }
}

/****************************************************************************
*   Function   : RemoveString
*   Description: This function removes the MAX_UNCODED long string starting
*                at slidingWindow[charIndex] from the binary tree.
*   Parameters : charIndex - sliding window index of the string to be
*                            removed from the binary tree list.
*   Effects    : The string starting at slidingWindow[charIndex] is removed
*                from the sorted binary tree.
*   Returned   : NONE
****************************************************************************/
static void RemoveString(const unsigned int charIndex)
{
    unsigned int here;

    if (NULL_INDEX == tree[charIndex].parent)
    {
        return;     /* string isn't in tree */
    }

    if (NULL_INDEX == tree[charIndex].rightChild)
    {
        /* node doesn't have right child.  promote left child. */
        here = tree[charIndex].leftChild;
    }
    else if (NULL_INDEX == tree[charIndex].leftChild)
    {
        /* node doesn't have left child.  promote right child. */
        here = tree[charIndex].rightChild;
    }
    else
    {
        /* promote rightmost left descendant */
        here = tree[charIndex].leftChild;

        while (tree[here].rightChild != NULL_INDEX)
        {
            here = tree[here].rightChild;
        }

        if (here != tree[charIndex].leftChild)
        {
            /* there was a right branch to follow and we're at the end */
            tree[tree[here].parent].rightChild = tree[here].leftChild;
            tree[tree[here].leftChild].parent = tree[here].parent;
            tree[here].leftChild = tree[charIndex].leftChild;
            tree[tree[charIndex].leftChild].parent = here;
        }

        tree[here].rightChild = tree[charIndex].rightChild;
        tree[tree[charIndex].rightChild].parent = here;
    }

    if (tree[tree[charIndex].parent].leftChild == charIndex)
    {
        tree[tree[charIndex].parent].leftChild = here;
    }
    else
    {
        tree[tree[charIndex].parent].rightChild = here;
    }

    tree[here].parent = tree[charIndex].parent;

    if (treeRoot == charIndex)
    {
        treeRoot = here;
    }

    /* clear all pointers in deleted node. */
    ClearNode(charIndex);
}

/****************************************************************************
*   Function   : ReplaceChar
*   Description: This function replaces the character stored in
*                slidingWindow[charIndex] with the one specified by
*                replacement.  The binary tree entries effected by the
*                replacement are also corrected.
*   Parameters : charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] is replaced by replacement.  Old
*                binary tree nodes for strings containing
*                slidingWindow[charIndex] are removed and new ones are
*                added.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    unsigned int firstIndex, i;

    if (charIndex < MAX_CODED)
    {
        firstIndex = (WINDOW_SIZE + charIndex) - MAX_CODED;
    }
    else
    {
        firstIndex = charIndex - MAX_CODED;
    }

    /* remove all tree entries containing character at char index */
    for (i = 0; i <= MAX_CODED; i++)
    {
        RemoveString(Wrap((firstIndex + i), WINDOW_SIZE));
    }

    slidingWindow[charIndex] = replacement;

    /* add all hash entries containing character at char index */
    for (i = 0; i <= MAX_CODED; i++)
    {
        AddString(Wrap((firstIndex + i), WINDOW_SIZE));
    }

    return 0;
}

/****************************************************************************
*   Function   : ClearNode
*   Description: This function sets the children and parent of a node in
*                the binary tree to NULL_INDEX.
*   Parameters : index - index of the tree node to be cleared.
*   Effects    : tree[index] is set to {NULL_INDEX, NULL_INDEX, NULL_INDEX}.
*   Returned   : None
****************************************************************************/
static void ClearNode(const unsigned int index)
{
    const tree_node_t nullNode = {NULL_INDEX, NULL_INDEX, NULL_INDEX};

    tree[index] = nullNode;
}

/****************************************************************************
*   Function   : PrintLen
*   Description: This function prints the string of length len that starts at
*                slidingWindow[charIndex].
*   Parameters : charIndex - sliding window index of the string to be
*                            printed.
*                len - length of the string to be printed.
*   Effects    : The string of length len starting at
*                slidingWindow[charIndex] is printed to stdout.
*   Returned   : NONE
****************************************************************************/
static void PrintLen(const unsigned int charIndex, const unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
    {
        if (isprint(slidingWindow[Wrap((i + charIndex), WINDOW_SIZE)]))
        {
            putchar(slidingWindow[Wrap((i + charIndex), WINDOW_SIZE)]);
        }
        else
        {
            printf("<%02X>", slidingWindow[Wrap((i + charIndex), WINDOW_SIZE)]);
        }
    }
}

/****************************************************************************
*   Function   : DumpTree
*   Description: This function dumps the contents of the (sub)tree starting
*                at node 'root' to stdout.
*   Parameters : root - root node for subtree to be dumped
*   Effects    : The nodes contents of the (sub)tree rooted at node 'root' 
*                are printed to stdout.
*   Returned   : NONE
****************************************************************************/
static void DumpTree(const unsigned int root)
{
    if (NULL_INDEX == root)
    {
        /* empty tree */
        return;
    }

    if (tree[root].leftChild != NULL_INDEX)
    {
        DumpTree(tree[root].leftChild);
    }

    printf("%03d: ", root);
    PrintLen(root, MAX_CODED);
    printf("\n");

    if (tree[root].rightChild != NULL_INDEX)
    {
        DumpTree(tree[root].rightChild);
    }
}
