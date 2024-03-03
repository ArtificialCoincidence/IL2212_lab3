#include <stdio.h>
#include "altera_avalon_performance_counter.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"
#include "system.h"

#include "../../hello_image/src_0/images.h"
#include "../../include/SDF.h"

#define DEBUG 0

#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3

#define COUNT 5

int task1(int current_image, int *image_gray)
{
    toGrayInt(image_sequence[current_image], image_gray);
    return 0;
}

int task2(int *image_gray, char *image_ascii)
{
    toAscii(image_gray, image_ascii);
    return 0;
}

int task3(int current_image, unsigned char *image_resize)
{
    resize(image_sequence[current_image], image_resize);
    return 0;
}

int main(void) {
    int *image_gray = malloc(SIZE_X * SIZE_Y * sizeof(int)); // gray scale image
    char *image_ascii = malloc(SIZE_X * SIZE_Y * sizeof(char));
    unsigned char *image_resize = malloc((SIZE_X * SIZE_Y * 3 / 4 + OFFSET) * sizeof(unsigned char));
    int count = 0;
    int current_image = 0;
    int res1, res2, res3;
    res1 = res2 = res3 = -1;

    while (1) {
        if (count == 0) {
            PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
            PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
        }

        if (res1 == -1 && res2 == -1 && res3 == -1) {
            PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
            res1 = task1(current_image, image_gray);
            PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,1);
        }

        if (res1 == 0 && res2 == -1 && res3 == -1) {
            PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
            res2 = task2(image_gray, image_ascii);
            PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,2);
        }

        if (res1 == 0 && res2 == 0 && res3 == -1) {
            PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
            res3 = task3(current_image, image_resize);
            PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,4);

            // reset
            res1 = res2 = res3 = -1;

            // count execution times
            count = (count + 1) % COUNT;
        }
        

        if (count == COUNT - 1) {
            perf_print_formatted_report
            (PERFORMANCE_COUNTER_0_BASE,            
            ALT_CPU_FREQ,        // defined in "system.h"
            3,                   // How many sections to print
            "Gray",             // Display-name of section(s).
            "ASCII",
            "resize"
            );  
        }

        current_image = (current_image + 1) % sequence_length;
    }


  
    return 0;
}
