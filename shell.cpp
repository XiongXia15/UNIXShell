#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<string.h>
#include<stdlib.h>


#define BUFLEN 80

char *read_order(char *buffer)
{
	char lc_char;
	char *input;
	int input_lenth=0;
	
	lc_char=getchar();
	while(lc_char != '\n' && input_lenth < BUFLEN)
	{
		buffer[input_lenth] = lc_char;
		lc_char = getchar();
		input_lenth ++;
	}
	if(input_lenth > BUFLEN)
	{
		fprintf(stderr,"The command is too long! Please reenter the command : \n");
		input_lenth = 0;
		return NULL;
	}
	else
		buffer[input_lenth] = '\0';
	if((input = (char *)malloc(sizeof(char)*(input_lenth+1))) == 0)
	{
		fprintf(stderr,"Error! Can't malloc enough space for input\n");
		return NULL;
	}
	strcpy(input,buffer);
	return input;
}

char **order_name(const char *input)
{
	

}

int pipel(char *input)
{


}

int main(int argc,char **argv)
{
	char *read_order(char *buffer);
	char **order_name(const char *input);
	int pipel(char *input);
}
