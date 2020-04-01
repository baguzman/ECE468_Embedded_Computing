

#include <stdio.h>
#include <stdlib.h>
#include <string.h

#define MAX_SIZE 65536

void createDictionary(unsigned char *dict[]);

int main()
{
	unsigned char *dict;
	createDictionary(dict);
	return(0);
}

void createDictionary(unsigned char *dict[])
{
	unsigned char c;
	int i;
	
	for(c = 0; c < 256; c++)
	{
		dict[c] = (unsigned char *)calloc(1,2*sizeof(unsigned char));
		dict[c][0] = c;
		printf("dict[%s] : %d\n",c, i);
	}
	for(i = 0; i < MAX_SIZE; i++)
	{
		dict[i] = NULL;
	}
}