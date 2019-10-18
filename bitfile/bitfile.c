/**
 * \brief Bit file stream library implementation
 * \file bitfile.c
 * \author Michael Dipperstein (mdipperstein@gmail.com)
 * \date January 9, 2004
 *
 * This file implements a simple library of I/O functions for files that
 * contain data in sizes that aren't integral bytes.  An attempt was made to
 * make the functions in this library analogous to functions provided to
 * manipulate byte streams.  The functions contained in this library were
 * created with compression algorithms in mind, but may be suited to other
 * applications.
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

 /**
 * \defgroup library Library Code
 * \brief This module contains the code for the ezini INI file handling
 * library
 * @{
 */

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include "bitfile.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/**
 * \typedef num_func_t
 * \brief This type to points to the kind of functions that put/get bits
 * from/to numerical data types (short, int, long, ...)
 *
 * The parameters are a bit file pointer, a pointer to a data structure,
 * the number of bits, and the sizeof the data structure.
 */
typedef int (*num_func_t)(bit_file_t*, void*, const unsigned int, const size_t);

/**
 * \struct bit_file_t
 * \brief This is an complete definition for the type containing data needed
 * to correctly manipulate a bit file.
 */
struct bit_file_t
{
    FILE *fp;                   /*!< file pointer used by stdio functions */
    unsigned char bitBuffer;    /*!< bits waiting to be read/written */
    unsigned char bitCount;     /*!< number of bits in bitBuffer */
    num_func_t PutBitsNumFunc;  /*!< endian specific BitFilePutBitsNum */
    num_func_t GetBitsNumFunc;  /*!< endian specific BitFileGetBitsNum */
    BF_MODES mode;              /*!< open for read, write, or append */
};

/**
 * \enum endian_t
 * \brief This is an enumeration of the types of endianess handled by this
 * library (big endian and little endian).
 */
typedef enum
{
    BF_UNKNOWN_ENDIAN,      /*!< unrecognized or untested endianess */
    BF_LITTLE_ENDIAN,       /*!< little endian */
    BF_BIG_ENDIAN           /*!< big endian */
} endian_t;

/**
 * \struct endian_test_t
 * \brief This is a union used when testing for endianess.
 */
typedef union
{
    unsigned long word;                             /*!< an unsigned long */
    unsigned char bytes[sizeof(unsigned long)];     /*!< bytes making up the
                                                        unsigned long */
} endian_test_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
static endian_t DetermineEndianess(void);

static int BitFilePutBitsLE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size);
static int BitFilePutBitsBE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size);

