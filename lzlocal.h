/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : lzlocal.h
*   Purpose : Internal headers for LZSS encode and decode routines.
*             Contains the prototypes to be used by the different match
*             finding algorithms.
*   Author  : Michael Dipperstein
*   Date    : February 18, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: lzlocal.h,v 1.1 2004/02/22 17:32:40 michael Exp $
*   $Log: lzlocal.h,v $
*   Revision 1.1  2004/02/22 17:32:40  michael
*   Initial revision of header files for sliding window search implementations.
*
*
****************************************************************************
*
* LZSS: An ANSI C LZSS Encoding/Decoding Routine
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
#ifndef _LZSS_LOCAL_H
#define _LZSS_LOCAL_H

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
/* unpacked encoded offset and length, gets packed into 12 bits and 4 bits*/
typedef struct encoded_string_t
{
    int offset;     /* offset to start of longest match */
    int length;     /* length of longest match */
} encoded_string_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define FALSE   0
#define TRUE    1

#define WINDOW_SIZE     4096   /* size of sliding window (12 bits) */

/* maximum match length not encoded and encoded (4 bits) */
#define MAX_UNCODED     2
#define MAX_CODED       (15 + MAX_UNCODED + 1)

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
void InitializeSearchStructures(void);

encoded_string_t FindMatch(int windowHead, int uncodedHead);

void ReplaceChar(int charIndex, unsigned char replacement);

#endif      /* ndef _LZSS_LOCAL_H */
