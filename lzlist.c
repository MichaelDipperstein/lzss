/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : lzlist.c
*   Purpose : Use lzss coding (Storer and Szymanski's modified lz77) to
*             compress/decompress files.  Linked lists are used to improve
*             dictionary search time during the encode.  There is one list
*             for each possible character value.
*   Author  : Michael Dipperstein
*   Date    : December 11, 2003
*
****************************************************************************
*
* LZLIST: An ANSI C LZss Encoding/Decoding Routine
* Copyright (C) 2003 by Michael Dipperstein (mdipper@cs.ucsb.edu)
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
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
/* unpacked encoded offset and length, gets packed into 12 bits and 4 bits*/
typedef struct encoded_string_t
{
    int offset;     /* offset to start of longest match */
    int length;     /* length of longest match */
} encoded_string_t;

typedef enum
{
    ENCODE,
    DECODE
} MODES;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define FALSE   0
#define TRUE    1

#define WINDOW_SIZE     4096   /* size of sliding window (12 bits) */
#define NULL_INDEX      (WINDOW_SIZE + 1)

/* maximum match length not encoded and encoded (4 bits) */
#define MAX_UNCODED     2
#define MAX_CODED       (15 + MAX_UNCODED + 1)

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
unsigned char slidingWindow[WINDOW_SIZE];

unsigned char uncodedLookahead[MAX_CODED];  /* characters to be encoded */
unsigned int lists[256];                    /* heads of linked lists */
unsigned int next[WINDOW_SIZE];             /* indices of next in list */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
void EncodeLZSS(FILE *inFile, FILE *outFile);   /* encoding routine */
void DecodeLZSS(FILE *inFile, FILE *outFile);   /* decoding routine */

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it validates
*                the command line input and, if valid, it will either
*                encode a file using the LZss algorithm or decode a
*                file encoded with the LZss algorithm.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Encodes/Decodes input file
*   Returned   : EXIT_SUCCESS for success, otherwise EXIT_FAILURE.
****************************************************************************/
int main(int argc, char *argv[])
{
    int opt;
    FILE *inFile, *outFile;  /* input & output files */
    MODES mode;

    /* initialize data */
    inFile = NULL;
    outFile = NULL;
    mode = ENCODE;

    /* parse command line */
    while ((opt = getopt(argc, argv, "cdtni:o:h?")) != -1)
    {
        switch(opt)
        {
            case 'c':       /* compression mode */
                mode = ENCODE;
                break;

            case 'd':       /* decompression mode */
                mode = DECODE;
                break;

            case 'i':       /* input file name */
                if (inFile != NULL)
                {
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    fclose(inFile);

                    if (outFile != NULL)
                    {
                        fclose(outFile);
                    }

                    exit(EXIT_FAILURE);
                }
                else if ((inFile = fopen(optarg, "rb")) == NULL)
                {
                    perror("Opening inFile");

                    if (outFile != NULL)
                    {
                        fclose(outFile);
                    }

                    exit(EXIT_FAILURE);
                }
                break;

            case 'o':       /* output file name */
                if (outFile != NULL)
                {
                    fprintf(stderr, "Multiple output files not allowed.\n");
                    fclose(outFile);

                    if (inFile != NULL)
                    {
                        fclose(inFile);
                    }

                    exit(EXIT_FAILURE);
                }
                else if ((outFile = fopen(optarg, "wb")) == NULL)
                {
                    perror("Opening outFile");

                    if (outFile != NULL)
                    {
                        fclose(inFile);
                    }

                    exit(EXIT_FAILURE);
                }
                break;

            case 'h':
            case '?':
                printf("Usage: lzlist <options>\n\n");
                printf("options:\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -i <filename> : Name of input file.\n");
                printf("  -o <filename> : Name of output file.\n");
                printf("  -h | ?  : Print out command line options.\n\n");
                printf("Default: lzlist -c\n");
                return(EXIT_SUCCESS);
        }
    }

    /* validate command line */
    if (inFile == NULL)
    {
        fprintf(stderr, "Input file must be provided\n");
        fprintf(stderr, "Enter \"lzlist -?\" for help.\n");

        if (outFile != NULL)
        {
            fclose(outFile);
        }

        exit (EXIT_FAILURE);
    }
    else if (outFile == NULL)
    {
        fprintf(stderr, "Output file must be provided\n");
        fprintf(stderr, "Enter \"lzlist -?\" for help.\n");

        if (inFile != NULL)
        {
            fclose(inFile);
        }

        exit (EXIT_FAILURE);
    }

    /* we have valid parameters encode or decode */
    if (mode == ENCODE)
    {
        EncodeLZSS(inFile, outFile);
    }
    else
    {
        DecodeLZSS(inFile, outFile);
    }

    fclose(inFile);
    fclose(outFile);
    return EXIT_SUCCESS;
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
encoded_string_t FindMatch(int uncodedHead)
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
*   Function   : EncodeLZSS
*   Description: This function will read an input file and write an output
*                file encoded using a slight modification to the LZss
*                algorithm.  I'm not sure who to credit with the slight
*                modification to LZss, but the modification is to group the
*                coded/not coded flag into bytes.  By grouping the flags,
*                the need to be able to write anything other than a byte
*                may be avoided as longs as strings encode as a whole byte
*                multiple.  This algorithm encodes strings as 16 bits (a 12
*                bit offset + a 4 bit length).
*   Parameters : inFile - file to encode
*                outFile - file to write encoded output
*   Effects    : inFile is encoded and written to outFile
*   Returned   : NONE
****************************************************************************/
void EncodeLZSS(FILE *inFile, FILE *outFile)
{
    /* 8 code flags and encoded strings */
    unsigned char flags, flagPos, encodedData[16];
    int nextEncoded;                /* index into encodedData */
    encoded_string_t matchData;
    int i, c;
    int len;                        /* length of string */
    int windowHead, uncodedHead;    /* head of sliding window and lookahead */

    flags = 0;
    flagPos = 0x01;
    nextEncoded = 0;
    windowHead = 0;
    uncodedHead = 0;

    /************************************************************************
    * Copy MAX_CODED bytes from the input file into the uncoded lookahead
    * buffer.
    ************************************************************************/
    for (len = 0; len < MAX_CODED && (c = getc(inFile)) != EOF; len++)
    {
        uncodedLookahead[len] = c;
    }

    if (len == 0)
    {
        return;  /* inFile was empty */
    }

    /************************************************************************
    * Fill the sliding window buffer with some known vales.  DecodeLZSS must
    * use the same values.  If common characters are used, there's an
    * increased chance of matching to the earlier strings.  Since the
    * sliding window is filled with only one character, there is only one
    * linked list and each item points to the character after it.
    ************************************************************************/
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        slidingWindow[i] = ' ';
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

    /* Look for matching string in sliding window */
    matchData = FindMatch(uncodedHead);

    /* now encoded the rest of the file until an EOF is read */
    while (len > 0)
    {
        if (matchData.length > len)
        {
            /* garbage beyond last data happened to extend match length */
            matchData.length = len;
        }

        if (matchData.length <= MAX_UNCODED)
        {
            /* not long enough match.  write uncoded byte */
            matchData.length = 1;   /* set to 1 for 1 byte uncoded */
            flags |= flagPos;       /* mark with uncoded byte flag */
            encodedData[nextEncoded++] = uncodedLookahead[uncodedHead];
        }
        else
        {
            /* match length > MAX_UNCODED.  Encode as offset and length. */
            encodedData[nextEncoded++] =
                (unsigned char)((matchData.offset & 0x0FFF) >> 4);

            encodedData[nextEncoded++] =
                (unsigned char)(((matchData.offset & 0x000F) << 4) |
                (matchData.length - (MAX_UNCODED + 1)));
        }

        if (flagPos == 0x80)
        {
            /* we have 8 code flags, write out flags and code buffer */
            putc(flags, outFile);

            for (i = 0; i < nextEncoded; i++)
            {
                /* send at most 8 units of code together */
                putc(encodedData[i], outFile);
            }

            /* reset encoded data buffer */
            flags = 0;
            flagPos = 0x01;
            nextEncoded = 0;
        }
        else
        {
            /* we don't have 8 code flags yet, use next bit for next flag */
            flagPos <<= 1;
        }

        /********************************************************************
        * Replace the matchData.length worth of bytes we've matched in the
        * sliding window with new bytes from the input file.
        ********************************************************************/
        i = 0;
        while ((i < matchData.length) && ((c = getc(inFile)) != EOF))
        {
            /* add old byte into sliding window and new into lookahead */
            RemoveChar(windowHead);
            slidingWindow[windowHead] = uncodedLookahead[uncodedHead];
            uncodedLookahead[uncodedHead] = c;
            AddChar(windowHead);
            windowHead = (windowHead + 1) % WINDOW_SIZE;
            uncodedHead = (uncodedHead + 1) % MAX_CODED;
            i++;
        }

        /* handle case where we hit EOF before filling lookahead */
        while (i < matchData.length)
        {
            RemoveChar(windowHead);
            slidingWindow[windowHead] = uncodedLookahead[uncodedHead];
            AddChar(windowHead);
            /* nothing to add to lookahead here */
            windowHead = (windowHead + 1) % WINDOW_SIZE;
            uncodedHead = (uncodedHead + 1) % MAX_CODED;
            len--;
            i++;
        }

        /* find match for the remaining characters */
        matchData = FindMatch(uncodedHead);
    }

    /* write out any remaining encoded data */
    if (nextEncoded != 0)
    {
        putc(flags, outFile);

        for (i = 0; i < nextEncoded; i++)
        {
            putc(encodedData[i], outFile);
        }
    }
}

/****************************************************************************
*   Function   : DecodeLZSS
*   Description: This function will read an LZss encoded input file and
*                write an output file.  The encoded file uses a slight
*                modification to the LZss algorithm.  I'm not sure who to
*                credit with the slight modification to LZss, but the
*                modification is to group the coded/not coded flag into
*                bytes.  By grouping the flags, the need to be able to
*                write anything other than a byte may be avoided as longs
*                as strings encode as a whole byte multiple.  This algorithm
*                encodes strings as 16 bits (a 12bit offset + a 4 bit length).
*   Parameters : inFile - file to decode
*                outFile - file to write decoded output
*   Effects    : inFile is decoded and written to outFile
*   Returned   : NONE
****************************************************************************/
void DecodeLZSS(FILE *inFile, FILE *outFile)
{
    int  i, c;
    unsigned char flags, flagsUsed;     /* encoded/not encoded flag */
    int nextChar;                       /* next char in sliding window */
    encoded_string_t code;              /* offset/length code for string */

    /* initialize variables */
    flags = 0;
    flagsUsed = 7;
    nextChar = 0;

    /************************************************************************
    * Fill the sliding window buffer with some known vales.  EncodeLZSS must
    * use the same values.  If common characters are used, there's an
    * increased chance of matching to the earlier strings.
    ************************************************************************/
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        slidingWindow[i] = ' ';
    }

    while (TRUE)
    {
        flags >>= 1;
        flagsUsed++;

        if (flagsUsed == 8)
        {
            /* shifted out all the flag bits, read a new flag */
            if ((c = getc(inFile)) == EOF)
            {
                break;
            }

            flags = c & 0xFF;
            flagsUsed = 0;
        }

        if (flags & 0x01)
        {
            /* uncoded character */
            if ((c = getc(inFile)) == EOF)
            {
                break;
            }

            /* write out byte and put it in sliding window */
            putc(c, outFile);
            slidingWindow[nextChar] = c;
            nextChar = (nextChar + 1) % WINDOW_SIZE;
        }
        else
        {
            /* offset and length */
            if ((code.offset = getc(inFile)) == EOF)
            {
                break;
            }

            if ((code.length = getc(inFile)) == EOF)
            {
                break;
            }

            /* unpack offset and length */
            code.offset <<= 4;
            code.offset |= ((code.length & 0x00F0) >> 4);
            code.length = (code.length & 0x000F) + MAX_UNCODED + 1;

            /****************************************************************
            * Write out decoded string to file and lookahead.  It would be
            * nice to write to the sliding window instead of the lookahead,
            * but we could end up overwriting the matching string with the
            * new string if abs(offset - next char) < match length.
            ****************************************************************/
            for (i = 0; i < code.length; i++)
            {
                c = slidingWindow[(code.offset + i) % WINDOW_SIZE];
                putc(c, outFile);
                uncodedLookahead[i] = c;
            }

            /* write out decoded string to sliding window */
            for (i = 0; i < code.length; i++)
            {
                slidingWindow[(nextChar + i) % WINDOW_SIZE] =
                    uncodedLookahead[i];
            }

            nextChar = (nextChar + code.length) % WINDOW_SIZE;
        }
    }
}

