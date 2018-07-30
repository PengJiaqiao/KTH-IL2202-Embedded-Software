// Skeleton for lab 2
// 
// Task 1 writes periodically RGB-images to the shared memory
//
// No guarantees provided - if bugs are detected, report them in the Issue tracker of the github repository!

#include <stdio.h>
#include "altera_avalon_performance_counter.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"
#include "system.h"
#include "io.h"
#include "images.h"
void grayscale();
void resize();
void brightness();
void sobel();
void toASCII();

#define DEBUG 1

#define HW_TIMER_PERIOD 100 /* 100ms */

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    task3_stk[TASK_STACKSIZE];
OS_STK    task4_stk[TASK_STACKSIZE];
OS_STK    task5_stk[TASK_STACKSIZE];
OS_STK    StartTask_Stack[TASK_STACKSIZE]; 

/* Definition of Task Priorities */

#define STARTTASK_PRIO      1
#define TASK1_PRIORITY      10
#define TASK2_PRIORITY      11
#define TASK3_PRIORITY      12
#define TASK4_PRIORITY      13
#define TASK5_PRIORITY      14

/* Definition of Task Periods (ms) */
#define TASK1_PERIOD 10000

#define grayscale1 1
#define resize2 2
#define brightness3 3
#define sobel4 4
#define toASCII5 5

void grayscale(unsigned char* orig)
{	
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, grayscale1);

	int sizeX=orig[0];
	int sizeY=orig[1];
	int fullsize=sizeX*sizeY;
	int i,j;
	unsigned char grayscale_img[fullsize+3];
	unsigned char* imgP;
	unsigned char* share;
	
	grayscale_img[0]=sizeX;
	grayscale_img[1]=sizeY;
	grayscale_img[2]=orig[2];
	for(i=0;i<fullsize;i++)
	{
		grayscale_img[i + 3]= 0.3125 * orig[3 * i + 3 ] + 0.5625 * orig[3 * i + 4] + 0.125  * orig[3 * i + 5];
	}

	 imgP = grayscale_img;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	for (i=0;i<fullsize+3;i++)
	{
		*share++ = *imgP++;
	}
	PERF_END(PERFORMANCE_COUNTER_0_BASE, grayscale1);  
//	resize (grayscale_img);

}

void resize(unsigned char* orig)
{	
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, resize2);

	int sizeX=orig[0];
	int sizeY=orig[1];
	int fullsize=sizeX*sizeY;
	int i,j;
//	int s=1;
	unsigned char resize_img[fullsize/4+3];
	
	unsigned char* imgP;
	unsigned char* share;

	resize_img[0]=sizeX/2;
	resize_img[1]=sizeY/2;
	resize_img[2]=orig[2];
	
	for ( i=0;i<sizeY;i=i+2)
	{
		for ( j=0;j<sizeX;j=j+2)
		{
		resize_img[j/2+i*sizeX/4+3]=(orig[i*sizeX+j+3]+orig[i*sizeX+j+4]+orig[(i+1)*sizeX+j+3]+orig[(i+1)*sizeX+j+4])/4;
		}
	}

	 	imgP = resize_img;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	for (i=0;i<fullsize+3;i++)
	{
		*share++ = *imgP++;
	}

	PERF_END(PERFORMANCE_COUNTER_0_BASE, resize2);  
}

void brightness (unsigned char* orig)
{	
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, brightness3);

	int sizeX=orig[0];
	int sizeY=orig[1];
	int Max=orig[2];
	int fullsize=sizeX*sizeY;
	int brimax=0,brimin=255;
	int enable=1;
	int i,j;
	unsigned char brightness_img[Max];
	unsigned char* imgP;
	unsigned char* share;

	brightness_img[0]=sizeX;
	brightness_img[1]=sizeY;
	brightness_img[2]=Max;

	for( i=0;i<sizeX*sizeY;i++)
	{
		brightness_img[i+3]=orig[i+3];
		if(brightness_img[i+3]>brimax)
		{
			brimax=brightness_img[i+3];
		}
		if(brightness_img[i+3]<brimin)
		{
			brimin=brightness_img[i+3];
		}
	}
	if (brimax-brimin<=127)
	{
		enable=0;
	}
	while (enable==0)
	{
		for( i=0;i<sizeX*sizeY;i++)
		{
			if (brightness_img[i+3]-brimin>63)
			brightness_img[i+3]=(orig[i+3]-brimin)*2;
				else if (orig[i+3]-brimin>31)
				brightness_img[i+3]=(orig[i+3]-brimin)*4;
					else if (orig[i+3]-brimin>15)
					brightness_img[i+3]=(orig[i+3]-brimin)*8;
						else 
						brightness_img[i+3]=(orig[i+3]-brimin)*16;

		}
		for( i=0;i<sizeX*sizeY;i++)
		{
			if(brightness_img[i+3]>brimax)
			{
				brimax=brightness_img[i+3];
			}
			if(brightness_img[i+3]<brimin)
			{
				brimin=brightness_img[i+3];
			}
		}
		if (brimax-brimin>127)
		{
			enable=1;
		}
	}

			imgP = brightness_img;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	for (i=0;i<fullsize+3;i++)
	{
		*share++ = *imgP++;
	}

	PERF_END(PERFORMANCE_COUNTER_0_BASE, brightness3);  
} 

