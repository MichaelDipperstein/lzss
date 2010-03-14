/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : lzss.c
*   Purpose : Use lzss coding (Storer and Szymanski's modified LZ77) to
*             compress/decompress files.
*   Author  : Michael Dipperstein
*   Date    : November 24, 2003
*
****************************************************************************
*   UPDATES
*
*   Date        Change
*   12/10/03    Changed handling of sliding window to better match standard
*               algorithm description.
*   12/11/03    Remebered to copy encoded characters to the sliding window
*               even when there are no more characters in the input stream.
*   $Id: lzss.c,v 1.2 2004/02/22 17:14:26 michael Exp $
*   $Log: lzss.c,v $
*   Revision 1.2  2004/02/22 17:14:26  michael
*   - Separated encode/decode, match finding, and main.
*   - Use bitfiles for reading/writing files
*   - Use traditional LZSS encoding where the coded/uncoded bits
*     precede the symbol they are associated with, rather than
*     aggregating the bits.
*
*   Revision 1.1.1.1  2004/01/21 06:25:49  michael
*   Initial version
*
*
****************************************************************************
*
* LZSS: An ANSI C LZSS Encoding/Decoding Routines
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
#include <string.h>
#include "lzlocal.h"
#include "bitfile.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define ENCODED     0       /* encoded string */
#define UNCODED     1       /* unencoded character */

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
unsigned char slidingWindow[WINDOW_SIZE];
unsigned char uncodedLookahead[MAX_CODED];

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/
/****************************************************************************
*   Function   : EncodeLZSS
*   Description: This function will read an input file and write an output
*                file encoded according to the traditional LZSS algorithm.
*                This algorithm encodes strings as 16 bits (a 12 bit offset
*                + a 4 bit length).
*   Parameters : inFile - name of file to encode
*                outFile - name of file to write encoded output
*   Effects    : inFile is encoded and written to outFile
*   Returned   : NONE
****************************************************************************/
void EncodeLZSS(char *inFile, char *outFile)
{
    FILE *fpIn;
    bit_file_t *bfpOut;

    encoded_string_t matchData;
    int i, c;
    int len;                        /* length of string */
    int windowHead, uncodedHead;    /* head of sliding window and lookahead */

    /* open binary input and output files */
    if ((fpIn = fopen(inFile, "rb")) == NULL)
    {
        perror(inFile);
        exit(EXIT_FAILURE);
    }

    if (outFile == NULL)
    {
        bfpOut = MakeBitFile(stdout, BF_WRITE);
    }
    else
    {
        if ((bfpOut = BitFileOpen(outFile, BF_WRITE)) == NULL)
        {
            perror(outFile);
            fclose(fpIn);
            exit(EXIT_FAILURE);
        }
    }

    windowHead = 0;
    uncodedHead = 0;

    /************************************************************************
    * Fill the sliding window buffer with some known vales.  DecodeLZSS must
    * use the same values.  If common characters are used, there's an
    * increased chance of matching to the earlier strings.
    ************************************************************************/
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        slidingWindow[i] = ' ';
    }

    /************************************************************************
    * Copy MAX_CODED bytes from the input file into the uncoded lookahead
    * buffer.
    ************************************************************************/
    for (len = 0; len < MAX_CODED && (c = getc(fpIn)) != EOF; len++)
    {
        uncodedLookahead[len] = c;
    }

    if (len == 0)
    {
        return;  /* inFile was empty */
    }

    /* Look for matching string in sliding window */
    InitializeSearchStructures();
    matchData = FindMatch(windowHead, uncodedHead);

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
            /* not long enough match.  write uncoded flag and character */
            BitFilePutBit(UNCODED, bfpOut);
            BitFilePutChar(uncodedLookahead[uncodedHead], bfpOut);
            
            matchData.length = 1;   /* set to 1 for 1 byte uncoded */
        }
        else
        {
            /* match length > MAX_UNCODED.  Encode as offset and length. */
            BitFilePutBit(ENCODED, bfpOut);
            BitFilePutChar((unsigned char)((matchData.offset & 0x0FFF) >> 4),
                bfpOut);
            BitFilePutChar((unsigned char)(((matchData.offset & 0x000F) << 4) |
                (matchData.length - (MAX_UNCODED + 1))), bfpOut);
        }

        /********************************************************************
        * Replace the matchData.length worth of bytes we've matched in the
        * sliding window with new bytes from the input file.
        ********************************************************************/
        i = 0;
        while ((i < matchData.length) && ((c = getc(fpIn)) != EOF))
        {
            /* add old byte into sliding window and new into lookahead */
            ReplaceChar(windowHead, uncodedLookahead[uncodedHead]);
            uncodedLookahead[uncodedHead] = c;
            windowHead = (windowHead + 1) % WINDOW_SIZE;
            uncodedHead = (uncodedHead + 1) % MAX_CODED;
            i++;
        }

        /* handle case where we hit EOF before filling lookahead */
        while (i < matchData.length)
        {
            ReplaceChar(windowHead, uncodedLookahead[uncodedHead]);
            /* nothing to add to lookahead here */
            windowHead = (windowHead + 1) % WINDOW_SIZE;
            uncodedHead = (uncodedHead + 1) % MAX_CODED;
            len--;
            i++;
        }

        /* find match for the remaining characters */
        matchData = FindMatch(windowHead, uncodedHead);
    }

    /* we've encoded everything, close the files */
    fclose(fpIn);
    BitFileClose(bfpOut);
}

