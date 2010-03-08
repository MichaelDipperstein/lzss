/***************************************************************************
*                       Bit File Library Usage Sample
*
*   File    : sample.c
*   Purpose : Demonstrates usage of bit file library.
*   Author  : Michael Dipperstein
*   Date    : February 8, 2004
*
****************************************************************************
*   HISTORY
*
*   $Id: sample.c,v 1.1.1.1 2004/02/09 05:31:42 michael Exp $
*   $Log: sample.c,v $
*   Revision 1.1.1.1  2004/02/09 05:31:42  michael
*   Initial release
*
*
****************************************************************************
*
* Sample: A bit file library sample usage program
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
#include <stdio.h>
#include <stdlib.h>
#include "bitfile.h"

/***************************************************************************
*                                 MACROS
***************************************************************************/
#define NUM_CALLS       5

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This function demonstrates the usage of each of the bit
*                bit file functions.
*   Parameters : argc - the number command line arguements (not used)
*   Parameters : argv - array of command line arguements (not used)
*   Effects    : Writes bit file, reads back results, printing them to
*                stdout.
*   Returned   : EXIT_SUCCESS
***************************************************************************/
int main(int argc, char *argv[])
{
    bit_file_t *bfp;
    int i, value;

    /* create bit file for writing */
    bfp = BitFileOpen("testfile", BF_WRITE);

    if (bfp == NULL)
    {
         perror("opening file");
         return (EXIT_FAILURE);
    }

    /* write chars */
    value = (int)'A';
    for (i = 0; i < NUM_CALLS; i++)
    {
        printf("writing char %c\n", value);
        if(BitFilePutChar(value, bfp) == EOF)
        {
            perror("writing char");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }

        value++;
    }
        
    /* write single bits */
    value = 0;
    for (i = 0; i < NUM_CALLS; i++)
    {
        printf("writing bit %d\n", value);
        if(BitFilePutBit(value, bfp) == EOF)
        {
            perror("writing bit");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }

        value = !value;
    }
    
    /* write ints as bits */
    value = 0x11111111;
    for (i = 0; i < NUM_CALLS; i++)
    {
        printf("writing bits %0X\n", value);
        if(BitFilePutBits(bfp, &value, (8 * sizeof(int))) == EOF)
        {
            perror("writing bits");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }

        value += 0x11111111;
    }

    /* close bit file */
    if (BitFileClose(bfp) != 0)
    {
         perror("closing file");
         return (EXIT_FAILURE);
    }

    /* now read back writes */

    /* open bit file */
    bfp = BitFileOpen("testfile", BF_READ);

    if (bfp == NULL)
    {
         perror("opening file");
         return (EXIT_FAILURE);
    }

    /* read chars */
    for (i = 0; i < NUM_CALLS; i++)
    {
        value = BitFileGetChar(bfp);
        if(value == EOF)
        {
            perror("reading char");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read %c\n", value);
        }
    }
        
    /* read single bits */
    for (i = 0; i < NUM_CALLS; i++)
    {
        value = BitFileGetBit(bfp);
        if(value == EOF)
        {
            perror("reading bit");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read bit %d\n", value);
        }
    }
    
    /* read ints as bits */
    for (i = 0; i < NUM_CALLS; i++)
    {
        if(BitFileGetBits(bfp, &value, (8 * sizeof(int))) == EOF)
        {
            perror("reading bits");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read bits %0X\n", value);
        }
    }

    /* close bit file */
    if (BitFileClose(bfp) != 0)
    {
         perror("closing file");
         return (EXIT_FAILURE);
    }

    return(EXIT_SUCCESS);
}
