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

/* Define some values for our example image. */
#define DIM_X 48
#define DIM_Y 48
#define DIM_Z 44 
#define VOLUME (DIM_X * DIM_Y * DIM_Z)
#define FNAME "binaryImg.txt" /* VOLUME characters of ascii '0' and '1' */ 

/* Declare a 3-D array. Note that the nesting of the dimensions is done
   such that in memory X is changing the fastest and Z the slowest. */ 
unsigned char srcData3D[DIM_Z][DIM_Y][DIM_X];

/* Read the source image into the source data array element by element,
   convert ascii to binary. */
void readSrcImg(void)
{
    int i, j, k;
    FILE     *fp;
    size_t    elemSize, elemCnt, elemRead; 

    fp = fopen(FNAME,"rb");
    if (fp == NULL) {
        printf("Failed to open %s for reading. \n", FNAME);
        exit(1);
    }

    /* C library function to read data: 
       size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream); */
    
    elemSize = 1;
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
                               "encountered at 3-D index %d, %d, %d "
                               "while expecting either %d or %d.\n",
                               srcData3D[k][j][i], i, j, k, '0', '1');
                        srcData3D[k][j][i] = 0; 
                }
            }
        }
    }
}

int main(void)
{
    printf("Dims: %d, %d, %d\n", DIM_X, DIM_Y, DIM_Z);
    printf("Volume: %d\n", VOLUME);

        /* Read the source image. */
    readSrcImg();

    printf("Done.\n");
	printf("Press enter to continue...\n");
	getchar();
    return 0;
}