static int BitFileGetBitsLE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size);
static int BitFileGetBitsBE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size);
static int BitFileNotSupported(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/**
 * \fn bit_file_t *BitFileOpen(const char *fileName, const BF_MODES mode)
 *
 * \brief This function opens a bit file for reading, writing, or appending.
 *
 * \param fileName A pointer to a \c NULL terminated string containing the
 * name of the file to be opened.
 *
 * \param mode The mode of the file to be opened (BF_READ, BF_WRITE, or
 * BF_APPEND).
 *
 * \effects
 * The specified file will be opened and file structure will be
 * allocated.
 *
 * \returns A pointer to the bit_file_t structure for the bit file opened,
 * or \c NULL on failure.  \c errno will be set for all failure cases.
 *
 * This function opens a bit file for reading, writing, or appending.  If
 * successful, a bit_file_t data structure will be allocated and a pointer
 * to the structure will be returned.
 */
 bit_file_t *BitFileOpen(const char *fileName, const BF_MODES mode)
{
    const char modes[3][3] = {"rb", "wb", "ab"};    /* binary modes for fopen */
    bit_file_t *bf;

    bf = (bit_file_t *)malloc(sizeof(bit_file_t));

    if (bf == NULL)
    {
        /* malloc failed */
        errno = ENOMEM;
    }
    else
    {
        bf->fp = fopen(fileName, modes[mode]);

        if (bf->fp == NULL)
        {
            /* fopen failed */
            free(bf);
            bf = NULL;
        }
        else
        {
            /* fopen succeeded fill in remaining bf data */
            bf->bitBuffer = 0;
            bf->bitCount = 0;
            bf->mode = mode;

            switch (DetermineEndianess())
            {
                case BF_LITTLE_ENDIAN:
                    bf->PutBitsNumFunc = &BitFilePutBitsLE;
                    bf->GetBitsNumFunc = &BitFileGetBitsLE;
                    break;

                case BF_BIG_ENDIAN:
                    bf->PutBitsNumFunc = &BitFilePutBitsBE;
                    bf->GetBitsNumFunc = &BitFileGetBitsBE;
                    break;

                case BF_UNKNOWN_ENDIAN:
                default:
                    bf->PutBitsNumFunc = BitFileNotSupported;
                    bf->GetBitsNumFunc = BitFileNotSupported;
                    break;
            }

            /***************************************************************
            * TO DO: Consider using the last byte in a file to indicate
            * the number of bits in the previous byte that actually have
            * data.  If I do that, I'll need special handling of files
            * opened with a mode of BF_APPEND.
            ***************************************************************/
        }
    }

    return (bf);
}

/**
 * \fn bit_file_t *MakeBitFile(FILE *stream, const BF_MODES mode)
 *
 * \brief This function naively wraps a standard file in a bit_file_t
 * structure.
 *
 * \param stream A pointer to the standard file being wrapped.
 *
 * \param mode The mode of the file being wrapped (BF_READ, BF_WRITE, or
 * BF_APPEND).
 *
 * \effects
 * A bit_file_t structure will be created for the stream passed as
 * a parameter.
 *
 * \returns Pointer to the bit_file_t structure for the bit file or \c NULL
 * on failure.  \c errno will be set for all failure cases.
 *
 * This function naively wraps a standard file in a bit_file_t structure.
 * ANSI-C doesn't support file status functions commonly found in other C
 * variants, so the caller must be passed as a parameter.
 */
bit_file_t *MakeBitFile(FILE *stream, const BF_MODES mode)
{
    bit_file_t *bf;

    if (stream == NULL)
    {
        /* can't wrapper empty steam */
        errno = EBADF;
        bf = NULL;
    }
    else
    {
        bf = (bit_file_t *)malloc(sizeof(bit_file_t));

        if (bf == NULL)
        {
            /* malloc failed */
            errno = ENOMEM;
        }
        else
        {
            /* set structure data */
            bf->fp = stream;
            bf->bitBuffer = 0;
            bf->bitCount = 0;
            bf->mode = mode;

            switch (DetermineEndianess())
            {
                case BF_LITTLE_ENDIAN:
                    bf->PutBitsNumFunc = &BitFilePutBitsLE;
                    bf->GetBitsNumFunc = &BitFileGetBitsLE;
                    break;

                case BF_BIG_ENDIAN:
                    bf->PutBitsNumFunc = &BitFilePutBitsBE;
                    bf->GetBitsNumFunc = &BitFileGetBitsBE;
                    break;

                case BF_UNKNOWN_ENDIAN:
                default:
                    bf->PutBitsNumFunc = BitFileNotSupported;
                    bf->GetBitsNumFunc = BitFileNotSupported;
                    break;
            }
        }
    }

    return (bf);
}

/**
 * \fn endian_t DetermineEndianess(void)
 *
 * \brief This function determines the endianess of the current hardware
 * architecture.
 *
 * \param void
 *
 * \effects
 * None
 *
 * \returns endian_t for the current machine architecture.
 *
 * This function determines the endianess of the current hardware
 * architecture.  An unsigned long is set to 1.  If the first byte of the
 * unsigned long gets the 1, this is a little endian machine.  If the last
 * byte gets the 1, this is a big endian machine.
 */
 static endian_t DetermineEndianess(void)
{
    endian_t endian;
    endian_test_t endianTest;

    endianTest.word = 1;

    if (endianTest.bytes[0] == 1)
    {
        /* LSB is 1st byte (little endian)*/
        endian = BF_LITTLE_ENDIAN;
    }
    else if (endianTest.bytes[sizeof(unsigned long) - 1] == 1)
    {
        /* LSB is last byte (big endian)*/
        endian = BF_BIG_ENDIAN;
    }
    else
    {
        endian = BF_UNKNOWN_ENDIAN;
    }

    return endian;
}

/**
 * \fn int BitFileClose(bit_file_t *stream)
 *
 * \brief This function closes a bit file and frees all associated data.
 *
 * \param stream A pointer to the bit file stream being closed.
 *
 * \effects
 * The specified file will be closed and the file structure will
 * be freed.
 *
 * \returns 0 for success or \c EOF for failure.
 */
int BitFileClose(bit_file_t *stream)
{
    int returnValue = 0;

    if (stream == NULL)
    {
        return(EOF);
    }

    if ((stream->mode == BF_WRITE) || (stream->mode == BF_APPEND))
    {
        /* write out any unwritten bits */
        if (stream->bitCount != 0)
        {
            (stream->bitBuffer) <<= 8 - (stream->bitCount);
            fputc(stream->bitBuffer, stream->fp);   /* handle error? */
        }
    }

    /***********************************************************************
    *  TO DO: Consider writing an additional byte indicating the number of
    *  valid bits (bitCount) in the previous byte.
    ***********************************************************************/

    /* close file */
    returnValue = fclose(stream->fp);

    /* free memory allocated for bit file */
    free(stream);

    return(returnValue);
}

/**
 * \fn FILE *BitFileToFILE(bit_file_t *stream)
 *
 * \brief This function flushes and frees the bitfile structure, returning
 * a pointer to a stdio file corresponding to the bitfile.
 *
 * \param stream A pointer to the bit file stream being converted
 *
 * \effects
 * None
 *
 * \returns A FILE pointer to stream.  \c NULL for failure.
 */
FILE *BitFileToFILE(bit_file_t *stream)
{
    FILE *fp = NULL;

    if (stream == NULL)
    {
        return(NULL);
    }

    if ((stream->mode == BF_WRITE) || (stream->mode == BF_APPEND))
    {
        /* write out any unwritten bits */
        if (stream->bitCount != 0)
        {
            (stream->bitBuffer) <<= 8 - (stream->bitCount);
            fputc(stream->bitBuffer, stream->fp);   /* handle error? */
        }
    }

    /***********************************************************************
    *  TO DO: Consider writing an additional byte indicating the number of
    *  valid bits (bitCount) in the previous byte.
    ***********************************************************************/

    /* close file */
    fp = stream->fp;

    /* free memory allocated for bit file */
    free(stream);

    return(fp);
}

/**
 * \fn int BitFileByteAlign(bit_file_t *stream)
 *
 * \brief This function aligns the bitfile to the nearest byte.
 *
 * \param stream A pointer to the bit file stream to align
 *
 * \effects
 * The bit buffer is flushed out.
 *
 * \returns \c EOF if stream is \c NULL or a write fails.  Writes return
 * the byte aligned contents of the bit buffer.  Reads returns the unaligned
 * contents of the bit buffer.
 *
 * This function aligns the bitfile to the nearest byte.  For output files,
 * this means writing out the bit buffer with extra bits set to 0.  For
 * input files, this means flushing the bit buffer.
 */
int BitFileByteAlign(bit_file_t *stream)
{
    int returnValue;

    if (stream == NULL)
    {
        return(EOF);
    }

    returnValue = stream->bitBuffer;

    if ((stream->mode == BF_WRITE) || (stream->mode == BF_APPEND))
    {
        /* write out any unwritten bits */
        if (stream->bitCount != 0)
        {
            (stream->bitBuffer) <<= 8 - (stream->bitCount);
            fputc(stream->bitBuffer, stream->fp);   /* handle error? */
        }
    }

    stream->bitBuffer = 0;
    stream->bitCount = 0;

    return (returnValue);
}

/**
 * \fn int BitFileFlushOutput(bit_file_t *stream,
 *  const unsigned char onesFill)
 *
 * \brief This function flushes an output bit buffer.
 *
 * \param stream A pointer to the bit file stream to flush
 *
 * \param onesFill set to non-zero if spare bits are to be filled with ones
 *
 * \effects
 * The bit buffer is flushed out.  Spare bits are filled with
 * either zeros or ones based on onesFill.
 *
 * \returns \c EOF if stream is \c NULL or not writable.  Otherwise, the
 * bit buffer value written.  -1 is returned if no data is written.
 *
 * This function flushes an output bit buffer.  This means left justifying
 * any pending bits, and filling spare bits with the fill value.
 */
int BitFileFlushOutput(bit_file_t *stream, const unsigned char onesFill)
{
    int returnValue;

    if (stream == NULL)
    {
        return(EOF);
    }

    returnValue = -1;

    /* write out any unwritten bits */
    if (stream->bitCount != 0)
    {
        stream->bitBuffer <<= (8 - stream->bitCount);

        if (onesFill)
        {
            stream->bitBuffer |= (0xFF >> stream->bitCount);
        }

        returnValue = fputc(stream->bitBuffer, stream->fp);
    }

    stream->bitBuffer = 0;
    stream->bitCount = 0;

    return (returnValue);
}

/**
 * \fn int BitFileGetChar(bit_file_t *stream)
 *
 * \brief This function returns the next byte from the file passed as a
 * parameter.
 *
 * \param stream A pointer to the bit file stream to read from
 *
 * \effects
 * Reads next byte from file and updates the pointer file and bit
 * buffer accordingly.
 *
 * \returns \c EOF if a whole byte cannot be obtained.  Otherwise, the
 * character read.
 */
int BitFileGetChar(bit_file_t *stream)
{
    int returnValue;
    unsigned char tmp;

    if (stream == NULL)
    {
        return(EOF);
    }

    returnValue = fgetc(stream->fp);

    if (stream->bitCount == 0)
    {
        /* we can just get byte from file */
        return returnValue;
    }

    /* we have some buffered bits to return too */
    if (returnValue != EOF)
    {
        /* figure out what to return */
        tmp = ((unsigned char)returnValue) >> (stream->bitCount);
        tmp |= ((stream->bitBuffer) << (8 - (stream->bitCount)));

        /* put remaining in buffer. count shouldn't change. */
        stream->bitBuffer = returnValue;

        returnValue = tmp;
    }

    return returnValue;
}

/**
 * \fn int BitFilePutChar(const int c, bit_file_t *stream)
 *
 * \brief This function writes the byte passed as a parameter to the file
 * passed a parameter.
 *
 * \param c The character to write
 *
 * \param stream A pointer to the bit file stream to write to
 *
 * \effects
 * Writes c to the file and updates the pointer file and bit
 * buffer accordingly.
 *
 * \returns The character written is returned on success.  \c EOF is returned
 * on failure.
 */
int BitFilePutChar(const int c, bit_file_t *stream)
{
    unsigned char tmp;

    if (stream == NULL)
    {
        return(EOF);
    }

    if (stream->bitCount == 0)
    {
        /* we can just put byte from file */
        return fputc(c, stream->fp);
    }

    /* figure out what to write */
    tmp = ((unsigned char)c) >> (stream->bitCount);
    tmp = tmp | ((stream->bitBuffer) << (8 - stream->bitCount));

    if (fputc(tmp, stream->fp) != EOF)
    {
        /* put remaining in buffer. count shouldn't change. */
        stream->bitBuffer = c;
    }
    else
    {
        return EOF;
    }

    return tmp;
}

/**
 * \fn int BitFileGetBit(bit_file_t *stream)
 *
 * \brief This function returns the next bit from the file passed as a
 * parameter.
 *
 * \param stream A pointer to the bit file stream to read from
 *
 * \effects
 * Reads next bit from bit buffer.  If the buffer is empty,
 * a new byte will be read from the file.
 *
 * \returns 0 if bit == 0, 1 if bit == 1, and \c EOF if operation fails.
 *
 * This function returns the next bit from the file passed as a parameter.
 * The bit value returned is the msb in the bit buffer.
 */
int BitFileGetBit(bit_file_t *stream)
{
    int returnValue;

    if (stream == NULL)
    {
        return(EOF);
    }

    if (stream->bitCount == 0)
    {
        /* buffer is empty, read another character */
        if ((returnValue = fgetc(stream->fp)) == EOF)
        {
            return EOF;
        }
        else
        {
            stream->bitCount = 8;
            stream->bitBuffer = returnValue;
        }
    }

    /* bit to return is msb in buffer */
    stream->bitCount--;
    returnValue = (stream->bitBuffer) >> (stream->bitCount);

    return (returnValue & 0x01);
}

/**
 * \fn int BitFilePutBit(const int c, bit_file_t *stream)
 *
 * \brief This function writes the bit passed as a parameter to the file
 * passed a parameter.
 *
 * \param c The bit value to write
 *
 * \param stream A pointer to the bit file stream to write to
 *
 * \effects
 * Writes a bit to the bit buffer.  If the buffer has a byte, the
 * buffer is written to the file and cleared.
 *
 * \returns The bit value written is returned on success.  \c EOF is
 * returned on failure.
 */
int BitFilePutBit(const int c, bit_file_t *stream)
{
    int returnValue = c;

    if (stream == NULL)
    {
        return(EOF);
    }

    stream->bitCount++;
    stream->bitBuffer <<= 1;

    if (c != 0)
    {
        stream->bitBuffer |= 1;
    }

    /* write bit buffer if we have 8 bits */
    if (stream->bitCount == 8)
    {
        if (fputc(stream->bitBuffer, stream->fp) == EOF)
        {
            returnValue = EOF;
        }

        /* reset buffer */
        stream->bitCount = 0;
        stream->bitBuffer = 0;
    }

    return returnValue;
}

/**
 * \fn int BitFileGetBits(bit_file_t *stream, void *bits,
 * const unsigned int count)
 *
 * \brief This function reads the specified number of bits from the file
 * passed as a parameter.
 *
 * \param stream A pointer to the bit file stream to read from
 *
 * \param bits The address to store bits read
 *
 * \param count The number of bits to read
 *
 * \effects
 * Reads bits from the bit buffer and file stream.  The bit buffer
 * will be modified as necessary.
 *
 * \returns \c EOF for failure, otherwise the number of bits read.  If
 * an \c EOF is reached before all the bits are read, bits will contain every
 * bit through the last complete byte.
 *
 * This function reads the specified number of bits from the file passed as
 * a parameter and writes them to the specified memory location (ms bit to
 * ls bit).
 */
int BitFileGetBits(bit_file_t *stream, void *bits, const unsigned int count)
{
    unsigned char *bytes, shifts;
    int offset, remaining, returnValue;

    bytes = (unsigned char *)bits;

    if ((stream == NULL) || (bits == NULL))
    {
        return(EOF);
    }

    offset = 0;
    remaining = count;

    /* read whole bytes */
    while (remaining >= 8)
    {
        returnValue = BitFileGetChar(stream);

        if (returnValue == EOF)
        {
            return EOF;
        }

        bytes[offset] = (unsigned char)returnValue;
        remaining -= 8;
        offset++;
    }

    if (remaining != 0)
    {
        /* read remaining bits */
        shifts = 8 - remaining;
        bytes[offset] = 0;

        while (remaining > 0)
        {
            returnValue = BitFileGetBit(stream);

            if (returnValue == EOF)
            {
                return EOF;
            }

            bytes[offset] <<= 1;
            bytes[offset] |= (returnValue & 0x01);
            remaining--;
        }

        /* shift last bits into position */
        bytes[offset] <<= shifts;
    }

    return count;
}

/**
 * \fn BitFilePutBits(bit_file_t *stream, void *bits,
 * const unsigned int count)
 *
 * \brief This function writes the specified number of bits from the memory
 * location passed as a parameter to the file passed as a parameter.
 *
 * \param stream A pointer to the bit file stream to write to
 *
 * \param bits The address to store bits write
 *
 * \param count The number of bits to write
 *
 * \effects
 * Writes bits to the bit buffer and file stream.  The bit buffer
 * will be modified as necessary.
 *
 * \returns \c EOF for failure, otherwise the number of bits read.  If
 * an \c EOF is reached before all the bits are read, bits will contain every
 * bit through the last complete byte.
 *
 * This function writes the specified number of bits from the memory location
 * passed as a parameter to the file passed as a parameter.   Bits are written
 * ms bit to ls bit.
 */
int BitFilePutBits(bit_file_t *stream, void *bits, const unsigned int count)
{
    unsigned char *bytes, tmp;
    int offset, remaining, returnValue;

    bytes = (unsigned char *)bits;

    if ((stream == NULL) || (bits == NULL))
    {
        return(EOF);
    }

    offset = 0;
    remaining = count;

    /* write whole bytes */
    while (remaining >= 8)
    {
        returnValue = BitFilePutChar(bytes[offset], stream);

        if (returnValue == EOF)
        {
            return EOF;
        }

        remaining -= 8;
        offset++;
    }

    if (remaining != 0)
    {
        /* write remaining bits */
        tmp = bytes[offset];

        while (remaining > 0)
        {
            returnValue = BitFilePutBit((tmp & 0x80), stream);

            if (returnValue == EOF)
            {
                return EOF;
            }

            tmp <<= 1;
            remaining--;
        }
    }

    return count;
}

/**
 * \fn int BitFileGetBitsNum(bit_file_t *stream, void *bits,
 * const unsigned int count, const size_t size)
 *
 * \brief This function provides a machine independent layer that allows a
 * single function call to stuff an arbitrary number of bits into an integer
 * type variable.
 *
 * \param stream A pointer to the bit file stream to read from
 *
 * \param bits The address to store bits read
 *
 * \param count The number of bits to read
 *
 * \param size sizeof type containing \c bits
 *
 * \effects
 * Calls a function that reads bits from the bit buffer and file
 * stream.  The bit buffer will be modified as necessary.  The bits will be
 * written to \c bits from least significant byte to most significant byte.
 *
 * \returns \c EOF for failure, \c -ENOTSUP for unsupported architecture,
 * otherwise the number of bits read by the called function.
 */
 int BitFileGetBitsNum(bit_file_t *stream, void *bits, const unsigned int count,
    const size_t size)
{
    if ((stream == NULL) || (bits == NULL))
    {
        return EOF;
    }

    if (NULL == stream->GetBitsNumFunc)
    {
        return -ENOTSUP;
    }

    /* call function that correctly handles endianess */
    return (stream->GetBitsNumFunc)(stream, bits, count, size);
}

/**
 * \fn static int BitFileGetBitsLE(bit_file_t *stream, void *bits,
 *  const unsigned int count, const size_t size)
 *
 * \brief This function reads the specified number of bits from the bit
 * file passed as a parameter and writes them to the requested memory
 * location (LSB to MSB).  The memory location is treated as a Little
 * Endian numeric data type.
 *
 * \param stream A pointer to the bit file stream to read from
 *
 * \param bits The address to store bits read
 *
 * \param count The number of bits to read
 *
 * \param size sizeof type containing \c bits
 *
 * \effects
 * Reads bits from the bit buffer and file stream.  The bit buffer
 * will be modified as necessary.  bits is treated as a little endian
 * integer of length >= (count/8) + 1.
 *
 * \returns \c EOF for failure, otherwise the number of bits read.  If an
 * \c EOF is reached before all the bits are read, bits will contain every
 * bit through the last successful read.
 */
static int BitFileGetBitsLE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size)
{
    unsigned char *bytes;
    int offset, remaining, returnValue;

    (void)size;
    bytes = (unsigned char *)bits;
    offset = 0;
    remaining = count;

    /* read whole bytes */
    while (remaining >= 8)
    {
        returnValue = BitFileGetChar(stream);

        if (returnValue == EOF)
        {
            return EOF;
        }

        bytes[offset] = (unsigned char)returnValue;
        remaining -= 8;
        offset++;
    }

    if (remaining != 0)
    {
        /* read remaining bits */
        while (remaining > 0)
        {
            returnValue = BitFileGetBit(stream);

            if (returnValue == EOF)
            {
                return EOF;
            }

            bytes[offset] <<= 1;
            bytes[offset] |= (returnValue & 0x01);
            remaining--;
        }

    }

    return count;
}

