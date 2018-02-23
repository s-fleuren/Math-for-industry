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
#include <stddef.h>
#include <stdlib.h> 
#include <omp.h>

/* Define a C data type "SizeType" that is a signed integer with the same
   number of bits as a pointer: it is suitable for array indexing up to any
   size that fits in memory. The format string for "ptrdiff_t" is "%td". */
typedef ptrdiff_t               SizeType;

/* Define the pixel data types of the source and destination images. */
typedef unsigned char           srcPixelType;
typedef unsigned short int      dstPixelType; 

/* Define some values for our example image. */
#define DIM_X ((SizeType)48)
#define DIM_Y ((SizeType)48)
#define DIM_Z ((SizeType)44) 
#define VOLUME (DIM_X * DIM_Y * DIM_Z)
#define FNAME "binaryImg.txt" /* VOLUME characters of ascii '0' and '1' */ 


/* Allocate memory for two 1-D arrays each with VOLUME elements and the
   specified data types. */ 
void allocateImages(srcPixelType     **srcDataPtrPtr,
                    dstPixelType     **dstDataPtrPtr)
{
    srcPixelType *srcPtr;
    dstPixelType *dstPtr; 

    srcPtr = malloc(VOLUME * sizeof(srcPixelType));
    if (srcPtr == NULL) {
        printf("Failed to allocate %zu bytes of memory for the source. \n",
               VOLUME * sizeof(srcPixelType));
        exit(1);
    }

    dstPtr = malloc(VOLUME * sizeof(dstPixelType));
    if (dstPtr == NULL) {
        printf("Failed to allocate %zu bytes of memory for the destination. \n",
               VOLUME * sizeof(dstPixelType));
        exit(1);
    }

    *srcDataPtrPtr = srcPtr;
    *dstDataPtrPtr = dstPtr; 
}

/* Read the source image into the source data array, convert ascii to binary. */
void readSrcImg(srcPixelType *srcDataPtr)
{
    SizeType i;
    FILE     *fp;
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
    elemRead = fread(srcDataPtr, elemSize, elemCnt, fp);
    if (elemRead != elemCnt) {
        printf("Failed to read %zu bytes from %s.\n",
               elemSize * elemCnt, FNAME);
        exit(1); 
    }
    fclose(fp);

        /* Convert ASCII '0' and '1' into binary 0 and 1. */
#pragma omp parallel for
    for (i = 0; i < VOLUME; i++) {
        switch(srcDataPtr[i]) {
            case '0':
                srcDataPtr[i] = 0;
                break;
            case '1':
                srcDataPtr[i] = 1;
                break;
            default:
                printf("Warning: character with ascii value %d encountered"
                       "at 1-D index %td while expecting either %d or %d.\n",
                       srcDataPtr[i], i, '0', '1');
                srcDataPtr[i] = 0; 
        }
    }
}

void process(const srcPixelType *srcDataPtr, dstPixelType *dstDataPtr)
{
    SizeType inx;

        /* Loop (in parallel) over the destination pixels, do the work that
           needs to be done for each destination pixel. Local variables
           declared inside an omp parallel for loop are local to each thread
           by default. */
#pragma omp parallel for     
    for(inx = 0; inx < VOLUME; inx++) {
        SizeType       i, j, k, temp;
        const SizeType planeVol = DIM_X * DIM_Y;
        dstPixelType   result;
        SizeType       srcInx; 

        k = inx / planeVol; 
        temp = inx % planeVol; /* % is modulo or remainder after division. */
        j = temp / DIM_X;
        i = temp % DIM_X;
        result = 0; 

            /* Check the 6-connected neighbor pixels and increment the result
               for each of them that exists and is not zero. */ 
        if (i >= 1) {
            srcInx = k * planeVol + j * DIM_X + (i - 1);
            if (srcDataPtr[srcInx] != 0) result++;
        }
        if (i < DIM_X - 1) {
            srcInx = k * planeVol + j * DIM_X + (i + 1);
            if (srcDataPtr[srcInx] != 0) result++;
        }

        if (j >= 1) {
            srcInx = k * planeVol + (j - 1) * DIM_X + i;
            if (srcDataPtr[srcInx] != 0) result++;
        }
        if (j < DIM_Y - 1) {
            srcInx = k * planeVol + (j + 1) * DIM_X + i;
            if (srcDataPtr[srcInx] != 0) result++;
        }
        
        if (k >= 1) {
            srcInx = (k - 1) * planeVol + j * DIM_X + i;
            if (srcDataPtr[srcInx] != 0) result++;
        }
        if (k < DIM_Z - 1) {
            srcInx = (k + 1) * planeVol + j * DIM_X + i;
            if (srcDataPtr[srcInx] != 0) result++;
        }

            /* Write result in a non-overlapping way between the threads. */ 
        dstDataPtr[inx] = result; 
    } /* End of OMP parallel for. */ 
}

