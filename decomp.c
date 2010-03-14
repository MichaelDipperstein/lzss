/***************************************************************************
*                     Sample Program Using LZSS Library
*
*   File    : decomp.c
*   Purpose : Demonstrate usage of LZSS library to decompress a file
*   Author  : Michael Dipperstein
*   Date    : November 7, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: decomp.c,v 1.1 2004/11/08 06:06:07 michael Exp $
*   $Log: decomp.c,v $
*   Revision 1.1  2004/11/08 06:06:07  michael
*   Initial revision of super simple compression and decompression samples.
*
*
****************************************************************************
*
* DECOMP: Super simple sample demonstraiting use of LZSS decompression
*         routine.
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
#include <stdio.h>
#include "lzss.h"

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : main
*   Description: This is the main function for this program it calls the
*                LZSS encoding routine using the command line arguments as
*                the file to decode and the decoded target.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Encodes specified file
*   Returned   : status from DecodeLZSS
****************************************************************************/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Invalid arguments\n");
        printf("Correct format:\n");
        printf("%s <compressed file> <decompressed file>\n", argv[0]);
    }

    return DecodeLZSS(argv[1], argv[2]);
}