/**
 * \fn static int BitFileGetBitsBE(bit_file_t *stream, void *bits,
 *  const unsigned int count, const size_t size)
 *
 * \brief This function reads the specified number of bits from the bit
 * file passed as a parameter and writes them to the requested memory
 * location (LSB to MSB).  The memory location is treated as a Big
 * Endian numeric data type.
 *
 * \param stream A pointer to the bit file stream to read from
 *
 * \param bits The address to store bits read
 *
 * \param count The number of bits to read
 *
 * \param size sizeof type containing \c bits
 *
 * \effects
 * Reads bits from the bit buffer and file stream.  The bit buffer
 * will be modified as necessary.  bits is treated as a big endian
 * integer of length >= (count/8) + 1.
 *
 * \returns \c EOF for failure, otherwise the number of bits read.  If an
 * \c EOF is reached before all the bits are read, bits will contain every
 * bit through the last successful read.
 */
static int BitFileGetBitsBE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size)
{
    unsigned char *bytes;
    int offset, remaining, returnValue;

    if (count > (size * 8))
    {
        /* too many bits to read */
        return EOF;
    }

    bytes = (unsigned char *)bits;

    offset = size - 1;
    remaining = count;

    /* read whole bytes */
    while (remaining >= 8)
    {
        returnValue = BitFileGetChar(stream);

        if (returnValue == EOF)
        {
            return EOF;
        }

        bytes[offset] = (unsigned char)returnValue;
        remaining -= 8;
        offset--;
    }

    if (remaining != 0)
    {
        /* read remaining bits */
        while (remaining > 0)
        {
            returnValue = BitFileGetBit(stream);

            if (returnValue == EOF)
            {
                return EOF;
            }

            bytes[offset] <<= 1;
            bytes[offset] |= (returnValue & 0x01);
            remaining--;
        }

    }

    return count;
}

