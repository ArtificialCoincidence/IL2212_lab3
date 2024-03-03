// Skeleton for lab 2
// 
// Task 1 writes periodically RGB-images to the shared memory
//
// No guarantees provided - if bugs are detected, report them in the Issue tracker of the github repository!

#include <stdio.h>
#include "io.h"

#define DEBUG 0

#define CPU3_DONE       0X102003
#define CPU3_TODO       0X102008

#define IMAGE_ORIGINAL2     0X102700
#define IMAGE_GRAY2         0X103300

#define SIZE_X 64
#define SIZE_Y 8
#define MAX_VAL 255

void printGray(unsigned char* image, int x_dim, int y_dim) {
  int k = 0;
  int l = 0;
  char asciiChars[] = {' ','.',':','-','=','+','/','t','z','U','w','*','0','#','%','@'};
  
  for(k = 0; k < y_dim; k++) {
    for(l = 0; l < x_dim; l++) {
      unsigned char pixel = image[k * x_dim + l];
      printf("%c", asciiChars[((16 - 1) * pixel) / 255]);
    }
    printf("\n");
  }
}

/* Generate gray scale image image_gray from a p3 image pimage. Use unsigned char for image_gray to reduce size. */
void toGrayInt(unsigned char *pimage, unsigned char *image_gray)
{
  unsigned char red, green, blue;
  int i, j;
  unsigned char *image;

  image = pimage;

  if (DEBUG) {
    printf("Executing: p3 to gray scale \n");
  }

  for (i = 0; i < SIZE_Y; i++) {
    for (j = 0; j < SIZE_X; j++) {
      red = *image++;
      green = *image++;
      blue = *image++;
      *(image_gray + i * SIZE_X + j) = (5 * red + 9 * green + 2 * blue) / 16;
    }
  }
}

int main(void) {
  printf("cpu_3 begin\n");

  while (1) {
    if (IORD_8DIRECT(CPU3_TODO, 0) == 1 && IORD_8DIRECT(CPU3_DONE, 0) == 0) {
      IOWR_8DIRECT(CPU3_TODO, 0, 0);

      if (DEBUG) {
        printf("graySDF2 execute!\n");
        printGray((unsigned char *) IMAGE_ORIGINAL2, SIZE_X, SIZE_Y);
      }

      toGrayInt((unsigned char *) IMAGE_ORIGINAL2, (unsigned char *) IMAGE_GRAY2);

      if (DEBUG) {
        printGray((unsigned char *) IMAGE_GRAY2, SIZE_X, SIZE_Y);
      }

      IOWR_8DIRECT(CPU3_DONE, 0, 1);
    }
  }

  return 0;
}
