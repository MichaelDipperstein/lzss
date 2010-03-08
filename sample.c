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
*   $Id: sample.c,v 1.4 2005/12/08 06:56:55 michael Exp $
*   $Log: sample.c,v $
*   Revision 1.4  2005/12/08 06:56:55  michael
*   Minor text corrections.
*
*   Revision 1.3  2005/12/06 15:06:37  michael
*   Added BitFileGetBitsInt and BitFilePutBitsInt for integer types.
*
*   Revision 1.2  2004/11/09 14:19:22  michael
*   Added examples of new functions BitFileToFILE and BitFileByteAlign
*
*   Revision 1.1.1.1  2004/02/09 05:31:42  michael
*   Initial release
*
*
****************************************************************************
*
* Sample: A bit file library sample usage program
* Copyright (C) 2004-2005 by Michael Dipperstein (mdipper@cs.ucsb.edu)
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
    FILE *fp;
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
    else
    {
        printf("closed file\n");
    }

    /* reopen file for appending */
    bfp = BitFileOpen("testfile", BF_APPEND);

    if (bfp == NULL)
    {
         perror("opening file");
         return (EXIT_FAILURE);
    }

    /* append some chars */
    value = (int)'A';
    for (i = 0; i < NUM_CALLS; i++)
    {
        printf("appending char %c\n", value);
        if(BitFilePutChar(value, bfp) == EOF)
        {
            perror("appending char");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }

        value++;
    }

    /* write some bits from an integer */
    value = 0x111;
    for (i = 0; i < NUM_CALLS; i++)
    {
        printf("writing 12 bits from an integer %03X\n", value);
        if(BitFilePutBitsInt(bfp, &value, 12, sizeof(value)) == EOF)
        {
            perror("writing bits from an integer");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }

        value += 0x111;
    }

    /* convert to normal file */
    fp = BitFileToFILE(bfp);

    if (fp == NULL)
    {
         perror("converting to stdio FILE");
         return (EXIT_FAILURE);
    }
    else
    {
        printf("converted to stdio FILE\n");
    }

    /* append some chars */
    value = (int)'a';
    for (i = 0; i < NUM_CALLS; i++)
    {
        printf("appending char %c\n", value);
        if(fputc(value, fp) == EOF)
        {
            perror("appening char to FILE");
            fclose(fp);
            return (EXIT_FAILURE);
        }

        value++;
    }

    /* close file */
    if (fclose(fp) == EOF)
    {
         perror("closing stdio FILE");
         return (EXIT_FAILURE);
    }

    /* now read back writes */

    /* open bit file */
    bfp = BitFileOpen("testfile", BF_READ);

    if (bfp == NULL)
    {
         perror("reopening file");
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

    if (BitFileByteAlign(bfp) == EOF)
    {
        fprintf(stderr, "failed to align file\n");
        BitFileClose(bfp);
        return (EXIT_FAILURE);
    }
    else
    {
        printf("byte aligning file\n");
    }

    /* read appended characters */
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

    /* read some bits into an integer */
    for (i = 0; i < NUM_CALLS; i++)
    {
        value = 0;
        if(BitFileGetBitsInt(bfp, &value, 12, sizeof(value)) == EOF)
        {
            perror("reading bits from an integer");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read 12 bits into an integer %03X\n", value);
        }
    }

    /* convert to stdio FILE */
    fp = BitFileToFILE(bfp);

    if (fp == NULL)
    {
         perror("converting to stdio FILE");
         return (EXIT_FAILURE);
    }
    else
    {
        printf("converted to stdio FILE\n");
    }

    /* read append some chars */
    value = (int)'a';
    for (i = 0; i < NUM_CALLS; i++)
    {
        value = fgetc(fp);
        if(value == EOF)
        {
            perror("stdio reading char");
            BitFileClose(bfp);
            return (EXIT_FAILURE);
        }
        else
        {
            printf("stdio read %c\n", value);
        }
    }

    /* close file */
    if (fclose(fp) == EOF)
    {
         perror("closing stdio FILE");
         return (EXIT_FAILURE);
    }

    return(EXIT_SUCCESS);
}
