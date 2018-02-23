/*
  C source for an OMP parallel demonstration program.
  example.c (C) 2018 by:
  Scientific Volume Imaging Holding B.V.
  Laapersveld 63,
  1213 VB Hilversum,
  The Netherlands,
  email: info@svi.nl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Include the header files needed for basic I/O and for OMP parallel
   work. Note that you need to also call the compiler with an omp flag in
   order to actually use omp. */ 

#include <stdio.h>
#include <stdlib.h> 
#include <omp.h>

/* Define a C data type "SizeType" that is a signed integer with the same
   number of bits as a pointer: it is suitable for array indexing up to any
   size that fits in memory. */
typedef long long int           SizeType;

/* Define the pixel data type of the source image. */
typedef unsigned char           srcPixelType;

/* Define some values for our example image. */
#define DIM_X ((SizeType)48)
#define DIM_Y ((SizeType)48)
#define DIM_Z ((SizeType)44) 
#define VOLUME (DIM_X * DIM_Y * DIM_Z)
#define FNAME "binaryImg.txt" /* VOLUME characters of ascii '0' and '1' */ 

srcPixelType srcData[VOLUME];

/* Read the source image into the source data array, convert ascii to binary. */
void readSrcImg(void)
{
    SizeType i;
    FILE     *fp = NULL;
    size_t   elemSize, elemCnt, elemRead; 

    fp = fopen(FNAME,"rb");
    if (fp == NULL) {
        printf("Failed to open %s for reading. \n", FNAME);
        exit(1);
    }

    /* C library function to read data: 
       size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream); */
    elemSize = sizeof(srcPixelType);
    elemCnt = (size_t)VOLUME;
    elemRead = fread(&srcData, elemSize, elemCnt, fp);
    if (elemRead != elemCnt) {
        printf("Failed to read %zu bytes from %s.\n",
               elemSize * elemCnt, FNAME);
        exit(1); 
    }
    fclose(fp);

        /* Convert ASCII '0' and '1' into binary 0 and 1. */
#pragma omp parallel for
    for (i = 0; i < VOLUME; i++) {
        switch(srcData[i]) {
            case '0':
                srcData[i] = 0;
                break;
            case '1':
                srcData[i] = 1;
                break;
            default:
                printf("Warning: character with ascii value %d encountered"
                       "at 1-D index %lldd while expecting either %d or %d.\n",
                       srcData[i], i, '0', '1');
                srcData[i] = 0; 
        }
    }
}


int main(void)
{
    printf("Dims: %lld, %lld, %lld\n", DIM_X, DIM_Y, DIM_Z);
    printf("Volume: %lld\n", VOLUME);

        /* Read the source image. */
    readSrcImg();
    
    printf("Done.\n");
	printf("Press enter to continue...\n");
	getchar();
}
