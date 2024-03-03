#include "altera_avalon_performance_counter.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"
#include "system.h"

#include "../../hello_image/src_0/images.h"
#include "../../include/SDF.h"

#define DEBUG 0

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    task3_stk[TASK_STACKSIZE];
OS_STK    StartTask_Stack[TASK_STACKSIZE]; 

/* Definition of Task Priorities */

#define STARTTASK_PRIO      1
#define TASK1_PRIORITY      13
#define TASK2_PRIORITY      12
#define TASK3_PRIORITY      11

/* Definition of Task Periods (ticks) */
#define TASK1_PERIOD 100
#define TASK2_PERIOD 100
#define TASK3_PERIOD 100

#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3

#define COUNT 5

/*
 * Global variables
 */
float *image_gray; // gray scale image
char *image_ascii; // ascii image
unsigned char *image_resize; // resized image

// Semaphores
OS_EVENT *Task1TmrSem;
OS_EVENT *Task2TmrSem;
OS_EVENT *Task3TmrSem;
OS_EVENT *Task1DoneSem;
OS_EVENT *Task2DoneSem;
OS_EVENT *Task3DoneSem;

// SW-Timer
OS_TMR *Task1Tmr;
OS_TMR *Task2Tmr;
OS_TMR *Task3Tmr;

/* Timer Callback Functions */ 
void Task1TmrCallback (void *ptmr, void *callback_arg){
  OSSemPost(Task1TmrSem);
  if (DEBUG) {
    printf("OSSemPost(Task1Sem);\n");
  }
}

void Task2TmrCallback (void *ptmr, void *callback_arg){
  OSSemPost(Task2TmrSem);
  if (DEBUG) {
    printf("OSSemPost(Task2Sem);\n");
  }
}

void Task3TmrCallback (void *ptmr, void *callback_arg){
  OSSemPost(Task3TmrSem);
  if (DEBUG) {
    printf("OSSemPost(Task3Sem);\n");
  }
}

/* Turn a rgb image to gray scale and write it to shared memory */
void task1(void* pdata)
{
  INT8U err;
	INT8U current_image=0;
  unsigned char *image = NULL;
  int count = 0;

	while (1)
	{ 
		OSSemPend(Task3DoneSem, 0, &err);
    if (DEBUG) {
      printf("task 1--\n");
    }

		if (count == 0) {
      PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
		  PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
    }
    
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
		
		/* Measurement here */
    toGrayFloat(image_sequence[current_image], image_gray);

		PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);    

    /* Print report every 100 executions */
    if (count == COUNT - 1) {
      perf_print_formatted_report
      (PERFORMANCE_COUNTER_0_BASE,            
      ALT_CPU_FREQ,        // defined in "system.h"
      3,                   // How many sections to print
      "Gray",             // Display-name of section(s).
      "ASCII",
      "Resize"
      ); 
    }
    
    /* Just to see that the task compiles correctly */
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,1);

		OSSemPend(Task1TmrSem, 0, &err);

		/* Increment the image pointer */
		current_image=(current_image+1) % sequence_length;

    OSSemPost(Task1DoneSem);

    count = (count + 1) % COUNT;
  }
}


void task2(void* pdata)
{
  INT8U err;

  while (1)
  { 
    OSSemPend(Task1DoneSem, 0, &err);
    if (DEBUG) {
      printf("task 2--\n");
    }
    
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
		
		/* Measurement here */
    toAscii(image_gray, image_ascii); // turn to ascii

		PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_2);

		/* Just to see that the task compiles correctly */
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,2);

    OSSemPend(Task2TmrSem, 0, &err);
    OSSemPost(Task2DoneSem);
  }
}

void task3(void *pdata) {
  INT8U err;
  int current_image = 0;

  while (1)
  { 
    OSSemPend(Task2DoneSem, 0, &err);
    if (DEBUG) {
      printf("task 3--\n");
    }
    
    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
        
    /* Measurement here */
    resize(image_sequence[current_image], image_resize);

    PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_3);

    /* Just to see that the task compiles correctly */
    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,4);

    OSSemPend(Task3TmrSem, 0, &err);
    OSSemPost(Task3DoneSem);
  }
}


