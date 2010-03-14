/***************************************************************************
*                     Sample Program Using LZSS Library
*
*   File    : comp.c
*   Purpose : Demonstrate usage of LZSS library to compress a file
*   Author  : Michael Dipperstein
*   Date    : November 7, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: comp.c,v 1.1 2004/11/08 06:06:07 michael Exp $
*   $Log: comp.c,v $
*   Revision 1.1  2004/11/08 06:06:07  michael
*   Initial revision of super simple compression and decompression samples.
*
*
****************************************************************************
*
* COMP: Super simple sample demonstraiting use of LZSS compression routine.
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
*                the file to encode and the encoded target.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Encodes specified file
*   Returned   : status from EncodeLZSS
****************************************************************************/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Invalid arguments\n");
        printf("Correct format:\n");
        printf("%s <compressed file> <decompressed file>\n", argv[0]);
        
        return 0;
    }

    return EncodeLZSS(argv[1], argv[2]);
}
