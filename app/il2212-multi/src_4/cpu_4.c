#include <stdio.h>
#include "system.h"
#include "io.h"

#define FLAG4 SHARED_ONCHIP_BASE+3           //used to indicate if data has been written or not
#define FLAG5 SHARED_ONCHIP_BASE+4           //used to indicate if data has been read or not
#define FLAG6 SHARED_ONCHIP_BASE+5           //used to indicate if data has been read or not
#define LOC_sobeledgesX  SHARED_ONCHIP_BASE+1290           //address where sobel edges detection data(gx) is written
#define LOC_sobeledgesY  SHARED_ONCHIP_BASE+1488           //address where sobel edges detection data(gy) is written
#define LOC_sobel  SHARED_ONCHIP_BASE+1686           //address where toASCII data is written

void sobel();

void sobel(unsigned char* orig)
{
  int sizeX=orig[0];
	int sizeY=orig[1];
  unsigned char* sobel_img;
  unsigned char* gx;
  unsigned char* gy;
  sobel_img = LOC_sobel;
  gx = LOC_sobeledgesX+2;
  gy = LOC_sobeledgesY+2;
	int i,j;

  while(1)
  {
    if(IORD_8DIRECT(FLAG6,0)==0)
		{
			*sobel_img++=sizeX;
			*sobel_img++=sizeY;
		  for ( i=0; i<sizeY; i++)
		  {
	      for ( j=0; j<sizeX; j++)
	      {
          *sobel_img++ = (*gx++)+(*gy++);
//printf("%d ",*sobel_img);
	      }
//printf("\n");
		  }
      IOWR_8DIRECT(FLAG4,0,0);      //indicate data readx	
      IOWR_8DIRECT(FLAG5,0,0);      //indicate data read
		  IOWR_8DIRECT(FLAG6,0,1);
      break;
		}
	}
}

int main()
{
  printf("Hello from cpu_4!\n");

  while(1)
  {
    if((IORD_8DIRECT(FLAG4,0)==1)&&(IORD_8DIRECT(FLAG5,0)==1))     //wait for data to be written
    {
      sobel(LOC_sobeledgesX);
    }
  }
  return 0;
}
