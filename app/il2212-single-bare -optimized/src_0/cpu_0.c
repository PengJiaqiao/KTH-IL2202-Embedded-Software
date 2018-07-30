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

#define grayscale1 1
#define resize2 2
#define brightness3 3
#define sobel4 4
#define toASCII5 5

unsigned char symbols[16] = {32, 46, 59, 42, 115, 116, 73, 40, 106, 117, 86, 119, 98, 82, 103, 64};

void grayscale(unsigned char* orig)
{
	#if DEBUG == 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, grayscale1);
	#endif

	int sizeX=*orig++;//orig[0];
	int sizeY=*orig++;//orig[1];
	int fullsize=sizeX*sizeY;
	int i;
	unsigned char* grayscale_img[fullsize+3];
	unsigned char* grayscale_pointer = grayscale_img;
	unsigned char r, g, b;

	*grayscale_pointer++=sizeX;//grayscale_img[0]=sizeX;
	*grayscale_pointer++=sizeY;//grayscale_img[1]=sizeY;
	*grayscale_pointer++=*orig++;//grayscale_img[2]=orig[2];
	for( i=0;i<fullsize;i++)
	{
		r=*orig++;
		g=*orig++;
		b=*orig++;
		*grayscale_pointer++ = (((r<<2)+ r)>>4)+(((g<<3)+ g)>>4)+(b>>3);//orig[3 * i + 3 ] + 0.5625 * orig[3 * i + 4] + 0.125  * orig[3 * i + 5];
	}
	#if DEBUG == 1
	PERF_END(PERFORMANCE_COUNTER_0_BASE, grayscale1);
	#endif
	resize (grayscale_img);
}

void resize(unsigned char* orig)
{
	#if DEBUG == 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, resize2);
	#endif

	int sizeX=orig[0];
	int sizeY=orig[1];
	int fullsize=sizeX*sizeY;
	int i,j;
	unsigned char resize_img[(fullsize>>2)+3];

	resize_img[0]=sizeX>>1;
	resize_img[1]=sizeY>>1;
	resize_img[2]=orig[2];

	for ( i=0;i<sizeY;i=i+2)
	{
		for ( j=0;j<sizeX;j=j+2)
		{
		resize_img[(j>>1)+(i*sizeX>>2)+3]=(orig[i*sizeX+j+3]+orig[i*sizeX+j+4]+orig[(i+1)*sizeX+j+3]+orig[(i+1)*sizeX+j+4])>>2;
		}
	}
	#if DEBUG == 1
	PERF_END(PERFORMANCE_COUNTER_0_BASE, resize2);
	#endif
	brightness(resize_img);
}

void brightness (unsigned char* orig)
{
	#if DEBUG == 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, brightness3);
	#endif

	int sizeX=orig[0];
	int sizeY=orig[1];
	int Max=orig[2];
	int brimax=0,brimin=255;
	int enable=1;
	int i;
	unsigned char brightness_img[Max];

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
			brightness_img[i+3]=(orig[i+3]-brimin)<<1;
				else if (orig[i+3]-brimin>31)
				brightness_img[i+3]=(orig[i+3]-brimin)<<2;
					else if (orig[i+3]-brimin>15)
					brightness_img[i+3]=(orig[i+3]-brimin)<<3;
						else
						brightness_img[i+3]=(orig[i+3]-brimin)<<4;

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
	#if DEBUG == 1
	PERF_END(PERFORMANCE_COUNTER_0_BASE, brightness3);
	#endif
	sobel(brightness_img);
}

void sobel(unsigned char* orig)
{
	#if DEBUG == 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, sobel4);
	#endif

	int sizeX=orig[0];
	int sizeY=orig[1];
	int sizeXforsobel=sizeX-2;
	int sizeYforsobel=sizeY-2;
	unsigned char sobel_img[sizeXforsobel*sizeYforsobel+2];
	int gx,gy;
	int i,j;

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

  for ( i=0; i<sizeYforsobel; i++)
  {
      for ( j=0; j<sizeXforsobel; j++)
      {
          gx = tem[i][j+2] - (tem[i+1][j]<<1) + (tem[i+1][j+2]<<1) - tem[i][j] - tem[i+2][j] + tem[i+2][j+2];
          gy = tem[i+2][j] + (tem[i+2][j+1]<<1) + tem[i+2][j+2] - (tem[i][j+1]<<1) - tem[i][j+2] - tem[i][j];
          sobel_img[j+i*sizeXforsobel+3] = (abs(gx)+abs(gy))>>6;//sqrt(gx*gx + gy*gy)/4;
      }
  }
	#if DEBUG == 1
	printf ("sobel:\n");
	for( i=0;i<sizeYforsobel;i++)
		{
			for( j=0;j<sizeXforsobel;j++)
			{
				printf("%3d",sobel_img[ i * sizeX + j+3]);
			}
			printf("\n");
		}
	PERF_END(PERFORMANCE_COUNTER_0_BASE, sobel4);
	#endif
	toASCII(sobel_img);
}

void toASCII(unsigned char* orig)
{
	#if DEBUG == 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, toASCII5);
	#endif

	int sizeX = orig[0], sizeY = orig[1];
	int i,j;
	unsigned char ASCII[sizeX * sizeY+3];

	ASCII[0] = sizeX ;
	ASCII[1] = sizeY ;
	ASCII[2] = orig[2] ;

	for( i = 0; i < sizeX * sizeY; i++)
	{
		ASCII[i+3] = symbols[(orig[i+3])];
	}
	#if DEBUG == 1
	printf ("ASCII:\n");
	for( i = 0; i < sizeY; i++)
		{
		for( j=0; j< sizeX;j++)
			{
			printf("%c ",ASCII[j+(i*sizeX)+3]);
			}
		printf("\n");
		}

	PERF_END(PERFORMANCE_COUNTER_0_BASE, toASCII5);
	#endif
}

int main(void)
{
  printf("MicroC/OS-II-Vesion: %1.2f\n", (double) OSVersion()/100.0);
  char number_of_images=3;
	unsigned char* img_array[3] = {img1_32_32, img2_32_32, img3_32_32};
	char current_image=0;
	int count = 0;

	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
	PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);

	#if DEBUG == 1
	while(1)
	{
		/* Measurement here */
		grayscale (img_array[current_image]);

		/* Print report */
		perf_print_formatted_report
		(PERFORMANCE_COUNTER_0_BASE,
		ALT_CPU_FREQ,        // defined in "system.h"
		5,                   // How many sections to print
		"grayscale",        // Display-name of section(s).
  	    "resize",
   		"brightness",
  	  	"sobel",
    	"toASCII"
		);

		/* Increment the image pointer */
		current_image=(current_image+1) % number_of_images;
	#else
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, 1);
	while(count < 100)
	{
		/* Measurement here */
		grayscale (img_array[current_image]);

		/* Increment the image pointer */
		current_image=(current_image+1) % number_of_images;
		count++;
	}
	PERF_END(PERFORMANCE_COUNTER_0_BASE, 1);

	/* Print report */
	perf_print_formatted_report(PERFORMANCE_COUNTER_0_BASE,
							 ALT_CPU_FREQ,        // defined in "system.h"
							 1,                   // How many sections to print
							 "Section 1"        // Display-name of section(s).
							 );
	printf("Number of images processed: %d\n",count);
	#endif

  return 0;
}
