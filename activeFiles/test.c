#include <stdio.h>

void af_read();

int main()
{
	void (*myAfRead)() = af_read;
	FILE *fd = fopen("testWasExecuted", "w+");
	fclose(fd);
	/*
	while(1)
	{
		sleep(1);
		printf("af_read address: %p\n", myAfRead);
	}
	*/
}

void af_read()
{
	printf("af_read called\n");
}
