/***************************************************************************
*                     Sample Program Using LZSS Library
*
*   File    : sample.c
*   Purpose : Demonstrate usage of LZSS library
*   Author  : Michael Dipperstein
*   Date    : February 21, 2004
*
****************************************************************************
*
* SAMPLE: Sample usage of LZSS Library
* Copyright (C) 2004, 2006, 2007, 2014 by
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
#include <stdlib.h>
#include <string.h>
#include "lzss.h"
#include "optlist.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
typedef enum
{
    ENCODE,
    DECODE
} modes_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it validates
*                the command line input and, if valid, it will either
*                encode a file using the LZSS algorithm or decode a
*                file encoded with the LZSS algorithm.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Encodes/Decodes input file
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/
int main(int argc, char *argv[])
{
    option_t *optList;
    option_t *thisOpt;
    FILE *fpIn;             /* pointer to open input file */
    FILE *fpOut;            /* pointer to open output file */
    modes_t mode;

    /* initialize data */
    fpIn = NULL;
    fpOut = NULL;
    mode = ENCODE;

    /* parse command line */
    optList = GetOptList(argc, argv, "cdi:o:h?");
    thisOpt = optList;

    while (thisOpt != NULL)
    {
        switch(thisOpt->option)
        {
            case 'c':       /* compression mode */
                mode = ENCODE;
                break;

            case 'd':       /* decompression mode */
                mode = DECODE;
                break;

            case 'i':       /* input file name */
                if (fpIn != NULL)
                {
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    fclose(fpIn);

                    if (fpOut != NULL)
                    {
                        fclose(fpOut);
                    }

                    FreeOptList(optList);
                    return -1;
                }

                /* open input file as binary */
                fpIn = fopen(thisOpt->argument, "rb");
                if (fpIn == NULL)
                {
                    perror("Opening input file");

                    if (fpOut != NULL)
                    {
                        fclose(fpOut);
                    }

                    FreeOptList(optList);
                    return -1;
                }
                break;

            case 'o':       /* output file name */
                if (fpOut != NULL)
                {
                    fprintf(stderr, "Multiple output files not allowed.\n");
                    fclose(fpOut);

                    if (fpIn != NULL)
                    {
                        fclose(fpIn);
                    }

                    FreeOptList(optList);
                    return -1;
                }

                /* open output file as binary */
                fpOut = fopen(thisOpt->argument, "wb");
                if (fpOut == NULL)
                {
                    perror("Opening output file");

                    if (fpIn != NULL)
                    {
                        fclose(fpIn);
                    }

                    FreeOptList(optList);
                    return -1;
                }
                break;

            case 'h':
            case '?':
                printf("Usage: %s <options>\n\n", FindFileName(argv[0]));
                printf("options:\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -i <filename> : Name of input file.\n");
                printf("  -o <filename> : Name of output file.\n");
                printf("  -h | ?  : Print out command line options.\n\n");
                printf("Default: %s -c -i stdin -o stdout\n",
                    FindFileName(argv[0]));

                FreeOptList(optList);
                return 0;
        }

        optList = thisOpt->next;
        free(thisOpt);
        thisOpt = optList;
    }

    /* use stdin/out if no files are provided */
    if (fpIn == NULL)
    {
        fpIn = stdin;
    }

    if (fpOut == NULL)
    {
        fpOut = stdout;
    }

    /* we have valid parameters encode or decode */
    if (mode == ENCODE)
    {
        EncodeLZSS(fpIn, fpOut);
    }
    else
    {
        DecodeLZSS(fpIn, fpOut);
    }

    /* remember to close files */
    fclose(fpIn);
    fclose(fpOut);
    return 0;
}
