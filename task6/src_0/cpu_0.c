#include <stdio.h>
#include "altera_avalon_performance_counter.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"
#include "system.h"

#include "../../hello_image/src_0/images.h"

#define DEBUG 0

#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3

#define COUNT 5

#define CPU1_DONE       0X102001
#define CPU2_DONE       0X102002
#define CPU3_DONE       0X102003
#define CPU4_DONE       0X102004
#define CPU1_TODO       0X102006
#define CPU2_TODO       0X102007
#define CPU3_TODO       0X102008
#define CPU4_TODO       0X102009

#define IMAGE_ORIGINAL      0X102100
#define IMAGE_GRAY          0X103100
#define IMAGE_ASCII         0X103500
#define IMAGE_RESIZE        0X103900

#define OFFSET 3
#define SIZE_X 64
#define SIZE_Y 64
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

/* Write a p3 image to shared memory. */
void copyImageP3(unsigned char *image_original, unsigned char *image_copy, int size_x, int size_y)
{
  int i, j;
  unsigned char *pimage_original = image_original;
  unsigned char *pimage_copy = image_copy;

  for (i = 0; i < size_y; i++) {
    for (j = 0; j < size_x; j++) {
      *pimage_copy++ = *pimage_original++; // r
      *pimage_copy++ = *pimage_original++; // g
      *pimage_copy++ = *pimage_original++; // b
    }
  }
}

/* Write a gray scale image to shared memory. */
void copyImageGray(unsigned char *image_original, unsigned char *image_copy, int size_x, int size_y)
{
  int i, j;
  unsigned char *pimage_original = image_original;
  unsigned char *pimage_copy = image_copy;

  for (i = 0; i < size_y; i++) {
    for (j = 0; j < size_x; j++) {
      *pimage_copy++ = *pimage_original++;
    }
  }
}

/* Split pimage(64x64) into four 16x64 images image_split1 and image_split2 */
void splitImage(unsigned char *pimage, unsigned char *image_split1, unsigned char *image_split2,
                unsigned char *image_split3, unsigned char *image_split4)
{
  int i, j;
  unsigned char *image = pimage + OFFSET;

  for (i = 0; i < SIZE_Y / 4; i++) {
    for (j = 0; j < SIZE_X; j++) {
      *image_split1++ = *image++;
      *image_split1++ = *image++;
      *image_split1++ = *image++;
    }
  }

  for (i = 0; i < SIZE_Y / 4; i++) {
    for (j = 0; j < SIZE_X; j++) {
      *image_split2++ = *image++;
      *image_split2++ = *image++;
      *image_split2++ = *image++;
    }
  }

  for (i = 0; i < SIZE_Y / 4; i++) {
    for (j = 0; j < SIZE_X; j++) {
      *image_split3++ = *image++;
      *image_split3++ = *image++;
      *image_split3++ = *image++;
    }
  }

  for (i = 0; i < SIZE_Y / 4; i++) {
    for (j = 0; j < SIZE_X; j++) {
      *image_split4++ = *image++;
      *image_split4++ = *image++;
      *image_split4++ = *image++;
    }
  }
}

void reset(int count)
{
  // state reset
  IOWR_8DIRECT(CPU1_DONE, 0, 0);
  IOWR_8DIRECT(CPU2_DONE, 0, 0);
  IOWR_8DIRECT(CPU3_DONE, 0, 0);
  IOWR_8DIRECT(CPU4_DONE, 0, 0);
  IOWR_8DIRECT(CPU1_TODO, 0, 0);
  IOWR_8DIRECT(CPU2_TODO, 0, 0);
  IOWR_8DIRECT(CPU3_TODO, 0, 0);
  IOWR_8DIRECT(CPU4_TODO, 0, 0);
  
  if (count % COUNT == 0) {
    perf_print_formatted_report(PERFORMANCE_COUNTER_0_BASE, ALT_CPU_FREQ, 1, "Gray, Resize and Ascii");
    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
    PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
  }

  IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE, 0);
}

