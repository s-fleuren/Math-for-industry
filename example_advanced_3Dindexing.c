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


/* Alocate memory for two 3-D arrays each with [DIM_Z][DIM_Y][DIM_X]
   elements and the specified data types. Return the pointers to the
   3-D arrays by reference (i.e. as yet another pointer to them). */
void allocateImages(srcPixelType ****srcDataPtrPtrPtrPtr,
                    dstPixelType ****dstDataPtrPtrPtrPtr)
{
    SizeType        j, k;
    srcPixelType   *src1D;
    srcPixelType  **src2D;
    srcPixelType ***src3D;
    dstPixelType   *dst1D;
    dstPixelType  **dst2D;
    dstPixelType ***dst3D;

    src3D = (srcPixelType ***)malloc(DIM_Z * sizeof(srcPixelType **));
    if (src3D == NULL) {
        printf("Failed to in allocating source image. \n");
        exit(1); 
    }
    for (k = 0; k < DIM_Z; k++) {
        src2D = (srcPixelType **)malloc(DIM_Y * sizeof(srcPixelType *));
        if (src2D == NULL) {
            printf("Failed to in allocating source image. \n");
            exit(1); 
        }
        
        for (j = 0; j < DIM_Y; j++) {
            src1D = (srcPixelType *)malloc(DIM_X * sizeof(srcPixelType));
            if (src1D == NULL) {
                printf("Failed to in allocating source image. \n");
                exit(1); 
            }

            src2D[j] = src1D; 
        }
        src3D[k] = src2D; 
    }

    dst3D = (dstPixelType ***)malloc(DIM_Z * sizeof(dstPixelType **));
    if (dst3D == NULL) {
        printf("Failed to in allocating source image. \n");
        exit(1); 
    }
    for (k = 0; k < DIM_Z; k++) {
        dst2D = (dstPixelType **)malloc(DIM_Y * sizeof(dstPixelType *));
        if (dst2D == NULL) {
            printf("Failed to in allocating destination image. \n");
            exit(1); 
        }
        
        for (j = 0; j < DIM_Y; j++) {
            dst1D = (dstPixelType *)malloc(DIM_X * sizeof(dstPixelType));
            if (dst1D == NULL) {
                printf("Failed to in allocating destination image. \n");
                exit(1); 
            }

            dst2D[j] = dst1D; 
        }
        dst3D[k] = dst2D; 
    }

    *srcDataPtrPtrPtrPtr = src3D;
    *dstDataPtrPtrPtrPtr = dst3D; 
}

/* Read the source image into the source data array element by element,
   convert ascii to binary. */
void readSrcImg(srcPixelType ***srcData3D)
{
    SizeType  i, j, k;
    FILE     *fp;
    size_t    elemSize, elemCnt, elemRead; 

    fp = fopen(FNAME,"rb");
    if (fp == NULL) {
        printf("Failed to open %s for reading. \n", FNAME);
        exit(1);
    }

    /* C library function to read data: 
       size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream); */
    
    elemSize = sizeof(srcPixelType);
    elemCnt  = 1;
    for (k = 0; k < DIM_Z; k++) {
        for (j = 0; j < DIM_Y; j++) {
            for (i = 0; i < DIM_X; i++) {
                elemRead = fread(&(srcData3D[k][j][i]), elemSize, elemCnt, fp);
                if (elemRead != elemCnt) {
                    printf("Failed to read %zu bytes from %s.\n",
                           elemSize * elemCnt, FNAME);
                    exit(1);
                }
            }
        }
    }
    fclose(fp);

        /* Convert ASCII '0' and '1' into binary 0 and 1. */
#pragma omp parallel for collapse(3) private(i,j)
    for (k = 0; k < DIM_Z; k++) {
        for (j = 0; j < DIM_Y; j++) {
            for (i = 0; i < DIM_X; i++) {
                switch(srcData3D[k][j][i]) {
                    case '0':
                        srcData3D[k][j][i] = 0; 
                        break;
                    case '1':
                        srcData3D[k][j][i] = 1;
                            break;
                    default:
                        printf("Warning: character with ascii value %d "
                               "encountered at 3-D index %td, %td, %td "
                               "while expecting either %d or %d.\n",
                               srcData3D[k][j][i], i, j, k, '0', '1');
                        srcData3D[k][j][i] = 0; 
                }
            }
        }
    }
}