void StartTask(void* pdata)
{
  INT8U err;
  void* context;

  /* 
   * Create and start Software Timer 
   */

   //Create Task1 Timer
   Task1Tmr = OSTmrCreate(0, //delay
                            TASK1_PERIOD, //period
                            OS_TMR_OPT_PERIODIC,
                            Task1TmrCallback, //OS_TMR_CALLBACK
                            (void *)0,
                            "Task1Tmr",
                            &err);
                            
   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if creation successful
      printf("Task1Tmr created\n");
    }
   }
   
   //Create Task2 Timer
   Task2Tmr = OSTmrCreate(0, //delay
                            TASK2_PERIOD, //period
                            OS_TMR_OPT_PERIODIC,
                            Task2TmrCallback, //OS_TMR_CALLBACK
                            (void *)0,
                            "Task2Tmr",
                            &err);
                            
   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if creation successful
      printf("Task2Tmr created\n");
    }
   }

   //Create Task3 Timer
   Task3Tmr = OSTmrCreate(0, //delay
                            TASK3_PERIOD, //period
                            OS_TMR_OPT_PERIODIC,
                            Task3TmrCallback, //OS_TMR_CALLBACK
                            (void *)0,
                            "Task3Tmr",
                            &err);
                            
   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if creation successful
      printf("Task3Tmr created\n");
    }
   }

   /*
    * Start timers
    */
   
   //start Task1 Timer
   OSTmrStart(Task1Tmr, &err);
   
   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if start successful
      printf("Task1Tmr started\n");
    }
   }

   //start Task2 Timer
   OSTmrStart(Task2Tmr, &err);
   
   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if start successful
      printf("Task2Tmr started\n");
    }
   }  

   //start Task3 Timer
   OSTmrStart(Task3Tmr, &err);
   
   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if start successful
      printf("Task3Tmr started\n");
    }
   }  

   /*
   * Creation of Kernel Objects
   */

  Task1TmrSem = OSSemCreate(0);   
  Task2TmrSem = OSSemCreate(0);  
  Task3TmrSem = OSSemCreate(0);    
  Task1DoneSem = OSSemCreate(0);
  Task2DoneSem = OSSemCreate(0);
  Task3DoneSem = OSSemCreate(1);

  /*
   * Create statistics task
   */

  OSStatInit();

  /* 
   * Creating Tasks in the system 
   */

  err=OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

  if (DEBUG) {
     if (err == OS_ERR_NONE) { //if start successful
      printf("Task1 created\n");
    }
   }  

  err=OSTaskCreateExt(task2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

  if (DEBUG) {
     if (err == OS_ERR_NONE) { //if start successful
      printf("Task 2 created\n");
    }
   }  

   err=OSTaskCreateExt(task3,
                  NULL,
                  (void *)&task3_stk[TASK_STACKSIZE-1],
                  TASK3_PRIORITY,
                  TASK3_PRIORITY,
                  task3_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

  if (DEBUG) {
     if (err == OS_ERR_NONE) { //if start successful
      printf("Task 3 created\n");
    }
   }  

  printf("All Tasks and Kernel Objects generated!\n");

  /* Task deletes itself */

  OSTaskDel(OS_PRIO_SELF);
}


int main(void) {
  printf("MicroC/OS-II-Vesion: %1.2f\n", (double) OSVersion()/100.0);

  image_gray = malloc(SIZE_X * SIZE_Y * sizeof(float)); // gray scale image
  image_ascii = malloc(SIZE_X * SIZE_Y * sizeof(char)); // ascii image
  image_resize = malloc((SIZE_X * SIZE_Y * 3 / 4 + OFFSET) * sizeof(unsigned char)); // resized image

     
  OSTaskCreateExt(
	 StartTask, // Pointer to task code
         NULL,      // Pointer to argument that is
                    // passed to task
         (void *)&StartTask_Stack[TASK_STACKSIZE-1], // Pointer to top
						     // of task stack 
         STARTTASK_PRIO,
         STARTTASK_PRIO,
         (void *)&StartTask_Stack[0],
         TASK_STACKSIZE,
         (void *) 0,  
         OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
         
  OSStart();
  
  return 0;
}
