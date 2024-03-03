// Skeleton for lab 2
// 
// Task 1 writes periodically RGB-images to the shared memory
//
// No guarantees provided - if bugs are detected, report them in the Issue tracker of the github repository!

#include <stdio.h>
#include "io.h"

#define DEBUG 0

#define CPU4_DONE       0X102004
#define CPU4_TODO       0X102009

#define IMAGE_ORIGINAL     0X102100
#define IMAGE_RESIZE       0X103900

#define SIZE_X 64
#define SIZE_Y 16
#define MAX_VAL 255

void printP3ToAscii(unsigned char* image, int x_dim, int y_dim) {
  int k = 0;
  int l = 0;
  char asciiChars[] = {' ','.',':','-','=','+','/','t','z','U','w','*','0','#','%','@'};
  for(k = 0; k < y_dim; k++) {
    for(l = 0; l < x_dim; l++) {
      unsigned char pixel_r = image[(k * x_dim + l) * 3];
      unsigned char pixel_g = image[(k * x_dim + l) * 3 + 1];
      unsigned char pixel_b = image[(k * x_dim + l) * 3 + 2];
      unsigned char pixel = (5 * pixel_r + 9 * pixel_g + 2 * pixel_b) / 16;
      // Clamp pixel value to 255
      unsigned char c_pixel = pixel > 255 ? 255 : pixel;
      // Print normalized value as ASCII character
      printf("%c", asciiChars[((16 - 1) * c_pixel) / 255]);
    }
    printf("\n");
  }
}

void resize(unsigned char *image, unsigned char *image_resize)
{
    unsigned char i, j, k;
    unsigned char left, right, up, down;

    for (i = 0; i < SIZE_Y / 2; i++) {
        for (j = 0; j < SIZE_X / 2; j++) {
            for (k = 0; k < 3; k++) {
                left = 2 * j;
                right = left + 1;
                up = 2 * i;
                down = up + 1;
                *(image_resize + (i * SIZE_X / 2 + j) * 3 + k) = (*(image + (up * SIZE_X + left) * 3 + k) + *(image + (up * SIZE_X + right) * 3 + k)
                                       + *(image + (down * SIZE_X + left) * 3 + k) + *(image + (down * SIZE_X + right) * 3 + k)) / 4;
            }
        }
    }
}

int main(void) {
  printf("cpu_4 begin\n");

  while (1) {
    if (IORD_8DIRECT(CPU4_TODO, 0) == 1 && IORD_8DIRECT(CPU4_DONE, 0) == 0) {
      IOWR_8DIRECT(CPU4_TODO, 0, 0);

      if (DEBUG) {
        printf("resizeSDF execute!\n");
        printP3ToAscii((unsigned char *) IMAGE_ORIGINAL, SIZE_X, SIZE_Y);
      }

      resize((unsigned char *) IMAGE_ORIGINAL, (unsigned char *) IMAGE_RESIZE);

      if (DEBUG) {
        printP3ToAscii((unsigned char *) IMAGE_RESIZE, SIZE_X / 2, SIZE_Y / 2);
      }

      IOWR_8DIRECT(CPU4_DONE, 0, 1);
    }
  }

  return 0;
}
