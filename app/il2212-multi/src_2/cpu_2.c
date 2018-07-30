#include <stdio.h>
#include "system.h"
#include "io.h"

#define FLAG2 SHARED_ONCHIP_BASE+1           //used to indicate if data has been written or not
#define FLAG4 SHARED_ONCHIP_BASE+3           //used to indicate if data has been read or not
#define LOC_resize_brightness  SHARED_ONCHIP_BASE+1032           //address where resize data is written
#define LOC_sobeledgesX  SHARED_ONCHIP_BASE+1290           //address where sobel edges detection data(gx) is written

void sobel_edges();

void sobel_edges(unsigned char* orig)
{
	int sizeX=orig[0];
	int sizeY=orig[1];
	int sizeXforsobel=sizeX-2;
	int sizeYforsobel=sizeY-2;
	unsigned char* sobel_img;
  sobel_img = LOC_sobeledgesX;
	int i,j;

	int tem[sizeX][sizeY];
  for ( i=0; i<sizeY; i++)
  {
	  for ( j=0; j<sizeX; j++)
    {
	    tem[i][j] = orig[j+i*sizeX+2];
    }
  }
	IOWR_8DIRECT(FLAG2,0,0);      //indicate data read

	while(1)
  {
    if(IORD_8DIRECT(FLAG4,0)==0)
		{
			*sobel_img++=sizeXforsobel;
			*sobel_img++=sizeYforsobel;
		  for ( i=0; i<sizeYforsobel; i++)
		  {
		      for ( j=0; j<sizeXforsobel; j++)
		      {
		          *sobel_img++ = (abs(tem[i][j+2] - (tem[i+1][j]<<1) + (tem[i+1][j+2]<<1) - tem[i][j] - tem[i+2][j] + tem[i+2][j+2]))>>6;
		      }
		  }
			IOWR_8DIRECT(FLAG4,0,1);
			break;
		}
	}
}

int main()
{
  printf("Hello from cpu_2!\n");

  while(1)
  {
    if(IORD_8DIRECT(FLAG2,0)==1)     //wait for data to be written
    {
      sobel_edges(LOC_resize_brightness);
    }
  }
  return 0;
}
