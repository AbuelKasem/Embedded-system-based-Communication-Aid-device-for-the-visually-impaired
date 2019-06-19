/*
 * LCD_GRAPHICS.c



 *
 *  Created on: Mar 17, 2019
 *      Author: Kasem
 */
#include <stdbool.h>
#include <stdlib.h>
#include "LCD_GRAPHICS.h"
#include <math.h>
#include "../DE1SoC_LT24/DE1SoC_LT24.h"
//#include "../HPS_Watchdog/HPS_Watchdog.h"
//#include "../HPS_usleep/HPS_usleep.h"


#define  T   0    // animatie time


void  Graphics_drawChar(char ascii_char,unsigned short x_off,unsigned short y_off){

	unsigned short x_y_pos[2] = { X_OFF, Y_OFF};  //x and y position of the character

	unsigned short x_y_initial[] = {0,0}; // initial values to zero


	unsigned short  width = 240;   // dimensions of the lcd
	unsigned short height = 320;

		unsigned short x_of=20,y_of=50;  // default coordinates variables
		unsigned short x=x_of,y=y_of;  // overriden by the two input arguments of the function
		char char_byte;
		unsigned short char_address;
		x_y_pos[0]=x_y_pos[0]+x_off;  // adding the offsets to the initial values
		x_y_pos[1]=x_y_pos[1]+y_off;
		if ((ascii_char >= ' ') && (ascii_char <= '~'))
		{
              // calculating the index number of the required char in the ascii char array
			//by subtracting the ascii character's ascii code in hex from the hex code of the first element in the array(space)
			char_address = ascii_char - ' ';
			char_address=5*char_address; // then offseting by the five bytes of each characters



	    for ( x = x_y_pos[0]; x < x_y_pos[0]+5; x++) //looping in the 5 bytes of the char
	    {
	    	char_byte = charactersArray[char_address++];  // extracting each byte values
	      for ( y = x_y_pos[1]; y<x_y_pos[1]+8; y++) // looping through the 8 bits of each byte
	      {
	        if ((char_byte>>(y-x_y_pos[1]))&0x01) LT24_drawPixel(LT24_BLUE,x,y);  // drawing a pixel if the bit[x][y]==1 else leave blank

	      }
	    }
	    x_y_pos[0] += 6; // incrementing the y offset after each char

	    if (x_y_pos[0] >= (width-6))  // checking x and y against the boundaries of the lcd
	    {
	    	x_y_pos[0] = x_y_initial[0];
	    	x_y_pos[1] += 8;
	      if (x_y_pos[1] >= (height-7)) x_y_pos[1] = x_y_initial[1];
	    }
		}
		}

signed int Graphics_initialise( unsigned int lcd_pio_base, unsigned int lcd_hw_base ){
	signed int init_status_code=0;  // init. the status code to 0
	if(!(LT24_isInitialised())) {   // check if the lt24 driver is init.
		init_status_code=-1;        // if not set code to -1
		init_status_code=LT24_initialise(lcd_pio_base,lcd_hw_base);  // call the lt24 init. function and assign the returned status
		//code to init_status_code

	}

	return init_status_code;  // return the value of the status code to the calling function


}

