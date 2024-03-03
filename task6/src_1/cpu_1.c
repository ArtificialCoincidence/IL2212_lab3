// Skeleton for lab 2
// 
// Task 1 writes periodically RGB-images to the shared memory
//
// No guarantees provided - if bugs are detected, report them in the Issue tracker of the github repository!

#include <stdio.h>
#include "io.h"

#define DEBUG 0

#define CPU1_DONE       0X102001
#define CPU1_TODO       0X102006

#define IMAGE_GRAY          0X103100
#define IMAGE_ASCII         0X103500

#define SIZE_X 64
#define SIZE_Y 16
#define MAX_VAL 255

void printAscii(unsigned char* image, int x_dim, int y_dim) {
  int k = 0;
  int l = 0;
  for(k = 0; k < y_dim; k++) {
    for(l = 0; l < x_dim; l++) {
      unsigned char pixel = image[k * x_dim + l];
      printf("%c", pixel);
    }
    printf("\n");
  }
}

void toAscii(unsigned char *image_gray, char *image_ascii)
{
  int i, j;
  char asciiChars[] = {' ','.',':','-','=','+','/','t','z','U','w','*','0','#','%','@'};

  for (i = 0; i < SIZE_Y; i++) {
    for (j = 0; j < SIZE_X; j++) {
      *(image_ascii + i * SIZE_X + j) = asciiChars[(int)(15 * ((int) *(image_gray + i * SIZE_X + j)) / MAX_VAL)];
    }
  }
}

int main(void) {
  printf("cpu_1 begin\n");

  while (1) {
    if (IORD_8DIRECT(CPU1_TODO, 0) == 1 && IORD_8DIRECT(CPU1_DONE, 0) == 0) {
      IOWR_8DIRECT(CPU1_TODO, 0, 0);

      if (DEBUG) {
        printf("asciiSDF execute!\n");
      }

      toAscii((unsigned char *) IMAGE_GRAY, (unsigned char *) IMAGE_ASCII);

      if (DEBUG) {
        printAscii((unsigned char *) IMAGE_ASCII, SIZE_X, SIZE_Y);
      }

      IOWR_8DIRECT(CPU1_DONE, 0, 1);
    }
  }

  return 0;
}
