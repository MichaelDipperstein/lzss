/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : lzss.h
*   Purpose : Header for LZSS encode and decode routines.  Contains the
*             prototypes to be used by programs linking to the LZSS
*             library.
*   Author  : Michael Dipperstein
*   Date    : February 21, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: lzss.h,v 1.3 2004/11/13 22:51:00 michael Exp $
*   $Log: lzss.h,v $
*   Revision 1.3  2004/11/13 22:51:00  michael
*   Provide distinct names for by file and by name functions and add some
*   comments to make their usage clearer.
*
*   Revision 1.2  2004/11/08 05:54:18  michael
*   1. Split encode and decode routines for smarter linking
*   2. Renamed lzsample.c sample.c to match my other samples
*   3. Makefile now builds code as libraries for better LGPL compliance.
*
*   Revision 1.1  2004/02/22 17:37:50  michael
*   Initial revision of headers for encode and decode routines.
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
#ifndef _LZSS_H
#define _LZSS_H

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>

/***************************************************************************
*                                 MACROS
***************************************************************************/
/* macros for compatibility with older library */
#define EncodeLZSS(in, out)     EncodeLZSSByName((in), (out))
#define DecodeLZSS(in, out)     DecodeLZSSByName((in), (out))

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
/***************************************************************************
* LZSS encoding and decoding prototypes for functions with file name
* parameters.  Provide these functions with name of the file to be
* encoded/decoded (inFile) and the name of the target file (outFile).
* These functions return EXIT_SUCCESS or EXIT_FAILURE.
***************************************************************************/
int EncodeLZSSByName(char *inFile, char *outFile);
int DecodeLZSSByName(char *inFile, char *outFile);

/***************************************************************************
* LZSS encoding and decoding prototypes for functions with file pointer
* parameters.  Provide these functions with a pointer to the open binary
* file to be encoded/decoded (fpIn) and pointer to the open binary target
* file (fpOut).  It is the job of the function caller to open the files
* prior to callings these functions and to close the file after these
* functions have been called.
* These functions return EXIT_SUCCESS or EXIT_FAILURE.
***************************************************************************/
int EncodeLZSSByFile(FILE *fpIn, FILE *fpOut);
int DecodeLZSSByFile(FILE *fpIn, FILE *fpOut);

#endif      /* ndef _LZSS_H */