signed int Graphics_drawLine(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short colour)
{
		signed int graphics_status=0;
	    int dy=0;  //difference in y axis
		int dx=0;   //difference in x axis
		float s=0; //slope of the line
	    float err=0;                     // the distance between the top of the pixel and the point where the line exited the pixel
		int x_step=0,y_step=0;   // the increment/decrement step
		int i=0,j=0,x=0,y=0,n=0,m=0,step1=0,step2=0;               // iterators i for x ,j for y

		if(x1 > LT24_WIDTH_limit || x2 > LT24_WIDTH_limit || y1 > LT24_HEIGHT_limit || y2 > LT24_HEIGHT_limit )
				{ graphics_status=Graphics_INVALIDSIZE; }   // if any of the points is beyond the limit of the lcd , return error of invalid size

				if (graphics_status != Graphics_SUCCESS ) return graphics_status;
		dy=y2-y1;      // delta y
		dx=x2-x1;      // delta x
		if(y2>y1){    // to decide the direction of movement
		y_step=1;  // if end larger than start , step is positive ,to increment by a single pixel
		}
		else if(y2<y1){
			y_step=-1;  // if end less than start , step is negative ,to decrement by a single pixel
		}
		if(x2>x1) {  // same as y ,to determine the step

			x_step=1;
		}
		else if(x2<x1){
		x_step=-1;
		}
                                         //y=s*x+c   the straight line equation                                   //y1=s*x1+c  , c=y1-s*x1  , calculating the intercept
		if(x1!=x2) s=(float)dy/(float)dx;  // to prevent division by zero,only calculate slope when x1 is different from x2
		if(s<0) s=s*(-1);  //to get the absolute value  of slope
		if(s>1) s=1/s;  // to use the reciporocal of the slope in case of slope greater than 1

		err=s-1; // init. the error parameter

		if(dx==-dy || dy==-dx) dx=dy;  //when the deltas are equal in value but different in sign ,adjust
		 if(dx <0 )  dx=dx*-1;     // getting the absolute values of the deltas ,because the step is already determined and we
		                          // only need 4 conditions ,disregarding the sign of the slope
		 if(dy<0)     dy=dy*-1;


        if(dx>=dy || dy==0) {    //when the slope is greater or equal to 1 or the line is horizontal
			n=x1;   // make x the driving axis
			m=x2;
			step1=x_step;  //the loop increment
			step2=y_step;  // the driven axis's increment
			j=y1;       // a temp. variable to either increment y o r x based on the case
		}
		else if(dx==0 || dx < dy  ) {   // this is if the slope is greater than 1 (either in negative or positive)
			n=y1;   // making y the driving axis
			m=y2;
			step1=y_step;  //loop increment
			step2=x_step;  // the driven axis's increment (x-axis)
			j=x1;

		}


		for(i=n;(i!=m);i=i+step1){   //the line drawing loop  ,incrementing or decrmenting the driving axis based on the
			                         // cases evaluated above

			if(y1==y2) {  // if the line is vertical ,then fix y and incrment x
				y=y2;
				x=i;
			}

			else if(x1==x2) {   // if the line is horizontal ,then fix x and incrment y
				x=x2;
				y=i;
			           }
			else if(dx>=dy){ //if the slope is < 1 make x the driving axis (the loop iterator )
				x=i;
				y=j;   // y is the driven axis
			}
			else if(dx < dy){   //slope greater than 1 , swap x and y , make y driving axis ,and x driven
							y=i;
							x=j;
						}

			//usleep(T);

			graphics_status= LT24_drawPixel(colour,x,y);   //call the pixel drawing function with the calcualted x and y position
			                                               // if there is error ,exit
			if (graphics_status != Graphics_SUCCESS ) return graphics_status;

		 usleep(T);  // to add animation effect (T) is delay , comment out if unneeded

			 if(err>=0) {   // when the error equals or above zero ,increment the driven axis
				 j=j+step2;

			    err=err-1; // adjust the error to the new pixel
			            }
			 err=err+s; // incrment the error when the driving axis is incremented


		}
		return Graphics_SUCCESS;  // if all is done , return success
	}


signed int Graphics_drawTriangle(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned int x3,unsigned int y3,unsigned short colour,bool noFill,unsigned short fillColour)
{
	  signed int graphics_status=0;
	  int n,m,xmax,xmin,ymax,ymin;


	graphics_status=Graphics_drawLine(x1,y1,x2,y2, colour);  // draw the first side
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;
	graphics_status=Graphics_drawLine(x2,y2,x3,y3, colour);       // draw the 2nd side
    if (graphics_status != Graphics_SUCCESS ) return graphics_status;
    graphics_status=Graphics_drawLine(x3,y3,x1,y1, colour);          // draw the 3rd side
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;

	if(noFill ) return Graphics_SUCCESS;  // if the filling option is not active , exit with success


	if(!noFill ) {    // if filling option is selected

   xmax = (x1 > x2) ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3);    // get the max of x and y
   ymax = (y1 > y2) ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3);    //
   xmin= (x1 < x2) ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3);  // calcualte the min of y and x in the three points
   ymin = (y1 <y2) ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3);

		for(n=xmin;n<=xmax;n++){  // loop through x axis starting from x min to x max
		         for(m=ymin;m<=ymax;m++){  // for each point in x ,loop through y axis from min to max

		         	if(Point_in_triangle(x1,y1,x2,y2,x3,y3,n,m)) {  // test each point generated if it is in the triangle or not
		         		LT24_drawPixel(fillColour,n,m); // if it is in the triangle  illuminate the pixel at this point
		         	    }
		}

	}


}
	return Graphics_SUCCESS;
}




float area_calc(int x1, int y1, int x2, int y2, int x3, int y3)
{    float area;
   area=abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
   return area;
}
bool Point_in_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{
   float area1,area2,area3,area;

   area = area_calc (x1, y1, x2, y2, x3, y3);  // calculating the area of the main triangle


   area1 =area_calc (x, y, x2, y2, x3, y3); // calculating the 3 areas of the 3 sub triangles created by the test point x,y

   area2 = area_calc (x1, y1, x, y, x3, y3);

   area3 = area_calc (x1, y1, x2, y2, x, y);

   return (area == area1 + area2 + area3);  // if the main area equals the sum of the 3 areas ,then the point is inside the triangle ,return true ,else false
}


