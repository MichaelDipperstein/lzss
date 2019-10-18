/**
 * \brief Sample program demonstrating the usage of the bit file library.
 * \file sample.c
 * \author Michael Dipperstein (mdipperstein@gmail.com)
 * \date February 8, 2004
 *
 * This file demonstrates a simple library of I/O functions for files that
 * contain data in sizes that aren't integral bytes.
 *
 * \copyright Copyright (C) 2004-2019 by Michael Dipperstein
 * (mdipperstein@gmail.com)
 *
 * \par
 * This file is part of the bit file library.
 *
 * \license
 * The bit file library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * \par
 * The bit file library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * \defgroup examples Usage Examples
 * \brief This module contains code demonstrating the usage of the bit file
 * library
 * @{
 */

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "bitfile.h"

/***************************************************************************
*                                 MACROS
***************************************************************************/
/** The number of times to loop through function calls */
#define NUM_CALLS       5

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/**
 * \fn int main(int argc, char *argv[])
 *
 * \brief This function demonstrates the usage of each of the bit file
 * functions.
 *
 * \param argc The number command line arguments (not used)
 *
 * \param argv An array of command line arguments (not used)
 *
 * \effects
 * Writes bit file, reads back results, printing them to stdout.
 *
 * \returns \c EXIT_SUCCESS on success, otherwise \c EXIT_FAILURE.
 */
int main(int argc, char *argv[])
{
    bit_file_t *bfp;
    FILE *fp;
    int i, numCalls, value;

    if (argc < 2)
    {
        numCalls = NUM_CALLS;
    }
    else
    {
        numCalls = atoi(argv[1]);
    }

    /* create bit file for writing */
    bfp = BitFileOpen("testfile", BF_WRITE);

    if (bfp == NULL)
    {
         perror("opening file");
         return (EXIT_FAILURE);
    }

    /* write chars */
    value = (int)'A';
    for (i = 0; i < numCalls; i++)
    {
        printf("writing char %c\n", value);
        if(BitFilePutChar(value, bfp) == EOF)
        {
            perror("writing char");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }

        value++;
    }

    /* write single bits */
    value = 0;
    for (i = 0; i < numCalls; i++)
    {
        printf("writing bit %d\n", value);
        if(BitFilePutBit(value, bfp) == EOF)
        {
            perror("writing bit");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }

        value = 1 - value;
    }

    /* write ints as bits */
    value = 0x11111111;
    for (i = 0; i < numCalls; i++)
    {
        printf("writing bits %0X\n", (unsigned int)value);
        if(BitFilePutBits(bfp, &value,
            (unsigned int)(8 * sizeof(int))) == EOF)
        {
            perror("writing bits");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
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
    for (i = 0; i < numCalls; i++)
    {
        printf("appending char %c\n", value);
        if(BitFilePutChar(value, bfp) == EOF)
        {
            perror("appending char");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }

        value++;
    }

    /* write some bits from an integer */
    value = 0x111;
    for (i = 0; i < numCalls; i++)
    {
        printf("writing 12 bits from an integer %03X\n", (unsigned int)value);
        if(BitFilePutBitsNum(bfp, &value, 12, sizeof(value)) == EOF)
        {
            perror("writing bits from an integer");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
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
    for (i = 0; i < numCalls; i++)
    {
        printf("appending char %c\n", value);
        if(fputc(value, fp) == EOF)
        {
            perror("appending char to FILE");
            if (fclose(fp) == EOF)
            {
                 perror("closing stdio FILE");
            }
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
    for (i = 0; i < numCalls; i++)
    {
        value = BitFileGetChar(bfp);
        if(value == EOF)
        {
            perror("reading char");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read %c\n", value);
        }
    }

    /* read single bits */
    for (i = 0; i < numCalls; i++)
    {
        value = BitFileGetBit(bfp);
        if(value == EOF)
        {
            perror("reading bit");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read bit %d\n", value);
        }
    }

    /* read ints as bits */
    for (i = 0; i < numCalls; i++)
    {
        if(BitFileGetBits(bfp, &value, (unsigned int)(8 * sizeof(int))) == EOF)
        {
            perror("reading bits");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read bits %0X\n", (unsigned int)value);
        }
    }

    if (BitFileByteAlign(bfp) == EOF)
    {
        fprintf(stderr, "failed to align file\n");
        if (0 != BitFileClose(bfp))
        {
            perror("closing bitfile");
        }
        return (EXIT_FAILURE);
    }
    else
    {
        printf("byte aligning file\n");
    }

    /* read appended characters */
    for (i = 0; i < numCalls; i++)
    {
        value = BitFileGetChar(bfp);
        if(value == EOF)
        {
            perror("reading char");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read %c\n", value);
        }
    }

    /* read some bits into an integer */
    for (i = 0; i < numCalls; i++)
    {
        value = 0;
        if(BitFileGetBitsNum(bfp, &value, 12, sizeof(value)) == EOF)
        {
            perror("reading bits from an integer");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
            return (EXIT_FAILURE);
        }
        else
        {
            printf("read 12 bits into an integer %03X\n", (unsigned int)value);
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
    for (i = 0; i < numCalls; i++)
    {
        value = fgetc(fp);
        if(value == EOF)
        {
            perror("stdio reading char");
            if (0 != BitFileClose(bfp))
            {
                perror("closing bitfile");
            }
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

    return (EXIT_SUCCESS);
}

/**@}*/
