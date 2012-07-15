#ifndef DRAW_H
#define DRAW_H

class cDraw
{

	private:
		void *buffer;
		int width, height;

	public:
		cDraw();
		cDraw(void *buf, int x, int y);
		inline void pixel(int x, int y, unsigned int colour)
		{
			if(x > 0 && x < width && y > 0 && y < height) *(((int *)buffer) + ((y * width) + x)) = colour;
		}
		inline unsigned int getPixel(int x, int y)
		{
			return *(((int *)buffer) + ((y * width) + x));
		}
		void floodFill(int x, int y, unsigned int newColor, unsigned int oldColor);
		void line(int x1, int y1, int x2, int y2, unsigned int colour);
		void circle(int x, int y, float radius, unsigned int colour, short filled);
		void square(int startX, int startY, int x, int y, unsigned int colour, short filled);
 

};

#endif