signed int Graphics_drawBox(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short colour,bool noFill,unsigned short fillColour)

{
	signed int graphics_status=0;
	// x1,y1  top left corner
	//x2,y2 bottom right corner
	unsigned int x3,y3,x4, y4;
	  x3=x1;  // assigning corner 3
	  y3=y2;
	  x4=x2;  // assigning corner 4
	  y4=y1;

	   graphics_status=Graphics_drawLine(x1,y1,x3,y3, colour);    // calling the draw line function 4 times to draw the four sides of
	   if (graphics_status != Graphics_SUCCESS ) return graphics_status;
                                                                              // the rectangle
	   graphics_status= Graphics_drawLine(x3,y3,x2,y2, colour);
	   if (graphics_status != Graphics_SUCCESS ) return graphics_status;

	   graphics_status=Graphics_drawLine(x2,y2,x4,y4, colour);
	   if (graphics_status != Graphics_SUCCESS ) return graphics_status;

	   graphics_status=Graphics_drawLine(x4,y4,x1,y1, colour);
	   if (graphics_status != Graphics_SUCCESS ) return graphics_status;

	   if(noFill ) return Graphics_SUCCESS;  // if no filling required ,exit with success

	   if(!noFill)  do {

		                                     //while(x1!=x2 && y1 != y2
		 x1++;   // because x1,y1 is the top left corner ,so increasing both moves the point to the the direction of the bottom right corner
		 y1++;
		 x2--;   // x2,y2 are decremented to move in the opposite side of x1,x2
		 y2--;

		 Graphics_drawBox(x1,y1,x2,y2,fillColour,1,fillColour);  // draw the new inner rectangele
		 if(x1==x2 || y1==y2) break;  // once the points become one point , break the loop , the centre of the rectangle is reached
	 } while(x1!=x2);  // as long as x1 still not met with x2



x1=0,x2=0,y1=0,y2=0;
 return Graphics_SUCCESS;

}

signed int Graphics_drawCircle(unsigned int xc,unsigned int yc,unsigned int r,unsigned short colour,bool noFill,unsigned short fillColour){

	signed int graphics_status=0;

	 int p,q;
	 int x,y,d;
	 if(xc > LT24_WIDTH_limit || (xc+r) > LT24_WIDTH_limit || yc > LT24_HEIGHT_limit || (yc+r) > LT24_HEIGHT_limit )
	 		 // if any of centre coordinates is beyond the limit of the lcd  or the centre plus radius is greater than limits
	 		//, return error of invalid size to prevent the program from iterating through non existent pixels
	 			{ graphics_status=Graphics_INVALIDSIZE; }

	 			if (graphics_status != Graphics_SUCCESS ) return graphics_status;
	 p=0;
	 q=r;
	x=xc;
	y=yc;
	d=3-(2*r); // the decision parameter
	graphics_status=eight_octant_draw(colour,x,y,p,q);
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;
	while(p<=q) {  //
		p++;
		if(d<0) {d=d+4*p+6;  // if the decision is lees than 0 , re-calculate it

		}
		else {
			q=q-1;  // decrement q by 1
			d=d+4*(p-q)+10;  // and re-assign the decision
		}
		graphics_status=eight_octant_draw(colour,x,y,p,q); // the values of p and q along with the centre point (x,y) are passed to
		// the 8 octants function ,that will draw eight pixels at on the circle  for each point calculated

		if (graphics_status != Graphics_SUCCESS ) return graphics_status;
	}
    r=r-1;


if(noFill ) return Graphics_SUCCESS;

if(!noFill) while(r>=1) {  //as long as the radius is greater or equal to 1
	r--;   // decrement radius

	// call the circle draw function with the new radius and same centre ,to fill the outer circle
	graphics_status=Graphics_drawCircle(xc,yc,r,fillColour,1,fillColour);

	if (graphics_status != Graphics_SUCCESS ) return graphics_status;

}
return Graphics_SUCCESS;

}


signed int eight_octant_draw(unsigned short colour,int x ,int y,int p,int q){


	signed int graphics_status=0;


	graphics_status= LT24_drawPixel(colour,x+p,y+q);  //1st octant
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x-p,y+q); //2nd
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x+p,y-q);  //3rd
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x-p,y-q);  //4th
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x+q,y+p); //5th
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x-q,y+p);
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x+q,y-p);
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	graphics_status= LT24_drawPixel(colour,x-q,y-p);
	if (graphics_status != Graphics_SUCCESS ) return graphics_status;


	return Graphics_SUCCESS;


}