/**
 * \fn int BitFilePutBitsNum(bit_file_t *stream, void *bits,
 * const unsigned int count, const size_t size)
 *
 * \brief This function provides a machine independent layer that allows a
 * single function call to write an arbitrary number of bits from an integer
 * type variable into a file.
 *
 * \param stream A pointer to the bit file stream to write to
 *
 * \param bits The address to store bits to write
 *
 * \param count The number of bits to write
 *
 * \param size sizeof type containing \c bits
 *
 * \effects
 * Calls a function that writes bits to the bit buffer and file
 * stream.  The bit buffer will be modified as necessary.  The bits will be
 * written to the file stream from least significant byte to most significant
 * byte.
 *
 * \returns \c EOF for failure, \c -ENOTSUP for unsupported architecture,
 * otherwise the number of bits written.  If an error occurs after a partial
 * write, the partially written bits will not be unwritten.
 */
int BitFilePutBitsNum(bit_file_t *stream, void *bits, const unsigned int count,
    const size_t size)
{
    if ((stream == NULL) || (bits == NULL))
    {
        return EOF;
    }

    if (NULL == stream->PutBitsNumFunc)
    {
        return ENOTSUP;
    }

    /* call function that correctly handles endianess */
    return (stream->PutBitsNumFunc)(stream, bits, count, size);
}

