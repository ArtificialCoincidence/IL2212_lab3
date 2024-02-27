#include "altera_avalon_performance_counter.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"
#include "system.h"

#include "../../hello_image/src_0/images.h"
#include "../../include/SDF.h"

#define DEBUG 0

#define HW_TIMER_PERIOD 100 /* 100ms */

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    StartTask_Stack[TASK_STACKSIZE]; 

/* Definition of Task Priorities */

#define STARTTASK_PRIO      1
#define TASK1_PRIORITY      13
#define TASK2_PRIORITY      12

/* Definition of Task Periods (ms) */
#define TASK1_PERIOD 400
#define TASK2_PERIOD 500

#define SECTION_1 1
#define SECTION_2 2

/*
 * Global variables
 */
float *image_gray; // gray scale image

// Semaphores
OS_EVENT *Task1TmrSem;
OS_EVENT *Task2TmrSem;
OS_EVENT *Task1DoneSem;
OS_EVENT *Task2DoneSem;

// SW-Timer
OS_TMR *Task1Tmr;
OS_TMR *Task2Tmr;

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

/* Turn a rgb image to gray scale and write it to shared memory */
void task1(void* pdata)
{
  INT8U err;
	INT8U current_image=0;
  unsigned char *image = NULL;
  int count = 0;

	while (1)
	{ 
		OSSemPend(Task2DoneSem, 0, &err);
    if (DEBUG) {
      printf("task 1--\n");
    }

		if (count % 100 == 0) {
      PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
		  PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
    }
    
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
		
		/* Measurement here */
    image_gray = toGrayFloat(image_sequence[current_image]);

		PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);    

    /* Print report every 100 executions */
    if (count == 99) {
      perf_print_formatted_report
      (PERFORMANCE_COUNTER_0_BASE,            
      ALT_CPU_FREQ,        // defined in "system.h"
      2,                   // How many sections to print
      "Gray",             // Display-name of section(s).
      "ASCII"
      );  
    }
    
    /* Just to see that the task compiles correctly */
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,1);

		OSSemPend(Task1TmrSem, 0, &err);

		/* Increment the image pointer */
		current_image=(current_image+1) % sequence_length;

    OSSemPost(Task1DoneSem);

    count++;
  }
}


void task2(void* pdata)
{
  INT8U err;
  float *image_gray;
  char *image_ascii;

  while (1)
  { 
    OSSemPend(Task1DoneSem, 0, &err);
    if (DEBUG) {
      printf("task 2--\n");
    }
    
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
		
		/* Measurement here */
    image_ascii = toAscii(image_gray); // turn to ascii

		PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_2);

		/* Just to see that the task compiles correctly */
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,2);

    OSSemPend(Task2TmrSem, 0, &err);
    OSSemPost(Task2DoneSem);
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
                            TASK1_PERIOD/HW_TIMER_PERIOD, //period
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
   
   //Create Task1 Timer
   Task2Tmr = OSTmrCreate(0, //delay
                            TASK2_PERIOD/HW_TIMER_PERIOD, //period
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

   /*
   * Creation of Kernel Objects
   */

  Task1TmrSem = OSSemCreate(0);   
  Task2TmrSem = OSSemCreate(0);   
  Task1DoneSem = OSSemCreate(0);
  Task2DoneSem = OSSemCreate(1);

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

  printf("All Tasks and Kernel Objects generated!\n");

  /* Task deletes itself */

  OSTaskDel(OS_PRIO_SELF);
}


int main(void) {


  printf("MicroC/OS-II-Vesion: %1.2f\n", (double) OSVersion()/100.0);
     
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