void process(srcPixelType ***srcData3D, dstPixelType ***dstData3D)
{
    SizeType i, j, k; 

        /* Loop (in parallel) over the destination pixels, do the work that
           needs to be done for each destination pixel. Local variables
           declared inside an omp parallel for loop are local to each thread
           by default. */
#pragma omp parallel for collapse(3) private(i,j) 
    for (k = 0; k < DIM_Z; k++) {
        for (j = 0; j < DIM_Y; j++) {
            for (i = 0; i < DIM_X; i++) {
                dstPixelType   result;

                result = 0;
                if (i >= 1) {
                    if (srcData3D[k][j][i - 1] != 0) result++;
                }
                if (i < DIM_X - 1) {
                    if (srcData3D[k][j][i + 1] != 0) result++;
                }

                if (j >= 1) {
                    if (srcData3D[k][j - 1][i] != 0) result++;
                }
                if (j < DIM_Y - 1) {
                    if (srcData3D[k][j + 1][i] != 0) result++;
                }

                if (k >= 1) {
                    if (srcData3D[k - 1][j][i] != 0) result++;
                }
                if (k < DIM_Z - 1) {
                    if (srcData3D[k + 1][j][i] != 0) result++;
                }

                    /* Write result in a non-overlapping way between the
                       threads. */ 
                dstData3D[k][j][i] = result;
            }
        }
    } /* End of OMP parallel for. */ 
}

/* Function to provide some output based on the destination image. We
   give a histogram of connections. */ 
void someOutput(dstPixelType ***dstData3D)
{
    SizeType sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
    SizeType sum4 = 0, sum5 = 0, sum6 = 0;
    SizeType i, j, k;
    
        /* A reduction for the operator + and a variable will do the operation
           in parallel, yet OMP will make sure that the final result from all 
           threads is accumulated into a single variable. */ 

#pragma omp parallel for collapse(3) private(i,j) \
    reduction(+:sum0) \
    reduction(+:sum1) \
    reduction(+:sum2) \
    reduction(+:sum3) \
    reduction(+:sum4) \
    reduction(+:sum5) \
    reduction(+:sum6) 
    for (k = 0; k < DIM_Z; k++) {
        for (j = 0; j < DIM_Y; j++) {
            for (i = 0; i < DIM_X; i++) {
                dstPixelType neighborCnt = dstData3D[k][j][i];
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
                               "index %td, %td, %td.\n", neighborCnt, i, j, k);
                }
            }
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

void freeImages(srcPixelType ***srcData3D, dstPixelType ***dstData3D)
{
    SizeType j, k;

    for (k = 0; k < DIM_Z; k++) {
        for (j = 0; j < DIM_Y; j++) {
            if (srcData3D[k][j] != NULL) {
                free(srcData3D[k][j]);
            } else {
                printf("Warning: srcData3D[][] was NULL during free.\n");
            }
        }
        if (srcData3D[k] != NULL) {
            free(srcData3D[k]);
        } else {
            printf("Warning: srcData3D[] was NULL during free.\n");
        }
    }
    if (srcData3D != NULL) {
        free(srcData3D);
    } else {
        printf("Warning: srcData3D was NULL during free.\n");
    }
    
    for (k = 0; k < DIM_Z; k++) {
        for (j = 0; j < DIM_Y; j++) {
            if (dstData3D[k][j] != NULL) {
                free(dstData3D[k][j]);
            } else {
                printf("Warning: dstData3D[][] was NULL during free.\n");
            }
        }
        if (dstData3D[k] != NULL) {
            free(dstData3D[k]);
        } else {
            printf("Warning: dstData3D[] was NULL during free.\n");
        }
    }
    if (dstData3D != NULL) {
        free(dstData3D);
    } else {
        printf("Warning: dstData3D was NULL during free.\n");
    }
}

int main(void)
{
    srcPixelType   ***srcData3D;
    dstPixelType   ***dstData3D;
    
    printf("Dims: %td, %td, %td\n", DIM_X, DIM_Y, DIM_Z);
    printf("Volume: %td\n", VOLUME);

        /* Allocate memory for source and destination images. Passing by
           reference results in a pointer to a pointer to a pointer to a
           pointer being passed. */ 
    allocateImages(&srcData3D, &dstData3D);

        /* Read the source image. */
    readSrcImg(srcData3D);

        /* Do the processing. */
    process(srcData3D, dstData3D);

        /* Do some output of results. */
    someOutput(dstData3D);

    freeImages(srcData3D, dstData3D);
    printf("Done.\n");
	printf("Press enter to continue...\n");
	getchar();
    return 0;
}