/****************************************************************************
*   Function   : DecodeLZSS
*   Description: This function will read an LZSS encoded input file and
*                write an output file.  This algorithm encodes strings as 16
*                bits (a 12 bit offset + a 4 bit length).
*   Parameters : inFile - name of file to decode
*                outFile - name of file to write decoded output
*   Effects    : inFile is decoded and written to outFile
*   Returned   : NONE
****************************************************************************/
void DecodeLZSS(char *inFile, char *outFile)
{
    bit_file_t *bfpIn;
    FILE *fpOut;

    int  i, c, nextChar;
    encoded_string_t code;              /* offset/length code for string */

    if ((bfpIn = BitFileOpen(inFile, BF_READ)) == NULL)
    {
        perror(inFile);
        exit(EXIT_FAILURE);
        return;
    }

    if (outFile == NULL)
    {
        fpOut = stdout;
    }
    else
    {
        if ((fpOut = fopen(outFile, "wb")) == NULL)
        {
            BitFileClose(bfpIn);
            perror(outFile);
            exit(EXIT_FAILURE);
        }
    }

    /************************************************************************
    * Fill the sliding window buffer with some known vales.  EncodeLZSS must
    * use the same values.  If common characters are used, there's an
    * increased chance of matching to the earlier strings.
    ************************************************************************/
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        slidingWindow[i] = ' ';
    }

    nextChar = 0;

    while (TRUE)
    {
        if ((c = BitFileGetBit(bfpIn)) == EOF)
        {
            /* we hit the EOF */
            break;
        }

        if (c == UNCODED)
        {
            /* uncoded character */
            if ((c = BitFileGetChar(bfpIn)) == EOF)
            {
                break;
            }

            /* write out byte and put it in sliding window */
            putc(c, fpOut);
            slidingWindow[nextChar] = c;
            nextChar = (nextChar + 1) % WINDOW_SIZE;
        }
        else
        {
            /* offset and length */
            if ((code.offset = BitFileGetChar(bfpIn)) == EOF)
            {
                break;
            }

            if ((code.length = BitFileGetChar(bfpIn)) == EOF)
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
                putc(c, fpOut);
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

    /* we've decoded everything, close the files */
    BitFileClose(bfpIn);
    fclose(fpOut);
}
