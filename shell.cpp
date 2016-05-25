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
	int i,j=0,k=0,max_len;
	char **order;
	
	max_len=strlen(input);
	k=order_number(input);
	order=(char **)malloc(k*sizeof(char *));
	for(i=0;i<k;i++)
	{
		order[i]=(char *)malloc((max_len+1)*sizeof(char));
		order[i][0]='\0';
	}
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


int pipe_number(const char *input)
{
	int sum=0,i;
	for (i=0;i<strlen(input);i++)
	if(input[i]=='|')
		sum++;
	return sum;
}

int pipel(char *input)
{
	int k=0;len;
	char **order;
	int *child;
	int **fd; 
	int i=0,j=0; 
	
	len=strlen(input);
	k=pipe_number(input);
	order=(char **)malloc((k+1)*sizeof(char *));
	for(i=0;i<k+1;i++)
		order[i]=(char *)malloc((len+1)*sizeof(char));
	child=(int *)malloc((k+1)*sizeof(char *));
	fd=(int **)malloc(k*sizeof(int *));
	for(i=0;i<k;i++)
		fd[i]=(int *)malloc(2*sizeof(int));
	
	for (i=0;i<=len;i++)
	{
		if (input[i]!='|')
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
	for(i=0;i<k+1;i++)
		printf("%s\n",order[i]);

	for(i=0;i<k;i++)
		if(pipe(fd[i]) == -1) 
		{
			fprintf(stderr, "Open pipe error !\n");
			return 0;
		}
	i=0;
	if((child[i]=fork())==0)
	{
		close(fd[i][0]);
		if(fd[i][1] != STDOUT_FILENO) 
		{
			if(dup2(fd[i][1], STDOUT_FILENO) == -1) 
			{
				fprintf(stderr, "Redirect Standard Out error !\n");
				//printf("Redirect Standard Out error !\n");
				return -1;
			}
			close(fd[i][1]);
		}
		redirect(order[i]);	
		exit(1);
	}
	else
	{
		waitpid(child[i],&status,0);
		close(fd[i][1]);
	}



}

int redirect(char *input)	
{

}

int main(int argc,char **argv)
{
	char *read_order(char *buffer);
	char **order_name(const char *input);
	int pipel(char *input);

	char *path,*buffer;
	char *all_order,**every_order;
	int i=0,pipe,k,number;

	if((buffer=(char *)malloc(BUFLEN*(sizeof(char))))==0)
	{
		printf("error! can't malloc enough space for buffer\n");
		return (0);
	}
	while(1)
	{
		path=getcwd(NULL,0);	
		printf("%s > $", path);
		all_order=read_order(buffer);		
		if(all_order==NULL)
			continue;
		number=order_number(all_order);
		if (number<0)
			continue;
		every_order=order_name(all_order);
	}
	while (i<number)
	{
		if(strlen(every_order[i])!=0)
		{
			k=pipe_number(every_order[i]);	
			if(k!=0)
				pipel(every_order[i]);
			else
				redirect(every_order[i]);
		}
		i++;
	}
	for(i=0;i<number;i++)
		free(every_order[i]);
	free(every_order);
	free(all_order);
	free(path);
}
