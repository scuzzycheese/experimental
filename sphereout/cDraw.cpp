#include "cDraw.h"
#include <math.h>
#include <stdlib.h>

cDraw::cDraw()
{
}

cDraw::cDraw(void *buf, int x, int y) : buffer(buf), width(x), height(y)
{
}

void cDraw::circle(int x, int y, float radius, unsigned int colour, short filled)
{
	float X = (sin(-0.1) * radius) + x, Y = (cos(-0.1) * radius) + y;
	for(float i = 0; i < 6.28318; i += 0.1)
	{
		float newX = (sin(i) * radius) + x;
		float newY = (cos(i) * radius) + y;
		line(X, Y, newX, newY, colour);
		X = newX;
		Y = newY;
	}
}

void cDraw::floodFill(int x, int y, unsigned int newColor, unsigned int oldColor)
{
	if(oldColor == newColor) return;
	if(getPixel(x, y) != oldColor) return;
      
	int y1;
    
	//cDraw current scanline from start position to the top
	y1 = y;
	while(y1 < height && getPixel(x, y1) == oldColor)
	{
		pixel(x, y1, newColor);
		y1++;
	}    
    
	//cDraw current scanline from start position to te bottom
	y1 = y - 1;
	while(y1 >= 0 && getPixel(x, y1) == oldColor)
	{
		pixel(x, y1, newColor);
		y1--;
	}

	//test for new scanlines to the left
	y1 = y;
	while(y1 < height && getPixel(x, y1) == newColor)
	{
		if(x > 0 && getPixel(x - 1, y1) == oldColor) 
		{
			floodFill(x - 1, y1, newColor, oldColor);
		} 
		y1++;
	}
	y1 = y - 1;
	while(y1 >= 0 && getPixel(x, y1) == newColor)
	{
		if(x > 0 && getPixel(x - 1, y1) == oldColor) 
		{
			floodFill(x - 1, y1, newColor, oldColor);
		}
		y1--;
	} 

	//test for new scanlines to the right 
	y1 = y;
	while(y1 < height && getPixel(x, y1) == newColor)
	{
		if(x < width - 1 && getPixel(x + 1, y1) == oldColor) 
		{           
			floodFill(x + 1, y1, newColor, oldColor);
		} 
		y1++;
	}
	y1 = y - 1;
	while(y1 >= 0 && getPixel(x, y1) == newColor)
	{
		if(x < width - 1 && getPixel(x + 1, y1) == oldColor) 
		{
			floodFill(x + 1, y1, newColor, oldColor);
		}
		y1--;
	}

}

void cDraw::square(int startX, int startY, int x, int y, unsigned int colour, short filled)
{
	line(startX, startY, startX + x, startY, colour); 			 // ---
	line(startX, startY, startX, startY + y, colour); 			 // |
	line(startX, startY + y, startX + x, startY + y, colour); // ___
	line(startX + x, startY, startX + x, startY + y, colour); //   |
}

void cDraw::line(int x1, int y1, int x2, int y2, unsigned int colour)
{
	int deltax = abs(x2 - x1);        // The difference between the x's
	int deltay = abs(y2 - y1);        // The difference between the y's
	int x = x1;                       // Start x off at the first pixel
	int y = y1;                       // Start y off at the first pixel
	int xinc1, xinc2, yinc1, yinc2;
	int den;
	int num;
	int numadd;
	int numpixels;

	xinc1 = xinc2 = (x2 > x1) ? 1 : - 1;
	yinc1 = yinc2 = (y2 > y1) ? 1 : - 1;
	
	if(deltax >= deltay)         // There is at least one x-value for every y-value
	{
		xinc1 = 0;                  // Don't change the x when numerator >= denominator
		yinc2 = 0;                  // Don't change the y for every iteration
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         // There are more x-values than y-values
	}
	else                          // There is at least one y-value for every x-value
	{
		xinc2 = 0;                  // Don't change the x for every iteration
		yinc1 = 0;                  // Don't change the y when numerator >= denominator
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         // There are more y-values than x-values
	}
	
	for(int curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		pixel(x, y, colour);             // Draw the current pixel
		num += numadd;              // Increase the numerator by the top of the fraction
		if(num >= den)             // Check if numerator >= denominator
		{
			num -= den;               // Calculate the new numerator value
			x += xinc1;               // Change the x as appropriate
			y += yinc1;               // Change the y as appropriate
		}
		x += xinc2;                 // Change the x as appropriate
		y += yinc2;                 // Change the y as appropriate
	}
}
