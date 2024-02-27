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

int main(void) {
    int *image_gray;
    char *image_ascii;
    unsigned char *image_resize;
    int count = 0;
    int current_image = 0;

    while (1) {
        if (count % 100 == 0) {
            PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
            PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
        }

        PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
        image_gray = toGrayInt(image_sequence[current_image]);
        PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);

        PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
        image_ascii = toAscii(image_gray);
        PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_2);

        PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
        image_resize = resize(image_sequence[current_image]);
        PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_3);

        if (count % 100 == 99) {
            perf_print_formatted_report
            (PERFORMANCE_COUNTER_0_BASE,            
            ALT_CPU_FREQ,        // defined in "system.h"
            3,                   // How many sections to print
            "Gray",             // Display-name of section(s).
            "ASCII",
            "resize"
            );  
        }

        count++;
    }


  
    return 0;
}
