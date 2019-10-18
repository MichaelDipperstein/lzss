/**
 * \brief Bit file stream library header
 * \file bitfile.h
 * \author Michael Dipperstein (mdipperstein@gmail.com)
 * \date January 9, 2004
 *
 * This file provides definitions and prototypes for a simple library of
 * I/O functions for files that contain data in sizes that aren't integral
 * bytes.  An attempt was made to make the functions in this library
 * analogous to functions provided to manipulate byte streams.  The functions
 * contained in this library were created with compression algorithms in mind,
 * but may be suited to other applications.
 *
 * \copyright Copyright (C) 2004 - 2019 by Michael Dipperstein
 * (mdipperstein@gmail.com)
 *
 * \par
 * This file is part of the bit file library.
 *
 * \license
 * The bitfile library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * \par
 * The bitfile library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _BITFILE_H_
#define _BITFILE_H_

/**
 * \mainpage Bitfile - Bit stream library
 *
 * These pages provide documentation for Bitfile, a bit stream handling library.
 *
 * \copyright Copyright (C) 2004 - 2019 by Michael Dipperstein
 * (mdipperstein@gmail.com)
 *
 * \license
 * The bitfile library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * \par
 * The bitfile library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/**
 * \enum BF_MODES
 * \brief This is an enumeration of the bit file modes (read, write, and
 * append)
 */
typedef enum
{
    BF_READ = 0,    /*!< indicate that the file is for reading */
    BF_WRITE = 1,   /*!< indicate that the file is for writing */
    BF_APPEND= 2,   /*!< indicate that writes will be appended to the file */
    BF_NO_MODE      /*!< end of enum */
} BF_MODES;

struct bit_file_t;

/**
 * \typedef bit_file_t
 * \brief This is shorthand for struct bit_file_t, the of the structure that
 * is used to access all bitfiles.  It is analogous to FILE.
 */
typedef struct bit_file_t bit_file_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* open/close file */
bit_file_t *BitFileOpen(const char *fileName, const BF_MODES mode);
bit_file_t *MakeBitFile(FILE *stream, const BF_MODES mode);
int BitFileClose(bit_file_t *stream);
FILE *BitFileToFILE(bit_file_t *stream);

/* toss spare bits and byte align file */
int BitFileByteAlign(bit_file_t *stream);

/* fill byte with ones or zeros and write out results */
int BitFileFlushOutput(bit_file_t *stream, const unsigned char onesFill);

/* get/put character */
int BitFileGetChar(bit_file_t *stream);
int BitFilePutChar(const int c, bit_file_t *stream);

/* get/put single bit */
int BitFileGetBit(bit_file_t *stream);
int BitFilePutBit(const int c, bit_file_t *stream);

/* get/put number of bits (most significant bit to least significat bit) */
int BitFileGetBits(bit_file_t *stream, void *bits, const unsigned int count);
int BitFilePutBits(bit_file_t *stream, void *bits, const unsigned int count);

/***************************************************************************
* get/put a number of bits from numerical types (short, int, long, ...)
*
* For these functions, the first bit in/out is the least significant bit of
* the least significant byte, so machine endiness is accounted for.  Only
* big endian and little endian architectures are currently supported.
*
* NOTE: size is the sizeof() for the data structure pointed to by bits.
***************************************************************************/
int BitFileGetBitsNum(bit_file_t *stream, void *bits, const unsigned int count,
    const size_t size);
int BitFilePutBitsNum(bit_file_t *stream, void *bits, const unsigned int count,
    const size_t size);

#endif /* _BITFILE_H_ */
