#include <stdio.h>

/* Definition of image parameters */
#define SIZE_X   64
#define SIZE_Y   64
#define MAX_VAL  255

float *toGrayFloat(unsigned char *image)
{
  float image_gray[SIZE_X][SIZE_Y];
  int red, green, blue;
  int i, j;

  image += 3; // skip size_x, size_y, max_val

  for (i = 0; i < SIZE_X; i++) {
    for (j = 0; j < SIZE_Y; j++) {
      red = *image++;
      green = *image++;
      blue = *image++;
      image_gray[i][j] = 0.3125 * red + 0.5625 * green + 0.125 * blue;
    }
  }

  return image_gray;
}

int *toGrayInt(unsigned char *image)
{
  int image_gray[SIZE_X][SIZE_Y];
  int red, green, blue;
  int i, j;

  image += 3; // skip size_x, size_y, max_val

  for (i = 0; i < SIZE_X; i++) {
    for (j = 0; j < SIZE_Y; j++) {
      red = *image++;
      green = *image++;
      blue = *image++;
      image_gray[i][j] = (5 * red + 9 * green + 2 * blue) / 16;
    }
  }

  return &(image_gray[0][0]);
}

char *toAscii(int *image_gray)
{
  int i, j;
  int index;
  char image_ascii[SIZE_X][SIZE_Y];
  char asciiChars[] = {' ','.',':','-','=','+','/','t','z','U','w','*','0','#','%','@'};

  for (i = 0; i < SIZE_X; i++) {
    for (j = 0; j < SIZE_Y; j++) {
      image_ascii[i][j] = asciiChars[(int)(15 * *(image_gray + i * SIZE_Y + j) / MAX_VAL)];
    }
  }

  return &(image_ascii[0][0]);
}

void printAscii(char *image_ascii)
{
  int i, j;

  for (i = 0; i < SIZE_X; i++) {
    for (j = 0; j < SIZE_Y; j++) {
        printf("%c", *image_ascii++);
    }
    printf("\n");
  }
}

unsigned char *resize(unsigned char *image_p)
{
    int i, j, k;
    int left, right, up, down;
    // unsigned char image[SIZE_X][SIZE_Y][3];
    unsigned char *image;
    unsigned char *image_resize[SIZE_X / 2][SIZE_Y / 2][3];

    image = image_p + 3;
    // memcpy(&image[0][0][0], image_p, SIZE_X * SIZE_Y * 3 * sizeof(unsigned char));

    for (i = 0; i < SIZE_X / 2; i++) {
        for (j = 0; j < SIZE_Y / 2; j++) {
            for (k = 0; k < 3; k++) {
                left = 2 * j;
                right = left + 1;
                up = 2 * i;
                down = up + 1;
                image_resize[i][j][k] = (*(image + (up * SIZE_Y + left) * 3 + k) + *(image + (up * SIZE_Y + right) * 3 + k)
                                       + *(image + (down * SIZE_Y + left) * 3 + k) + *(image + (down * SIZE_Y + right) * 3 + k)) / 4;
            }
        }
    }

    return image_resize;
}