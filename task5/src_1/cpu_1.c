// Skeleton for lab 2
// 
// Task 1 writes periodically RGB-images to the shared memory
//
// No guarantees provided - if bugs are detected, report them in the Issue tracker of the github repository!

#include <stdio.h>
#include "io.h"

#define DEBUG 0

#define CPU1_DONE       0X102001
#define CPU1_TODO       0X102003

#define IMAGE_SHARE       0X102100

#define SIZE_X 64
#define SIZE_Y 64
#define MAX_VAL 255

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

      toAscii((unsigned char *) IMAGE_SHARE, (unsigned char *) IMAGE_SHARE);

      IOWR_8DIRECT(CPU1_DONE, 0, 1);
    }
  }

  return 0;
}
