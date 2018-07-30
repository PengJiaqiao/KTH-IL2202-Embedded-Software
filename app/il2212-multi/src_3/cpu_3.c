#include <stdio.h>
#include "system.h"
#include "io.h"

#define FLAG3 SHARED_ONCHIP_BASE+2           //used to indicate if data has been written or not
#define FLAG5 SHARED_ONCHIP_BASE+4           //used to indicate if data has been read or not
#define LOC_resize_brightness  SHARED_ONCHIP_BASE+1032          //address where resize data is written
#define LOC_sobeledgesY  SHARED_ONCHIP_BASE+1488           //address where sobel edges detection data(gy) is written

void sobel_edges();

void sobel_edges(unsigned char* orig)
{
	int sizeX=orig[0];
	int sizeY=orig[1];
	int sizeXforsobel=sizeX-2;
	int sizeYforsobel=sizeY-2;
	unsigned char* sobel_img;
  sobel_img = LOC_sobeledgesY;
	int i,j;

	int tem[sizeX][sizeY];
  for ( i=0; i<sizeY; i++)
  {
    for ( j=0; j<sizeX; j++)
    {
      tem[i][j] = orig[j+i*sizeX+2];
    }
  }
	IOWR_8DIRECT(FLAG3,0,0);      //indicate data read

  while(1)
  {
    if(IORD_8DIRECT(FLAG5,0)==0)
		{
			*sobel_img++=sizeXforsobel;
			*sobel_img++=sizeYforsobel;
		  for ( i=0; i<sizeYforsobel; i++)
		  {
		      for ( j=0; j<sizeXforsobel; j++)
		      {
		          *sobel_img++ = (abs(tem[i+2][j] + (tem[i+2][j+1]<<1) + tem[i+2][j+2] - (tem[i][j+1]<<1) - tem[i][j+2] - tem[i][j]))>>6;
//printf("%d ",*sobel_img);
		      }//printf("\n");
		  }
			IOWR_8DIRECT(FLAG5,0,1);
			break;
		}
	}
}

int main()
{
  printf("Hello from cpu_3!\n");

  while(1)
  {
    if(IORD_8DIRECT(FLAG3,0)==1)     //wait for data to be written
    {
      sobel_edges(LOC_resize_brightness);
    }
  }
  return 0;
}
