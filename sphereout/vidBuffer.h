#ifndef VIDBUFFER_H
#define VIDBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

template <int T>
class vidBuffer
{
	private:
		void *data[T];
		int width, height;
		int size;

	public:
		vidBuffer()
		{
		}
		vidBuffer(int x, int y) : width(x), height(y)
		{
			size = x * y * 4;
			for(int i = 0; i < T; i ++)
			{
				data[i] = (void *)malloc(size);
				memset(data[i], 0x00, size);	
			}
		}

		~vidBuffer()
		{
			for(int i = 0; i < T; i ++)
			{
				free(data[i]);
			}
		}
	
		void copyBuffer(int toBuffer, int fromBuffer)
		{
			memcpy(data[toBuffer], data[fromBuffer], size);
		}

		void *getBuffer(int number)
		{
			return data[number];
		}

		int getWidth()
		{
			return width;
		}

		int getHeight()
		{
			return height;
		}
};

#endif