void  sobel(unsigned char* orig)
{	
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, sobel4);

	int sizeX=orig[0];
	int sizeY=orig[1];
	int sizeXforsobel=sizeX-2;
	int sizeYforsobel=sizeY-2;
	int fullsize=sizeXforsobel*sizeYforsobel;
	unsigned char sobel_img[sizeXforsobel*sizeYforsobel+3];
	int gx,gy;
	int i,j;
	unsigned char* imgP;
	unsigned char* share;

	sobel_img[0]=sizeXforsobel;
	sobel_img[1]=sizeYforsobel;
	sobel_img[2]=orig[2];
	int tem[sizeX][sizeY];
        for ( i=0; i<sizeY; i++)
        {
            for ( j=0; j<sizeX; j++)
            {
                tem[i][j] = orig[j+i*sizeX+3];
                
            }
        }
        
        
        
        for ( i=0; i<sizeY-2; i++)
        {
            for ( j=0; j<sizeX-2; j++)
            {
                
                gx = (-1) * tem[i][j] + tem[i][j+2] + (-2) * tem[i+1][j]   + 2*tem[i+1][j+2] + (-1) * tem[i+2][j] + tem[i+2][j+2];
                
                gy =  1 * tem[i][j]  + 2*tem[i][j+1] + 1*tem[i][j+2] + (-2) * tem[i+2][j+1] - tem[i+2][j] + (-1)*tem[i+2][j+2];
                
                sobel_img[j+i*sizeXforsobel+3] = sqrt(gx*gx + gy*gy)/4;
            }
            printf ("\n");
        }

	imgP = sobel_img;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	for (i=0;i<fullsize+3;i++)
	{
		*share++ = *imgP++;
	}

	PERF_END(PERFORMANCE_COUNTER_0_BASE, sobel4);  
}

void toASCII(unsigned char* orig)
{
PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, toASCII5);

int sizeX = orig[0], sizeY = orig[1];
int i,j;
int fullsize=sizeX*sizeY;
unsigned char ASCII[sizeX * sizeY+3];
unsigned char symbols[16] = {32, 46, 59, 42, 115, 116, 73, 40, 106, 117, 86, 119, 98, 82, 103, 64};
	unsigned char* imgP;
	unsigned char* share;

ASCII[0] = sizeX ;
ASCII[1] = sizeY ;
ASCII[2] = orig[2] ;
	printf ("ASCII:\n");

for( i = 0; i < sizeX * sizeY; i++)
{
ASCII[i+3] = symbols[((orig[i+3])/16)];
}
for( i = 0; i < sizeY; i++)
	{
	for( j=0; j< sizeX;j++)
		{
		printf("%c ",ASCII[j+(i*sizeX)+3]);
		}
	printf("\n");
	}
		imgP = ASCII;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	for (i=0;i<fullsize+3;i++)
	{
		*share++ = *imgP++;
	}

	PERF_END(PERFORMANCE_COUNTER_0_BASE, toASCII5);  
}



int delay; // Delay of HW-timer 

/*
 * ISR for HW Timer
 */
alt_u32 alarm_handler(void* context)
{
  OSTmrSignal(); /* Signals a 'tick' to the SW timers */
  
  return delay;
}

// Semaphores
OS_EVENT *Task1TmrSem;
OS_EVENT *Task1Sem;
OS_EVENT *Task2Sem;
OS_EVENT *Task3Sem;
OS_EVENT *Task4Sem;
OS_EVENT *Task5Sem;

// SW-Timer
OS_TMR *Task1Tmr;

