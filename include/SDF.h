#include <stdio.h>

/* Definition of image parameters */
#define SIZE_X   64
#define SIZE_Y   64
#define MAX_VAL  255
#define OFFSET   3

void toGrayFloat(unsigned char *pimage, float *image_gray)
{
  int red, green, blue;
  int i, j;
  unsigned char *image;

  image = pimage + OFFSET; // skip size_x, size_y, max_val

  for (i = 0; i < SIZE_Y; i++) {
    for (j = 0; j < SIZE_X; j++) {
      red = (int) *image++;
      green = (int) *image++;
      blue = (int) *image++;
      *(image_gray + i * SIZE_X + j) = 0.3125 * red + 0.5625 * green + 0.125 * blue;
    }
  }
}

void toGrayInt(unsigned char *pimage, int *image_gray)
{
  int red, green, blue;
  int i, j;
  unsigned char *image;

  image = pimage + OFFSET; // skip size_x, size_y, max_val

  printf("Executing: p3 to gray scale \n");

  for (i = 0; i < SIZE_Y; i++) {
    for (j = 0; j < SIZE_X; j++) {
      red = (int) *image++;
      green = (int) *image++;
      blue = (int) *image++;
      *(image_gray + i * SIZE_X + j) = (5 * red + 9 * green + 2 * blue) / 16;
    }
  }
}

void toAscii(int *image_gray, char *image_ascii)
{
  int i, j;
  char asciiChars[] = {' ','.',':','-','=','+','/','t','z','U','w','*','0','#','%','@'};

  for (i = 0; i < SIZE_Y; i++) {
    for (j = 0; j < SIZE_X; j++) {
      *(image_ascii + i * SIZE_X + j) = asciiChars[(int)(15 * *(image_gray + i * SIZE_X + j) / MAX_VAL)];
    }
  }
}

void printAscii(const char *image_ascii)
{
  int i, j;

  for (i = 0; i < SIZE_Y; i++) {
    for (j = 0; j < SIZE_X; j++) {
        printf("%c", *(image_ascii + i * SIZE_X + j));
    }
    printf("\n");
  }
}

void resize(unsigned char *pimage, unsigned char *image_resize)
{
    int i, j, k;
    int left, right, up, down;
    unsigned char *image;

    for (i = 0; i < OFFSET; i++) {
      *(image_resize + i) = *(pimage + i);
    }

    image = pimage + OFFSET;

    for (i = 0; i < SIZE_Y / 2; i++) {
        for (j = 0; j < SIZE_X / 2; j++) {
            for (k = 0; k < 3; k++) {
                left = 2 * j;
                right = left + 1;
                up = 2 * i;
                down = up + 1;
                *(image_resize + (i * SIZE_X + j) * 3 + k + OFFSET) = ((int) *(image + (up * SIZE_X + left) * 3 + k) + (int) *(image + (up * SIZE_X + right) * 3 + k)
                                       + (int) *(image + (down * SIZE_X + left) * 3 + k) + (int) *(image + (down * SIZE_X + right) * 3 + k)) / 4;
            }
        }
    }
}

/*
 * function for copying a p3 image from sram to the shared on-chip mempry
 */
void sram2sm_p3(unsigned char* base, unsigned char* shared)
{
	int x, y;

	int size_x = *base++;
	int size_y = *base++;
	int max_col= *base++;
	*shared++  = size_x;
	*shared++  = size_y;
	*shared++  = max_col;
	printf("Executing: sram to shared memory \n");
	for(y = 0; y < size_y; y++)
	for(x = 0; x < size_x; x++)
	{
		*shared++ = *base++; 	// R
		*shared++ = *base++;	// G
		*shared++ = *base++;	// B
	}
}