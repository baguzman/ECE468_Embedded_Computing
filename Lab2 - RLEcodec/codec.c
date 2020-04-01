//Brian Guzman
//ECE 4680
//Lab2 - RLE CODEC
//Spring 2019

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *fin;
	FILE *fout;
	long size;
	int command = -1;
	unsigned char *file_data;
	char *ext;
	char *file_name;
	char *fout_name;
	int i;
	long comp_size = 0;
	long decomp_size = 0;
	
	//MAKE SURE A FILE IS PASSED ALONG WITH A COMMAND
	if(argc != 3)
	{
		printf("Usage: codec <input file> <command>\n Where command = -c to compress or -d to decompress\n");
		exit(0);
	}
	//CHECK THE TYPE OF COMMAND
	if(strcmp(argv[2],"-c") == 0)
	{
		command = 1;
	}
	else if(strcmp(argv[2],"-d") == 0)
	{
		command = 0;
	}
	else
	{
		printf("Error: Could not verify command\n");
		exit(0);
	}
	//OPEN THE FILE TO BE PARSED
	fin = fopen(argv[1],"rb");
	if(fin == NULL)
	{
		printf("ERROR: Could not open file %s.\n",argv[1]);
		exit(0);
	}
	//BUILD THE OUTPUT FILE NAME
	file_name = argv[1];
	fout_name = calloc(1,strlen(file_name)+strlen("_decompressed")+1);
	file_name = argv[1];
	ext = strstr(file_name,".");
	if(ext != NULL)
	{		
		i = strlen(ext);
	}
	else
	{
		i = 0;
	}
	snprintf(fout_name,strlen(file_name)+1-i,"%s",file_name);
	if(command == 1)
	{
		strcat(fout_name,"_compressed");
	}
	else
	{
		strcat(fout_name,"_decompressed");
	}
	strcat(fout_name,ext);	
	//CHECK THE SIZE OF THE INPUT FILE AND STORE IN VARIABLE size
	fseek(fin,0,SEEK_END);
	size = ftell(fin);
	rewind(fin);
	//ALLOCATE SPACE FOR THE INPUT FILE AND READ SAID FILE TO file_data
	file_data = (unsigned char *)calloc(1,size*sizeof(unsigned char));
	fread(file_data, sizeof(unsigned char), size,fin);
	fclose(fin);
	//OPEN OUTPUT FILE
	fout = fopen(fout_name,"wb");
	//PERFORM COMPRESSION OR DECOMPRESSION	
	unsigned char data,count;
	int x;
	//COMPRESSION
	if(command == 1)
	{
		i = 0;
		while(i < size)
		{
			data = file_data[i]; //READ BYTE i AND INCREMENT count BY 1 
			count = 1;
			while(file_data[i+count] == data && i+count < size)//CHECK THE NEXT BYTE TO SEE IF THEY ARE EQUAL AND CHECK THAT IT IS NOT THE LAST BIT
			{
				if(count == 255)//CHECK THAT THE STAYS BETWEEN 0-255
				{
					break;
				}
				count++;
			}
			//INCREMENT I BY COUNT WHICH IS THE NUMBER OF BYTES THAT HAVE ALREADY BEEN READ
			i = i + count;
			//WRITE THE COUNT FOLLOWED BY THE BYTE
			fwrite(&count,sizeof(unsigned char),1,fout);
			fwrite(&data,sizeof(unsigned char),1,fout);
			//GET THE SIZE OF THE COMPRESSED FILE (IF NEEDED FOR COMPUTATIONS)
			comp_size = comp_size + (2*sizeof(unsigned char));
		}
	}
	//DECOMPRESSION
	else if(command == 0)
	{
		//TRAVERSE THE FILE ONE BYTE AT A TIME
		for(i = 0; i < size; i+=2)
		{
			//READ THE COUNT AND THE BYTE
			count = file_data[i];
			data = file_data[i+1];
			x = 0;
			while(x < count)//WRITE THE BYTE THAT HAS BEEN READ AS MANY TIMES AS COUNT SAYS
			{
				fwrite(&data,sizeof(unsigned char),1,fout);
				decomp_size = decomp_size + sizeof(unsigned char);
				x++;
			}
		}
	}
	//ERROR CHECK FOR COMMAND VARIABLE
	else
	{
		printf("ERROR\n");
	}	
	//CLOSE THE OUTPUT FILE
	fclose(fout);
	return(0);
}