void executeGrayResizeAscii(int current_image, unsigned char *image_split[], unsigned char *image_gray,
                            unsigned char *image_resize, unsigned char *image_ascii)
{
  int i;
  unsigned char *pwrite_gray = image_gray;
  unsigned char *pwrite_resize = image_resize + OFFSET;
  unsigned char *pwrite_ascii = image_ascii;

  // write header
  *image_resize = (unsigned char) SIZE_X / 2;
  *(image_resize + 1) = (unsigned char) SIZE_Y / 2;
  *(image_resize + 2) = (unsigned char) MAX_VAL;

  for (i = 0; i < 4; i++) {
    // write image_originali to shared memory
    copyImageP3(image_split[i], (unsigned char *) IMAGE_ORIGINAL, SIZE_X, SIZE_Y / 4);
    IOWR_8DIRECT(CPU2_TODO, 0, 1); // graySDF1
    IOWR_8DIRECT(CPU3_TODO, 0, 1); // graySDF2
    IOWR_8DIRECT(CPU4_TODO, 0, 1); // resizeSDF

    if (DEBUG) {
        printf("Executing graySDF and resizeSDF\n");
    }
    while (IORD_8DIRECT(CPU2_DONE, 0) != 1 || IORD_8DIRECT(CPU3_DONE, 0) != 1) { continue;}
    // graySDF finish
    IOWR_8DIRECT(CPU2_DONE, 0, 0);
    IOWR_8DIRECT(CPU3_DONE, 0, 0);
    // asciiSDF begin
    IOWR_8DIRECT(CPU1_TODO, 0, 1);
    copyImageGray((unsigned char *) IMAGE_GRAY, pwrite_gray, SIZE_X, SIZE_Y / 4);

    while (IORD_8DIRECT(CPU4_DONE, 0) != 1) { continue;}
    // resizeSDF finish
    IOWR_8DIRECT(CPU4_DONE, 0, 0);
    copyImageP3((unsigned char *) IMAGE_RESIZE, pwrite_resize, SIZE_X / 2, SIZE_Y / 8);

    if (DEBUG) {
        printf("Executing asciiSDF\n");
    }
    while (IORD_8DIRECT(CPU1_DONE, 0) != 1) { continue;}
    // asciiSDF finish
    IOWR_8DIRECT(CPU1_DONE, 0, 0);
    copyImageGray((unsigned char *) IMAGE_ASCII, pwrite_ascii, SIZE_X, SIZE_Y / 4);

    pwrite_gray += SIZE_X * SIZE_Y / 4;
    pwrite_resize += SIZE_X * SIZE_Y * 3 / 16;
    pwrite_ascii += SIZE_X * SIZE_Y / 4;
  }
}

int main()
{
  printf("cpu_0 begin\n");

  int count = 0;
  int current_image = 0;
  
  unsigned char *image_original1 = malloc(SIZE_X * SIZE_Y * 3 / 4 * sizeof(unsigned char));
  unsigned char *image_original2 = malloc(SIZE_X * SIZE_Y * 3 / 4 * sizeof(unsigned char));
  unsigned char *image_original3 = malloc(SIZE_X * SIZE_Y * 3 / 4 * sizeof(unsigned char));
  unsigned char *image_original4 = malloc(SIZE_X * SIZE_Y * 3 / 4 * sizeof(unsigned char));
  unsigned char *image_original[] = {image_original1, image_original2, image_original3, image_original4};
  unsigned char *image_gray = malloc(SIZE_X * SIZE_Y * sizeof(unsigned char));
  unsigned char *image_ascii = malloc(SIZE_X * SIZE_Y * sizeof(unsigned char));
  unsigned char *image_resize = malloc((SIZE_X * SIZE_Y * 3 / 4 + OFFSET) * sizeof(unsigned char));

  while (1) {
    reset(count);

    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
    // split image_original
    splitImage(image_sequence[current_image], image_original1, image_original2, image_original3, image_original4);
    executeGrayResizeAscii(current_image, image_original, image_gray, image_resize, image_ascii);
    PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE, 1);

    if (DEBUG && count % COUNT == 0) {
      printAscii(image_ascii, SIZE_X, SIZE_Y);
      printf("--------------------------------------------------------------------\n");
      printP3ToAscii(image_resize + 3, SIZE_X / 2, SIZE_Y / 2);
    }

    count = (count + 1) % COUNT;
    current_image = (current_image + 1) % sequence_length;
  }
  return 0;
}