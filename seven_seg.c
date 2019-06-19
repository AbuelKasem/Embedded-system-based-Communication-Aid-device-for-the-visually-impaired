/*
 * main.c
 *
 * This program is a simple piece of code to test the Watchdog Timer.
 *
 * The watchdog will be reset any time a button is pressed.
 *
 * The value of the watchdog timer is displayed on the red LEDs.
 *
 *  Created on: 13 Jan 2018
 *      Author: Doug from Up.
 *       Notes: Squirrel!
 */


#include "seven_seg.h"

void seg_hex(int x,int seg,int *ptr){
int shft;
    switch (seg){
    case 0: shft=0;
    break;
    case 1: shft=8;
    break;
    case 2: shft=16;
    break;
    case 3: shft=24;
    break;

    }
	if ((x<=15) && (x>=0)){


	switch(x) {
		         case 0:    {*ptr=0x00;
		        	 *ptr=*ptr| (0x3F<<shft);  //0
		         }
		         break;
		         case 1:   {*ptr=0x00;
		         *ptr=*ptr| (0x06<<shft);  //1
		         }
		         break;
		         case 2:   {*ptr=0x00;
		         *ptr=*ptr| (0x5B<<shft);//2
		         }
		         break;
		         case 3:  {*ptr=0x00;
		         *ptr=*ptr| (0x4F<<shft);//3
		         }
		         break;
		         case 4:   {*ptr=0x00;
		         *ptr=*ptr| (0x66<<shft);//4
		         }
		         break;
		         case 5:   {*ptr=0x00;
		         *ptr=*ptr| (0x6D<<shft);//5
		         }
		         break;
		         case 6:   {*ptr=0x00;
		         *ptr=*ptr|(0x5F<<shft);//6
		         }
		         break;
		         case 7:  {*ptr=0x00;
		         *ptr=*ptr|(0x07<<shft);//7
		         }
		         break;
		         case 8:   {*ptr=0x00;
		         *ptr=*ptr| (0x7F<<shft);//8
		         }
		         break;
		         case 9:   {*ptr=0x00;
		         *ptr=*ptr| (0x6F<<shft);//9
		         }
		         break;
		         case 10:   {*ptr=0x00;
		         *ptr=*ptr| (0x77<<shft);//A
		         }
		         break;
		         case 11:   {*ptr=0x00;
		         *ptr=*ptr| (0x7F<<shft);//B
		         }
		         break;
		         case 12:   {*ptr=0x00;
		         *ptr=*ptr| (0x39<<shft);//C
		         }
		         break;
		         case 13:   {*ptr=0x00;
		         *ptr=*ptr|(0x3F<<shft);//D
		         }
		         break;
		         case 14:  {*ptr=0x00;
		         *ptr=*ptr| (0x79<<shft);//E
		         }
		         break;
		         case 15:   {*ptr=0x00;
		         *ptr=*ptr| (0x71<<shft);//F
		         }
		         }
	}
	else *ptr=*ptr| 0x40; // display a - as an error

}
