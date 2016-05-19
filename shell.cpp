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

int order_number(const char *input)
{
	int sum=0,i=0,len;
	len=strlen(input);
	while(i<len && (input[i] == ' ' || input[i] == '	'))
		i++;
	if(input[i] == ';')
	{
		fprintf(stderr,"Syntax Error. Unexpected token ';' \n");
		return -1;
	}
	if(i == len)
		return -1;
	for(i=0;i<len;i++)
		if(input[i] == ';')
		{
			while(i<strlen(input) && (input[i+1] == ' ' || input[i+1] == '	'))
			i++;
			if(input[i+1] == ';')
			{
				fprintf(stderr,"Syntax Error. Unexpected token ';;' \n")
				return -1;
			}
			else
				sum++;
		}
	sum=sum+1;
	return sum;
}

char **order_name(const char *input)
{
//	int order_number(const char *input);
	int i,j,k,max_len;
	char **order;
	
	max_len=strlen(input);
	k=order_number(input);
	order=(char **)malloc(k*sizeof(char *));
	for(i=0;i<k;i++)
	{
		order[i]=(char *)malloc((max_len+1)*sizeof(char));
		order[i][0]='\0';
	}
	//分别取出被";"分割开的命令
	k=0;
	j=0;
	for (i=0;i<=max_len;i++)
	{
		if (input[i]!=';')
		{
			order[k][j]=input[i];
			j++;
		}
		else
		{
			order[k][j]='\0';
			k++;
			j=0;
		}
	}
	//show the orders that are departed by ';'
	/*for(i=0;i<k+1;i++)
		printf("%s\n",order[i]);*/
	return order;
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
