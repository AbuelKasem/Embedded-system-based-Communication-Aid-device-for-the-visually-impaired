/*
 * Argo+(two-way communication) for Blind
 * ------------------------
 * By: Christina Fayad & Mohammed Mahmoud 
 * Date: 13 MAY 2019
 *
 * Short Description
 * -----------------
 *
 * This Project is designed to represent the Interface for the Buttons distribution and Vibration Motors for a Special New way for Blind people to communicate with the outside world vice versa.
 * The Distribution includes English Alphabet, Number (0-9), Symbols such as "?", "!", ".", "+", "*", "#", "space", "Yes", "No", "Accept", "Reject", "Send"
 * It makes use of the LT24Display, WM8731 for Audio, SD Card, 7-Segment, External Tactile Push Buttons, DC Vibration Motors, GPIO, & DDR ROM.
 * 
 */

#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include "HPS_usleep/HPS_usleep.h"
#include "LCD_GRAPHICS/LCD_GRAPHICS.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "FatFS/ff.h"
#include "DE1SoC_WM8731/DE1SoC_WM8731.h"
#include"Audio_files/Audio_files.h"

void exitOnFail(signed int status, signed int successStatus){
    if (status != successStatus) {
        exit((int)status);
    }
}

FATFS FatFs;	// Creating space for the fatfs library on the sd card
FIL Fil;         // The object file that will be used in the functions
FRESULT res,res1;  // Return flag of the fatfs library functions