/* Timer Callback Functions */ 
void Task1TmrCallback (void *ptmr, void *callback_arg){
  OSSemPost(Task1TmrSem);
}
void task1(void* pdata)
{
	INT8U err;
	INT8U value=0;
	char current_image=0;
	
	#if DEBUG == 1
	/* Sequence of images for testing if the system functions properly */
	char number_of_images=10;
	unsigned char* img_array[10] = {img1_24_24, img1_32_22, img1_32_32, img1_40_28, img1_40_40, 
			img2_24_24, img2_32_22, img2_32_32, img2_40_28, img2_40_40 };
	#else
	/* Sequence of images for measuring performance */
	char number_of_images=3;
	unsigned char* img_array[3] = {img1_32_32, img2_32_32, img3_32_32};
	#endif

	while (1)
	{ 
		/* Extract the x and y dimensions of the picture */
		unsigned char i = *img_array[current_image];
		unsigned char j = *(img_array[current_image]+1);
		OSSemPend(Task1Sem, 0, &err);

		PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
		PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
//		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);

		/* Measurement here */
   	grayscale (img_array[current_image]);

		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,value++);

		OSSemPost(Task2Sem);


		/* Increment the image pointer */
		current_image=(current_image+1) % number_of_images;

	}
}
void task2(void* pdata) {
	INT8U err;
	unsigned char* share;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	while(1)
	{
	OSSemPend(Task2Sem, 0, &err);
//	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, resizeImg2);		
		/* Measurement here */	
	resize(share);
//	PERF_END(PERFORMANCE_COUNTER_0_BASE, resizeImg2);
	// printf("hello from task2\n");
	
	OSSemPost(Task3Sem);
	}
}
void task3(void* pdata) {
	INT8U err;
	unsigned char* share;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	while(1)
	{
		
	OSSemPend(Task3Sem, 0, &err);
//	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, brightCorrect3);		
		/* Measurement here */	
	brightness(share);
//	PERF_END(PERFORMANCE_COUNTER_0_BASE, brightCorrect3);
	
	// printf("hello from task3\n");
	
	OSSemPost(Task4Sem);
	}
}
void task4(void* pdata) {
	INT8U err;
	unsigned char* share;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	while(1)
	{
	OSSemPend(Task4Sem, 0, &err);

	sobel(share);

	OSSemPost(Task5Sem);
	}
}
void task5(void* pdata) {
	INT8U err;
	unsigned char* share;
	share = (unsigned char*) SHARED_ONCHIP_BASE;
	while(1)
	{
	OSSemPend(Task5Sem, 0, &err);

	toASCII(share);


		/* Print report */
		perf_print_formatted_report
		(PERFORMANCE_COUNTER_0_BASE,            
		ALT_CPU_FREQ,        // defined in "system.h"
		5,                   // How many sections to print
		"rgbToGray",        // Display-name of section(s).
		"resizeImg",
		"brightCorrect",
		"sobel",
		"printAscii"
		);  
	OSSemPost(Task1Sem);		
	}
}

void StartTask(void* pdata)
{
  INT8U err;
  void* context;

  static alt_alarm alarm;     /* Is needed for timer ISR function */
  
  /* Base resolution for SW timer : HW_TIMER_PERIOD ms */
  delay = alt_ticks_per_second() * HW_TIMER_PERIOD / 1000; 
  printf("delay in ticks %d\n", delay);


  if (alt_alarm_start (&alarm,
      delay,
      alarm_handler,
      context) < 0)
      {
          printf("No system clock available!n");
      }


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


   /*
   * Creation of Kernel Objects
   */

  Task1TmrSem = OSSemCreate(1);   
  Task1Sem = OSSemCreate(1);   
  Task2Sem = OSSemCreate(0);
  Task3Sem = OSSemCreate(0);
  Task4Sem = OSSemCreate(0);
  Task5Sem = OSSemCreate(0);  
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
      printf("Task2 created\n");
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
      printf("Task3 created\n");
    }
   }  
   err=OSTaskCreateExt(task4,
                  NULL,
                  (void *)&task4_stk[TASK_STACKSIZE-1],
                  TASK4_PRIORITY,
                  TASK4_PRIORITY,
                  task4_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

  if (DEBUG) {
     if (err == OS_ERR_NONE) { //if start successful
      printf("Task4 created\n");
    }
   }  
   err=OSTaskCreateExt(task5,
                  NULL,
                  (void *)&task5_stk[TASK_STACKSIZE-1],
                  TASK5_PRIORITY,
                  TASK5_PRIORITY,
                  task5_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

  if (DEBUG) {
     if (err == OS_ERR_NONE) { //if start successful
      printf("Task5 created\n");
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