/**
 * \fn static int BitFilePutBitsLE(bit_file_t *stream, void *bits,
 * const unsigned int count, const size_t size)
 *
 * \brief This function writes the specified number of bits from the memory
 * location passed as a parameter to the file passed as a parameter.  Bits
 * are written LSB to MSB.  The memory location is treated as a Little
 * Endian numeric data type.
 *
 * \param stream A pointer to the bit file stream to write to
 *
 * \param bits The address to store bits to write
 *
 * \param count The number of bits to write
 *
 * \param size sizeof type containing \c bits
 *
 * \effects
 * Writes bits to the bit buffer and file stream.  The bit buffer
 * will be modified as necessary.  bits is treated as a little endian integer
 * of length >= (count/8) + 1.
 *
 * \returns \c EOF for failure, otherwise the number of bits written.  If an
 * error occurs after a partial write, the partially written bits will not be
 * unwritten.
 */
static int BitFilePutBitsLE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size)
{
    unsigned char *bytes, tmp;
    int offset, remaining, returnValue;

    (void)size;
    bytes = (unsigned char *)bits;
    offset = 0;
    remaining = count;

    /* write whole bytes */
    while (remaining >= 8)
    {
        returnValue = BitFilePutChar(bytes[offset], stream);

        if (returnValue == EOF)
        {
            return EOF;
        }

        remaining -= 8;
        offset++;
    }

    if (remaining != 0)
    {
        /* write remaining bits */
        tmp = bytes[offset];
        tmp <<= (8 - remaining);

        while (remaining > 0)
        {
            returnValue = BitFilePutBit((tmp & 0x80), stream);

            if (returnValue == EOF)
            {
                return EOF;
            }

            tmp <<= 1;
            remaining--;
        }
    }

    return count;
}