/* Function to provide some output based on the destination image. We
   give a histogram of connections. */ 
void someOutput(const dstPixelType *dstDataPtr)
{
    SizeType sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
    SizeType sum4 = 0, sum5 = 0, sum6 = 0;
    SizeType inx; 
    
        /* A reduction for the operator + and a variable will do the operation
           in parallel, yet OMP will make sure that the final result from all 
           threads is accumulated into a single variable. */ 

#pragma omp parallel for \
    reduction(+:sum0) \
    reduction(+:sum1) \
    reduction(+:sum2) \
    reduction(+:sum3) \
    reduction(+:sum4) \
    reduction(+:sum6) 
    
    for (inx = 0; inx < VOLUME; inx++) {
        dstPixelType neighborCnt = dstDataPtr[inx];
        switch(neighborCnt) {
            case 0:
                sum0++;
                break;
            case 1:
                sum1++;
                break;
            case 2:
                sum2++;
                break;
            case 3:
                sum3++;
                break;
            case 4:
                sum4++;
                break;
            case 5:
                sum5++;
                break;
            case 6:
                sum6++;
                break;
            default:
                printf("Warning: unexpected value %hu encountered at "
                       "index %td.\n", neighborCnt, inx);
        }
    } /* End of OMP parallel for. */ 
    printf("The number of pixels having 0 neighbors is: %td.\n", sum0); 
    printf("The number of pixels having 1 neighbor  is: %td.\n", sum1); 
    printf("The number of pixels having 2 neighbors is: %td.\n", sum2); 
    printf("The number of pixels having 3 neighbors is: %td.\n", sum3); 
    printf("The number of pixels having 4 neighbors is: %td.\n", sum4); 
    printf("The number of pixels having 5 neighbors is: %td.\n", sum5); 
    printf("The number of pixels having 6 neighbors is: %td.\n", sum6);
    printf("Total: %td. \n", sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6); 
}

void freeImages(srcPixelType *srcDataPtr, dstPixelType *dstDataPtr)
{
    if (srcDataPtr != NULL) {
        free(srcDataPtr); 
    } else {
        printf("Warning: srcDataPtr was NULL when attempting to free it.\n");
    }

    if (dstDataPtr != NULL) {
        free(dstDataPtr); 
    } else {
        printf("Warning: dstDataPtr was NULL when attempting to free it.\n");
    }
}

int main(void)
{
    srcPixelType   *srcDataPtr;
    dstPixelType   *dstDataPtr;
    
    printf("Dims: %td, %td, %td\n", DIM_X, DIM_Y, DIM_Z);
    printf("Volume: %td\n", VOLUME);

        /* Allocate memory for source and destination images. In order to get
           a pointer as result, a pointer to a pointer is used as function
           argument. */
    allocateImages(&srcDataPtr, &dstDataPtr);

        /* Read the source image. */
    readSrcImg(srcDataPtr);

        /* Do the processing. */
    process(srcDataPtr, dstDataPtr);

        /* Do some output of results. */
    someOutput(dstDataPtr);

    freeImages(srcDataPtr, dstDataPtr);
    printf("Done.\n");
	printf("Press enter to continue...\n");
	getchar();
}