int main(void) 
{
    /*
     *  Declare pointers to I/O registers (volatile keyword means memory not cached)
     */
    // KEY btn base address
    volatile int *KEY   = (int *) 0xFF200050;
	// SW  base address
	volatile int *SW = (int *) 0xFF200040;
	// 7-Segment base address
	volatile int *seg_0 = (int *) 0xFF200020;
	// GPIO1 declared as OUTPUT
	volatile long *JP2PORT_DATA =( long *) 0xFF200070;
	volatile long *JP2PORT_DIR =( long *) 0xFF200074;
	*(JP2PORT_DIR) = 0x000f;
	*(JP2PORT_DATA) = 0x0000;
    volatile unsigned char* fifospace_ptr;
    volatile unsigned int*  audio_left_ptr;
    volatile unsigned int*  audio_right_ptr;
    int j=0;
      int  start_flag=0;
      signed int audio_sample = 0;
      //Initialise the Audio Codec.
          exitOnFail(
                  WM8731_initialise(0xFF203040),  //Initialise Audio Codec
                  WM8731_SUCCESS);                //Exit if not successful
          //Clear both FIFOs
          WM8731_clearFIFO(true,true);
          //Grab the FIFO Space and Audio Channel Pointers
          fifospace_ptr = WM8731_getFIFOSpacePtr();
          audio_left_ptr = WM8731_getLeftFIFOPtr();
          audio_right_ptr = WM8731_getRightFIFOPtr();
	//SD_DDR declaration
	char Buff[15] = {0};  // char buffer to store the read words from sd card
	char Buff1[1] = {0}; // char buffer to store one char from sd card
	unsigned int  bw;
	unsigned int  cnt = sizeof(Buff);
	unsigned int  cnt2 = sizeof(Buff1);
	unsigned int  cnt1 = 0;
	unsigned int  cnt1_2 = 0;
	unsigned short i = 0, x_off = 8, y_off = 100, x_off1 = 30, y_off1 = 150;
		 exitOnFail(
				 Graphics_initialise(0xFF200060,0xFF200080),
				 Graphics_SUCCESS);

		    HPS_ResetWatchdog();

		    exitOnFail(
		    		LT24_clearDisplay(0),
					Graphics_SUCCESS);
		    ResetWDT();
    /*
     *  Primary Run Loop.
     */
     while(1) 
	 { 
        // Check which switch has been turned
		 if ((*SW & 0x01) && !(*SW & 0x02))
		 {
			if (*KEY & 0x01)  // if Key_0 has been pressed
			{
				j=0;  // audio loop and array index
				start_flag=1;  // a flag is set for starting the audio track playing
				f_mount(&FatFs, "", 0); //registering the file system object in sd card
				res = f_open(&Fil, "A.txt", FA_OPEN_EXISTING | FA_READ );   // open the text file of char A to read it
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);  // read the bytes of file a.text and store it in the buff1 array
				if (res == FR_OK && res1 == FR_OK) // check if the open and read operations are success
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);  // display the first character from the read buffer on the LCD
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0001;
				if(start_flag) 
				{
				    while(j<32808) 
					{  // if j is less than the max number of the audio track array
				    		if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
							{ //check if there is space in the data registers
				    			audio_sample=A[j];  //assign each sample from the array to this variable
				    			//A is an array that contain the converted wav file audio samples
				    			//it is defined in the folder Audio_files ,and declared from Audio_files.h
				    			*audio_left_ptr = audio_sample; // output each sample to both of channels registers output
				    			*audio_right_ptr = audio_sample;
				    				j++;  // increment to extract the next sample from the audio array
				    				if(j>=32808) start_flag=0; // once all samples are played , reset the flag
				    		}
					}
				}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "B.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0002;
				if(start_flag) 
				{
					while(j<30552) 
					{
						if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
								    audio_sample=B[j];
								   *audio_left_ptr = audio_sample;
								    *audio_right_ptr = audio_sample;
								    j++;
								   if(j>=30552) start_flag=0;
								   }
						}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "C.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0004;
				if(start_flag) 
				{
					while(j<30504) 
					{
						if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
								    audio_sample=C[j];
								   *audio_left_ptr = audio_sample;
								    *audio_right_ptr = audio_sample;
								    j++;
								   if(j>=30504) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				 j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "D.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0008;
				if(start_flag) 
				{
					while(j<33372) 
						{
								if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
									{
										    audio_sample=D[j];
										   *audio_left_ptr = audio_sample;
										    *audio_right_ptr = audio_sample;
										    j++;
										   if(j>=33372)
										   start_flag=0;
									}
						}
		 		}
			}
		 }

		 else if ((*SW & 0x02) && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x08) && !(*SW & 010) && !(*SW & 0x20) && !(*SW & 0x40) && !(*SW & 0x80)  && !(*SW & 0x100))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{     j=0;
				 start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "E.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0000;
				if(start_flag) 
				{
					while(j<33240) 
					{
						if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
								    audio_sample=E[j];
								   *audio_left_ptr = audio_sample;
								    *audio_right_ptr = audio_sample;
								    j++;
								   if(j>=33240) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{    j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "F.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0003;
				if(start_flag) 
				{
					while(j<27444) 
					{
						if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
								    audio_sample=F[j];
								   *audio_left_ptr = audio_sample;
								    *audio_right_ptr = audio_sample;
								    j++;
								   if(j>=27444) start_flag=0;

						}
					}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{   j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "G.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x0006;
				if(start_flag) 
				{
						while(j<32640) 
							{
								if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
									{
										    audio_sample=G[j];
										   *audio_left_ptr = audio_sample;
											*audio_right_ptr = audio_sample;
										    j++;
										   if(j>=32640) start_flag=0;
									}
							}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "H.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				*(JP2PORT_DATA) = 0x000C;
				if(start_flag) 
				{
				  while(j<37284) 
				  {
			       if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
						 audio_sample=H[j];
						*audio_left_ptr = audio_sample;
						 *audio_right_ptr = audio_sample;
						  j++;
						  if(j>=37284) start_flag=0;
						}
					}
				}
			}
		 }
		 else if ((*SW & 0x04) && !(*SW & 0x08) && !(*SW & 0x01) && !(*SW & 0x04))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
					j=0;
					start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "I.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
						 while(j<34356) 
							  {
							       if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
								   {
										 audio_sample=I[j];
										*audio_left_ptr = audio_sample;
										 *audio_right_ptr = audio_sample;
										  j++;
										  if(j>=34356) start_flag=0;
									}
								}
				}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "J.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
				  while(j<34488) 
					   {
					       if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						   {
								 audio_sample=J[j];
								*audio_left_ptr = audio_sample;
								 *audio_right_ptr = audio_sample;
								  j++;
								  if(j>=34488) start_flag=0;
							}
						}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "K.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
						while(j<35724) 
						   {
						      if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
								   {
										 audio_sample=K[j];
										*audio_left_ptr = audio_sample;
										 *audio_right_ptr = audio_sample;
										  j++;
										  if(j>=35724) start_flag=0;
									}
							}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "L.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					while(j<32352) 
							{
								 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
								 {
											 audio_sample=L[j];
											*audio_left_ptr = audio_sample;
											 *audio_right_ptr = audio_sample;
											  j++;
											  if(j>=32352) start_flag=0;
									}
							}
				}
		  }
		}
		 else if ((*SW & 0x08)  && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x02) && !(*SW & 010) && !(*SW & 0x20) && !(*SW & 0x40) && !(*SW & 0x80)  && !(*SW & 0x100))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
					j=0;
					start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "M.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<34404) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
							{
						 audio_sample=M[j];
						*audio_left_ptr = audio_sample;
						*audio_right_ptr = audio_sample;
							 j++;
							if(j>=34404) start_flag=0;
							}
					}
				}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "N.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
						 while(j<33180) 
									 {
									    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
										{
										 audio_sample=N[j];
										*audio_left_ptr = audio_sample;
										*audio_right_ptr = audio_sample;
											 j++;
											if(j>=33180) start_flag=0;
										}
									}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "O.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<32844) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
						 audio_sample=O[j];
						*audio_left_ptr = audio_sample;
						*audio_right_ptr = audio_sample;
							 j++;
							if(j>=32844) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "P.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<30120) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
						 audio_sample=P[j];
						*audio_left_ptr = audio_sample;
						*audio_right_ptr = audio_sample;
						 j++;
						if(j>=30120) start_flag=0;
						}
					}
				}
			}
		 }
		 else if ((*SW & 0x10)  && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x08) && !(*SW & 02) && !(*SW & 0x20) && !(*SW & 0x40) && !(*SW & 0x80)  && !(*SW & 0x100))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
					j=0;
					start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "Q.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<31356) 
					  {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=Q[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=31356) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "R.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<32424) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=R[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=32424) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "S.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<29292) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=S[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=29292) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "T.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<30420) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=T[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=30420) start_flag=0;
						}
					}
				}
			}
		 }
		 else if ((*SW & 0x20) && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x08) && !(*SW & 010) && !(*SW & 0x02) && !(*SW & 0x40) && !(*SW & 0x80)  && !(*SW & 0x100))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
					j=0;
					start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "U.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
			}
			if(start_flag) 
			{
				 while(j<34632) 
				 {
				    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
					{
						audio_sample=U[j];
						*audio_left_ptr = audio_sample;
						*audio_right_ptr = audio_sample;
						 j++;
						if(j>=34632) start_flag=0;
					}
				}
			}

			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "V.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<37044) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=V[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=37044) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "W.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<41592) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=W[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=41592) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "X.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<26364) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=X[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=26364) start_flag=0;
						}
					}
				}
			}
		 }
		 else if ((*SW & 0x40)  && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x08) && !(*SW & 010) && !(*SW & 0x20) && !(*SW & 0x02) && !(*SW & 0x80)  && !(*SW & 0x100))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
					j=0;
					start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "Y.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<37464) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=Y[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=37464) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
				start_flag=1;
				f_mount(&FatFs, "", 0);
				res = f_open(&Fil, "Z.txt", FA_OPEN_EXISTING | FA_READ );
				res1 = f_read(&Fil, Buff1, cnt2, &cnt1_2);
				if (res == FR_OK && res1 == FR_OK)
				{
					Graphics_drawChar(Buff1[0],x_off1,y_off1);
					x_off1 = x_off1 + 7;
				}
				if(start_flag) 
				{
					 while(j<35232) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=Z[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=35232) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				seg_hex(0, 0, seg_0);
				if(start_flag) 
				{
					 while(j<49224) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=zero[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=49224) start_flag=0;
						}
					}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				seg_hex(1, 0, seg_0);
				if(start_flag) 
				{
					 while(j<43764) 
					 {
					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
						{
							audio_sample=one[j];
							*audio_left_ptr = audio_sample;
							*audio_right_ptr = audio_sample;
							 j++;
							if(j>=43764) start_flag=0;
						}
					}
				}
			}
		 }

		 else if ((*SW & 0x80) && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x08) && !(*SW & 010) && !(*SW & 0x20) && !(*SW & 0x40) && !(*SW & 0x02)  && !(*SW & 0x100))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
				 j=0;
				 start_flag=1;
				 seg_hex(2, 0, seg_0);
				 if(start_flag) 
				 {
				 			while(j<38520) 
								{
				 					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
										{
				 						 audio_sample=two[j];
				 						*audio_left_ptr = audio_sample;
				 						*audio_right_ptr = audio_sample;
										j++;
										if(j>=38520) start_flag=0;
										}
								}
				 }
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
			    start_flag=1;
				seg_hex(3, 0, seg_0);
				if(start_flag) 
				{
						while(j<40320) 
							{
		   					    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
								{
					 				 audio_sample=three[j];
								 	*audio_left_ptr = audio_sample;
									*audio_right_ptr = audio_sample;
				 					j++;
								    if(j>=40320) start_flag=0;
								}
							}
				}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				seg_hex(4, 0, seg_0);
				if(start_flag) 
				{
						while(j<38664) 
							{
								 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
										{
								 	   			 audio_sample=four[j];
								 				*audio_left_ptr = audio_sample;
								 				*audio_right_ptr = audio_sample;
								 				j++;
								 				if(j>=38664) start_flag=0;
										}
							}
				}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				seg_hex(5, 0, seg_0);
				if(start_flag) 
						{
							while(j<21053)	
								  {
									    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
												{
								 					 audio_sample=five[j];
								 					*audio_left_ptr = audio_sample;
								 					*audio_right_ptr = audio_sample;
								 					j++;
								 					if(j>=21053) start_flag=0;
								 				}
								}
						}
		 }
		 }
		 else if ((*SW & 0x100) && !(*SW & 0x01) && !(*SW & 0x04) && !(*SW & 0x08) && !(*SW & 010) && !(*SW & 0x20) && !(*SW & 0x40) && !(*SW & 0x80)  && !(*SW & 0x02))
		 {
			 if (*KEY & 0x01)  // if Key_0 has been pressed
			{
				 j=0;
				 start_flag=1;
				seg_hex(6, 0, seg_0);
				if(start_flag) 
					{
							while(j<33132) 
									{
									    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
											{
								 				 audio_sample=six[j];
								 				*audio_left_ptr = audio_sample;
								 				*audio_right_ptr = audio_sample;
								 				 j++;
												 if(j>=33132) start_flag=0;
								 			}
								 	}
					}
			}
			else if (*KEY & 0x02)  // if Key_1 has been pressed
			{
				j=0;
			    start_flag=1;
				seg_hex(7, 0, seg_0);
				if(start_flag) 
					{
							while(j<38040) 
								{
									    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
											{
								 				 audio_sample=seven[j];
								     			*audio_left_ptr = audio_sample;
												*audio_right_ptr = audio_sample;
								 				 j++;
								 				if(j>=38040) start_flag=0;
								 			}
								 }
					}
			}
			else if (*KEY & 0x04)  // if Key_2 has been pressed
			{
				j=0;
				start_flag=1;
				seg_hex(8, 0, seg_0);
				if(start_flag) 
					{
						 while(j<31188) 
							 {
								    if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
										{
												audio_sample=eight[j];
								 	     		*audio_left_ptr = audio_sample;
												*audio_right_ptr = audio_sample;
								 				 j++;
								 				if(j>=31188) start_flag=0;
								 		}
							}
					}
			}
			else if (*KEY & 0x08)  // if Key_3 has been pressed
			{
				j=0;
				start_flag=1;
				seg_hex(9, 0, seg_0);
				if(start_flag) 
					{
							while(j<44748) 
								{
					     			 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) 
											{
				 		 						 audio_sample=nine[j];
						 						*audio_left_ptr = audio_sample;
												*audio_right_ptr = audio_sample;
												 j++;
												if(j>=44748) start_flag=0;
											}
								}
					}
			}
		 }
		 else if ((*SW & 0x01) && (*SW & 0x02))
		 		 {
			        x_off1 = 30; y_off1 = 200;
		 			 j=0;
		 			LT24_clearDisplay(0);
		 			 start_flag=1;
		 			 f_mount(&FatFs, "", 0);
		 			 res = f_open(&Fil, "hi.txt", FA_OPEN_EXISTING | FA_READ );
		 			 res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 			 if (res == FR_OK && res1 == FR_OK)
		 			 		{
 		        		for(i=0;i<=sizeof(Buff);i++){  //this loop extract each character from the read buffer
 		      // send it to the lcd display function , and then increment the x coordinates to display the next one
	        		        		    Graphics_drawChar(Buff[i],x_off,y_off);
	        		        		    x_off=x_off+7;
	        		        		   if (x_off>238) {
	        		        		    	 x_off=0;
	        		        		    	y_off=y_off+7; }
	        		        		}
		 			 		}
		 			 		if(start_flag) {
		 			 				while(j<48064) {
		 			 				 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 			 				 audio_sample=hi[j];
		 			 				*audio_left_ptr = audio_sample;
		 			 				*audio_right_ptr = audio_sample;
		 			 				j++;
		 			 				if(j>=48064) start_flag=0;
		 			 								}
		 			 						}
		 			 				}
		 		 }
		 		 else if ((*SW & 0x02) && (*SW & 0x04))
		 		 		 {
		 		 			 j=0;
		 		 			LT24_clearDisplay(0);
		 		 			 start_flag=1;
		 		 			 f_mount(&FatFs, "", 0);
		 		 			 res = f_open(&Fil, "this.txt", FA_OPEN_EXISTING | FA_READ );
		 		 			res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 		 			if (res == FR_OK && res1 == FR_OK)
		 		 					 	{
		 		 			 		       for(i=0;i<=sizeof(Buff);i++){
		 		 				        		      Graphics_drawChar(Buff[i],x_off,y_off);
		 		 				        		       x_off=x_off+7;
		 		 				        		       if (x_off>238) {
		 		 				        		       	 x_off=0;
		 		 				        		        	y_off=y_off+7; }
		 		 				        		           }
		 		 					 			 		}
		 		 			 		if(start_flag) {
		 		 			 				while(j<58304) {
		 		 			 				 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 		 			 				 audio_sample=thi[j];
		 		 			 				*audio_left_ptr = audio_sample;
		 		 			 				*audio_right_ptr = audio_sample;
		 		 			 				j++;
		 		 			 				if(j>=58304) start_flag=0;
		 		 			 								}
		 		 			 						}
		 		 			 				}
		 		 		 }
		 		 else if ((*SW & 0x04) && (*SW & 0x08))
		 		 		 		 {
		 		 		 			 j=0;
		 		 		 			LT24_clearDisplay(0);
		 		 		 			 start_flag=1;
		 		 		 			 f_mount(&FatFs, "", 0);
		 		 		 			 res = f_open(&Fil, "is.txt", FA_OPEN_EXISTING | FA_READ );
		 		 		 			res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 		 		 			if (res == FR_OK && res1 == FR_OK)
		 		 		 					 	{
		 		 		 			        		for(i=0;i<=sizeof(Buff);i++){
		 		 	        		        		    Graphics_drawChar(Buff[i],x_off,y_off);
			 		        		        		    x_off=x_off+7;
		 		 		 	     		        		   if (x_off>238) {
		 		 		 	    		        		    	 x_off=0;
		 		 		 	     		        		    	y_off=y_off+7; }
		 		 		 				        		        		}
		 		 		 					 			 		}
		 		 		 			 		if(start_flag) {
		 		 		 			 				while(j<58304) {
		 		 		 			 				 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 		 		 			 				 audio_sample=is[j];
		 		 		 			 				*audio_left_ptr = audio_sample;
		 		 		 			 				*audio_right_ptr = audio_sample;
		 		 		 			 				j++;
		 		 		 			 				if(j>=58304) start_flag=0;
		 		 		 			 								}
		 		 		 			 						}
		 		 		 			 				}
		 		 		 		 }
		 		 else if ((*SW & 0x08) && (*SW & 0x10))
		 		 		 		 		 {
		 		 		 		 			 j=0;
		 		 		 		 			LT24_clearDisplay(0);
		 		 		 		 			 start_flag=1;
		 		 		 		 			 f_mount(&FatFs, "", 0);
		 		 		 		 			 res = f_open(&Fil, "our.txt", FA_OPEN_EXISTING | FA_READ );
		 		 		 		 			res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 		 		 		 			if (res == FR_OK && res1 == FR_OK)
		 		 		 		 						{
		 		 		 		 			 		      	for(i=0;i<=sizeof(Buff);i++){
		 		 		 		 				        	    Graphics_drawChar(Buff[i],x_off,y_off);
		 		 		 		 				        	    x_off=x_off+7;
		 		 		 		 				        		  if (x_off>238) {
		 		 		 		 				        		  	 x_off=0;
		 		 		 		 				        		     y_off=y_off+7; }
		 		 		 		 				        		        }
		 		 		 		 					 			 }
		 		 		 		 			 		if(start_flag) {
		 		 		 		 			 				while(j<53184) {
		 		 		 		 			 				 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 		 		 		 			 				 audio_sample=our[j];
		 		 		 		 			 				*audio_left_ptr = audio_sample;
		 		 		 		 			 				*audio_right_ptr = audio_sample;
		 		 		 		 			 				j++;
		 		 		 		 			 				if(j>=53184) start_flag=0;
		 		 		 		 			 								}
		 		 		 		 			 						}
		 		 		 		 			 				}
		 		 		 		 		 }
		 		 else if ((*SW & 0x10) && (*SW & 0x20))
		 				 		 		 		 {
		 			x_off1 = 30; y_off1 = 250;
		 				 		 		 			 j=0;
		 				 		 		 			LT24_clearDisplay(0);
		 				 		 		 			 start_flag=1;
		 				 		 		 			 f_mount(&FatFs, "", 0);
		 				 		 		 			 res = f_open(&Fil, "Embedded.txt", FA_OPEN_EXISTING | FA_READ );
		 				 		 		 			res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 				 		 		 			if (res == FR_OK && res1 == FR_OK)
		 				 		 		 						{
		 				 		 		 			 			for(i=0;i<=sizeof(Buff);i++){
		 				 		 		 				        		  Graphics_drawChar(Buff[i],x_off,y_off);
		 				 		 		 				        		  x_off=x_off+7;
		 				 		 		 				        		    if (x_off>238) {
		 				 		 		 				        		       x_off=0;
		 				 		 		 				        		       	y_off=y_off+7; }
		 				 		 		 				        		        		}
		 				 		 		 					 			 		}
		 				 		 		 			 		if(start_flag) {
		 				 		 		 			 				while(j<58304) {
		 				 		 		 			 				 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 				 		 		 			 				 audio_sample=embedded[j];
		 				 		 		 			 				*audio_left_ptr = audio_sample;
		 				 		 		 			 				*audio_right_ptr = audio_sample;
		 				 		 		 			 				j++;
		 				 		 		 			 				if(j>=58304) start_flag=0;
		 				 		 		 			 								}
		 				 		 		 			 						}
		 				 		 		 			 				}
		 				 		 		 		 }
		 		 else if ((*SW & 0x20) && (*SW & 0x40))
		 		 				 		 		 		 {
		 		 				 		 		 			 j=0;
		 		 				 		 		 		LT24_clearDisplay(0);
		 		 				 		 		 			 start_flag=1;
		 		 				 		 		 			 f_mount(&FatFs, "", 0);
		 		 				 		 		 			 res = f_open(&Fil, "mini.txt", FA_OPEN_EXISTING | FA_READ );
		 		 				 		 		 		res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 		 				 		 		 		if (res == FR_OK && res1 == FR_OK)
		 		 				 		 		 				{
		 		 				 		 		 		 		   	for(i=0;i<=sizeof(Buff);i++){
		 		 				 		 		 			        	Graphics_drawChar(Buff[i],x_off,y_off);
		 		 				 		 		 			        	x_off=x_off+7;
		 		 				 		 		 			          if (x_off>238) {
		 		 				 		 		 			           	 x_off=0;
		 		 				 		 		 			        	  y_off=y_off+7; }
		 		 				 		 		 			        		      	}
		 		 				 		 		 				 		}
		 		 				 		 		 			 		if(start_flag) {
		 		 				 		 		 			 				while(j<53184) {
		 		 				 		 		 			 				 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 		 				 		 		 			 				 audio_sample=mini[j];
		 		 				 		 		 			 				*audio_left_ptr = audio_sample;
		 		 				 		 		 			 				*audio_right_ptr = audio_sample;
		 		 				 		 		 			 				j++;
		 		 				 		 		 			 				if(j>=53184) start_flag=0;
		 		 				 		 		 			 								}
		 		 				 		 		 			 						}
		 		 				 		 		 			 				}
		 		 				 		 		 		 }
		 		 else if ((*SW & 0x40) && (*SW & 0x80))
		 		 			 {
		 		 				 		 j=0;
		 		 				 		LT24_clearDisplay(0);
		 		 				 		  start_flag=1;
		 		 				 		  f_mount(&FatFs, "", 0);
		 		 				 		  res = f_open(&Fil, "project.txt", FA_OPEN_EXISTING | FA_READ );
		 		 				 		res1 = f_read(&Fil, Buff, cnt, &cnt1);
		 		 				 		if (res == FR_OK && res1 == FR_OK)
		 		 				 					{
		 		 				 		 		       for(i=0;i<=sizeof(Buff);i++){
		 		 				 			        		  Graphics_drawChar(Buff[i],x_off,y_off);
		 		 				 			        		   x_off=x_off+7;
		 		 				 			        		   if (x_off>238) {
		 		 				 			        		        x_off=0;
		 		 				 			        		        y_off=y_off+7; }
		 		 				 			        		        	}
		 		 				 				 			 		}
		 		 				 		 		 if(start_flag) {
		 		 				 		 		 	 while(j<58304) {
		 		 				 		 		 	 if ((fifospace_ptr[2] > 0) && (fifospace_ptr[3] > 0)) {
		 		 				 		 		 		 audio_sample=project[j];
		 		 				 		 		 			*audio_left_ptr = audio_sample;
		 		 				 		 		 			*audio_right_ptr = audio_sample;
		 		 				 		 		 			j++;
		 		 				 		 		 			if(j>=58304) start_flag=0;
		 		 				 		 		 			 			}
		 		 				 		 		 					}
		 		 				 		 		 			}
		 		 				 		 }
		 else if(!(*SW & 0x200))
		 {
			 j=0;
			 start_flag=0;
			res = 0;
			res1 = 0;
			//cnt1 = 0;
			//cnt = 15;
			//cnt2 = 0;
			//cnt1_2 = 0;
			LT24_clearDisplay(0);
			x_off1 = 30; y_off1 = 150;
			 for(i=0;i<=sizeof(Buff);i++){
							 Buff[i]=0;
						 }

	     }
		 else 
		 {
			 *(JP2PORT_DATA) = 0x0000;
			 for(i=0;i<=sizeof(Buff);i++){
				 Buff[i]=0;
			 }


		 }
		 HPS_ResetWatchdog();  //Watchdog reset.
    }
}