/**
 * \fn static int BitFilePutBitsBE(bit_file_t *stream, void *bits,
 * const unsigned int count, const size_t size)
 *
 * \brief This function writes the specified number of bits from the memory
 * location passed as a parameter to the file passed as a parameter.  Bits
 * are written LSB to MSB.  The memory location is treated as a Big
 * Endian numeric data type.
 *
 * \param stream A pointer to the bit file stream to write to
 *
 * \param bits The address to store bits to write
 *
 * \param count The number of bits to write
 *
 * \param size sizeof type containing \c bits
 *
 * \effects
 * Writes bits to the bit buffer and file stream.  The bit buffer
 * will be modified as necessary.  bits is treated as a big endian integer
 * of length \c size.
 *
 * \returns \c EOF for failure, otherwise the number of bits written.  If an
 * error occurs after a partial write, the partially written bits will not be
 * unwritten.
 */
static int BitFilePutBitsBE(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size)
{
    unsigned char *bytes, tmp;
    int offset, remaining, returnValue;

    if (count > (size * 8))
    {
        /* too many bits to write */
        return EOF;
    }

    bytes = (unsigned char *)bits;
    offset = size - 1;
    remaining = count;

    /* write whole bytes */
    while (remaining >= 8)
    {
        returnValue = BitFilePutChar(bytes[offset], stream);

        if (returnValue == EOF)
        {
            return EOF;
        }

        remaining -= 8;
        offset--;
    }

    if (remaining != 0)
    {
        /* write remaining bits */
        tmp = bytes[offset];
        tmp <<= (8 - remaining);

        while (remaining > 0)
        {
            returnValue = BitFilePutBit((tmp & 0x80), stream);

            if (returnValue == EOF)
            {
                return EOF;
            }

            tmp <<= 1;
            remaining--;
        }
    }

    return count;
}

/**
 * \fn static int BitFileNotSupported(bit_file_t *stream, void *bits,
 * const unsigned int count, const size_t size)
 *
 * \brief This function returns /c -ENOTSUP.  It is called when a Get/PutBits
 * function is called on an unsupported architecture.
 *
 * \param stream Not Used
 *
 * \param bits Not Used
 *
 * \param count Not Used
 *
 * \param size Not Used
 *
 * \effects
 * None
 *
 * \returns \c -ENOTSUP
 */
static int BitFileNotSupported(bit_file_t *stream, void *bits,
    const unsigned int count, const size_t size)
{
    (void)stream;
    (void)bits;
    (void)count;
    (void)size;

    return -ENOTSUP;
}

/**@}*/
