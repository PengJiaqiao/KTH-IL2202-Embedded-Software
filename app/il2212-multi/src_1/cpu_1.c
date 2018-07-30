#include <stdio.h>
#include "system.h"
#include "io.h"

#define FLAG1 SHARED_ONCHIP_BASE             //used to indicate if data has been written or not
#define FLAG2 SHARED_ONCHIP_BASE+1           //used to indicate if data has been read or not
#define FLAG3 SHARED_ONCHIP_BASE+2           //used to indicate if data has been read or not
#define LOC_grayscale  SHARED_ONCHIP_BASE+6           //address where grayscale data is written
#define LOC_resize_brightness  SHARED_ONCHIP_BASE+1032           //address where resize data is written

void resize_brightness();

void resize_brightness(unsigned char* orig)
{
	int sizeX=orig[0];
	int sizeY=orig[1];
	int sizeX_1=sizeX>>1;
	int sizeY_1=sizeY>>1;
	int fullsize=sizeX_1*sizeY_1;
	int i,j;
	int brimax=0,brimin=255;
	int enable=1;
	unsigned char grayscale[sizeX*sizeY+2];
	for ( i=0;i<sizeX*sizeY+2;i++)
	{
		grayscale[i]=orig[i];//copy original data
	}
	IOWR_8DIRECT(FLAG1,0,0);      //indicate data read

	unsigned char resize_img[fullsize+2];
	unsigned char* resize_brightness_img;
	resize_brightness_img = LOC_resize_brightness;
	*resize_brightness_img++=sizeX_1;
	*resize_brightness_img++=sizeY_1;

  while(1)
  {
    if((IORD_8DIRECT(FLAG2, 0)==0) && (IORD_8DIRECT(FLAG3, 0)==0))
    {
    	for ( i=0;i<sizeY;i=i+2)
    	{
    		for ( j=0;j<sizeX;j=j+2)
    		{
          *resize_brightness_img++=(grayscale[i*sizeX+j+2]+grayscale[i*sizeX+j+3]+grayscale[(i+1)*sizeX+j+2]+grayscale[(i+1)*sizeX+j+3])>>2;
    		}
    	}
			resize_brightness_img = LOC_resize_brightness+2;

			for(i=0;i<fullsize;i++)
			{
				if(*resize_brightness_img>brimax)
				{
					brimax=*resize_brightness_img;
				}
				if(*resize_brightness_img<brimin)
				{
					brimin=*resize_brightness_img;
				}
				resize_brightness_img++;
			}
			if (brimax-brimin<=127)
			{
				enable=0;
			}
			while (enable==0)
			{
				resize_brightness_img = LOC_resize_brightness+2;
				for( i=0;i<fullsize;i++)
				{
					if (*resize_brightness_img-brimin>63)
					*resize_brightness_img=(*resize_brightness_img-brimin)<<1;
						else if (*resize_brightness_img-brimin>31)
						*resize_brightness_img=(*resize_brightness_img-brimin)<<2;
							else if (*resize_brightness_img-brimin>15)
							*resize_brightness_img=(*resize_brightness_img-brimin)<<3;
								else
								*resize_brightness_img=(*resize_brightness_img-brimin)<<4;
					resize_brightness_img++;
				}
				resize_brightness_img = LOC_resize_brightness+2;
				for( i=0;i<fullsize;i++)
				{
					if(*resize_brightness_img>brimax)
					{
						brimax=*resize_brightness_img;
					}
					if(*resize_brightness_img<brimin)
					{
						brimin=*resize_brightness_img;
					}
					resize_brightness_img++;
				}
				if (brimax-brimin>127)
				{
					enable=1;
				}
    	}
			IOWR_8DIRECT(FLAG2,0,1);
			IOWR_8DIRECT(FLAG3,0,1);
			break;
		}
  }
}

int main()
{
  printf("Hello from cpu_1!\n");

  while(1)
  {
    if(IORD_8DIRECT(FLAG1,0)==1)     //wait for data to be written

//should it wait for next function to finish reading???

    {
      resize_brightness(LOC_grayscale);
    }
  }
  return 0;
}
