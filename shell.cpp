#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<string.h>
#include<stdlib.h>
#include <fcntl.h>
#include <errno.h>


#define BUFLEN 100

char *read_order(char *buffer);		//从键盘中读取命令
int order_number(const char *input);	//统计命令数量,以;相隔
char **order_name(const char *input);	//返回每条命令
int pipe_number(const char *input);//统计管道数量
int pipel(char * input);
int is_back(char *order);     //分析是否为后台进程,并且将字符&去掉
int redirect(char *input);    //输入输出重定向
void do_cd(char *argv[]);     //用于cd命令
char *is_order_exist(const char *order);   //判断命令是否存在
int number(const char *input);	//分析命令和参数数量，来划分相应字符串
char **analize(const char *input);    //分析键入的命令,获取命令和参数并保存在arg中
    

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
				fprintf(stderr,"Syntax Error. Unexpected token ';;' \n");
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
	int i,j=0,k,max_len;
	char **order;
	
	max_len=strlen(input);
	k=order_number(input);
	order=(char **)malloc(k*sizeof(char *));
	for(i=0;i<k;i++)
	{
		order[i]=(char *)malloc((max_len+1)*sizeof(char));
		order[i][0]='\0';
	}
	k=0;
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
	int k=0,len,status;
	char **order;
	int *child;
	int **fd; 
	int i=0,j=0,back=0; 
	
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
	
	/*for(i=0;i<k+1;i++)
		printf("%s\n",order[i]);*/

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
	i++;
	while(i<k)
	{
		if ((child[i]=fork())==0)
		{
			if(fd[i][0] != STDIN_FILENO) 
			{
				if(dup2(fd[i-1][0], STDIN_FILENO) == -1) 				
				{
					fprintf(stderr, "Redirect Standard In error !\n");
					//printf("Redirect Standard In Error !\n");
					return -1;
				}
				close(fd[i-1][0]);
				// 将标准的输出重定向到管道的写入端，这样该子进程的输出就写入了管道
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
			//wait for child
			waitpid(child[i],&status,0);
			close(fd[i][1]);
			i++;
		}
	}
	
	if((child[i] = fork()) == 0) 
	{
		close(fd[i-1][1]);
		if(fd[i-1][0] != STDIN_FILENO) 
		{
			// 将标准的输入重定向到管道的读入端
			if(dup2(fd[i-1][0], STDIN_FILENO) == -1) 
			{
				fprintf(stderr, "Redirect Standard In error !\n");
				//printf("Redirect Standard In Error !\n");
				return -1;
			}
			close(fd[i-1][0]);
		}
		redirect(order[i]);
		exit(1);
	}
	else if(back==0)  
	{
		waitpid(child[i], NULL, 0);
		close(fd[i-1][1]);
	}

	for(i=0;i<k;i++)
		free(fd[i]);
	free(fd);
	for(i=0;i<k+1;i++)
		free(order[i]);
	free(order);
	free(child);
	return 1;

}

void do_cd(char *argv[])	//专门用于cd命令
{ 
	if(argv[1]!=NULL)
	{ 
		if(chdir(argv[1])<0) 
			switch(errno)
		{ 
			case ENOENT: 
				fprintf(stderr,"DIRECTORY NOT FOUND\n"); 
				break; 
			case ENOTDIR: 
				fprintf(stderr,"NOT A DIRECTORY NAME\n"); 
				break; 
			case EACCES: 
				fprintf(stderr,"YOU DO NOT HAVE RIGHT TO ACCESS\n"); 
				break; 
			default: 
				fprintf(stderr,"SOME ERROR HAPPENED IN CHDIR\n"); 
		} 
	}
} 

int is_back(char *order)	//分析是否为后台进程,并且将字符&去掉
{
	int len=strlen(order);
	if(order[len]=='&')
	{
		order[len]='\0';
		return 1;
	}
	else return 0;
}

int redirect(char *input)	
{
	char *order_path,*real_order;
	char *out_filename,*in_filename;
	char **analized_order;
	int len,status,i,j,k,back=0,fd_out,fd_in,flag_out=0,flag_in=0;
	pid_t pid;
	
	back=is_back(input);
	len=strlen(input);
	out_filename=(char *)malloc((len+1)*(sizeof(char)));
	in_filename=(char *)malloc((len+1)*(sizeof(char)));
	real_order=(char *)malloc((len+1)*(sizeof(char)));

	//读取字符串中的命令,并存放于real_order中
	for(i=0;i<len;i++)
	{
		if (input[i]!='>'&&input[i]!='<')
			real_order[i]=input[i];
		else
		{
			if (input[i]=='>')
				flag_out=1;
			if (input[i]=='<')
				flag_in=1;
			break;
		}
	}
	real_order[i]='\0';
	i++;
	if(flag_out==1&&input[i]=='>')
	{
		flag_out=2;
		i++;
	}
	else if (flag_in==1&&input[i]=='<')
	{
		flag_in=2;
		i++;
	}
	//读出前面的空格
	while ((input[i]==' '||input[i]=='	')&&i<len)
		i++;
	j=0;
	out_filename[0]='\0';
	in_filename[0]='\0';
	//读取定向输入或输出的文件
	if(flag_out>0)
	{	
		while (i<=len)
		{
			if(input[i]=='<')
			{
				out_filename[j]='\0';
				break;
			}
			out_filename[j]=input[i];
			i++;
			j++;
		}
	}
	if(flag_in>0)
	{
		while (i<=len)
		{
			if (input[i]=='>')
			{
				in_filename[j]='\0';
				break;
			}
			in_filename[j]=input[i];
			i++;
			j++;
		}
	}
	//既存在输出重定向也存在输入重定向
	if (i<len)
	{
		j=0;
		if (flag_out>0&&input[i]=='<')
		{
			i++;
			flag_in=1;
			if(input[i]=='>')
			{
				flag_in=2;
				i++;
			}
			//读出前面的空格
			while ((input[i]==' '||input[i]=='	')&&i<len)
				i++;
			while (i<=len)
			{
				in_filename[j]=input[i];
				i++;
				j++;
			}
		}
		else if (flag_in>0&&input[i]=='>')
		{
			i++;
			flag_out=1;
			if(input[i]=='>')
			{
				flag_out=2;
				i++;
			}
			//读出前面的空格
			while ((input[i]==' '||input[i]=='	')&&i<len)
				i++;
			while (i<=len)
			{
				out_filename[j]=input[i];
				i++;
				j++;
			}
		}
		else
		{
			fprintf(stderr,"ERROR!can't find the file!\n");
			return -1;
		}
	}
	
	//for debug
	/*printf("real_order: %s\n",real_order);
	printf("out_filename: %s\n",out_filename);
	printf("in_filename: %s\n",in_filename);*/

	k=number(real_order);
	analized_order=analize(real_order);//命令已经保存在*analized_order[]中
	if(strcmp(analized_order[0], "q") == 0) //退出命令
	{
		printf("QUIT\n");
		// 释放申请的空间
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		exit(1);
		return 1;
	}

	/*如果输入的是cd命令*/
	if (strcmp(analized_order[0],"cd")==0)
	{
		do_cd(analized_order);
		// 释放申请的空间
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		return 1;
	}
	order_path=is_order_exist(analized_order[0]);
	if(order_path==NULL)	//can't find the order
	{
		fprintf(stderr,"This is command is not founded ?!\n");
		// 释放申请的空间
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		return -1;
	}
		//创建子进程用于执行命令
	if((pid = fork()) == 0) 
	{
		/* 存在输出输入重定向*/
	    if(flag_out==1)
			fd_out = open(out_filename,O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );
		if(flag_out==2)
			fd_out = open(out_filename, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR );
		if(flag_in==1)
			fd_in = open(in_filename, O_RDONLY, S_IRUSR|S_IWUSR );
		if(flag_in==2)
			fd_in = open(in_filename, O_RDONLY, S_IRUSR|S_IWUSR );
        if(fd_out==-1) 
		{
			printf("Open out %s error \n", out_filename);
			return -1;
		}
		if(fd_in==-1) 
		{
			fprintf(stderr,"Open in %s error \n", in_filename);
			return -1;
		}
		  //使用dup2函数将标准输出重定向到fd_out上
		if(flag_out>0)
			if(dup2(fd_out, STDOUT_FILENO) == -1)
			{
				fprintf(stderr,"Redirect Standard Out Error !\n");
				exit(1);
			}
		  //使用dup2函数将标准输入重定向到fd_in上
		if(flag_in>0)
			if (dup2(fd_in,STDIN_FILENO)==-1)
			{
				fprintf(stderr,"Redirect Standard Out Error !\n");
				exit(1);
			}
		execv(order_path,analized_order);
		exit(1);		//子进程推出
	}
	else                     //父进程
	if(back==0)        // 并非后台执行指令
		pid=waitpid(pid, &status, 0);
	// 释放申请的空间
	free(out_filename);
	free(in_filename);
	free(order_path);
	for(i=0;i<k;i++)   
		free(analized_order[i]);
	free(analized_order);
	return 1;
}

char *is_order_exist(const char *order)
{
	char * path, * p;
	char *buffer;
	int i=0,max_length;

	path=getenv("PATH");
	p=path;
	max_length=strlen(path)+strlen(order)+2;
	if((buffer=(char *)malloc(max_length*(sizeof(char))))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for buffer\n");
		return NULL;
	}
	while(*p != '\0') 
	{
		if(*p != ':')  
			buffer[i++] = *p;
		else 
		{
		buffer[i++] = '/';
		buffer[i] = '\0';
		strcat(buffer,order);
		if(access(buffer,F_OK) == 0)     
			return buffer;
		else                          
			i=0;
		}
		p++;
	}
	return NULL;
}

int number(const char *input)	
{
	int i=0,k=0;
	int input_len=strlen(input);	
	int flag=0;
	for (i=0;i<input_len;i++)
	{

		if(input[i]==' '||input[i]=='<'||input[i]=='>'||input[i]=='	')
		{
			flag=0;
			continue;
		}
		else 
		{
			if(flag==0)
			{
				flag=1;
				k++;
			}
		}
	}
	return k;
}

char **analize(const char *input)
{
	int i,j,k;	
	int input_len;
	int is_back=0;

	char *buffer;
	char **arg;

	input_len=strlen(input);

	if((buffer=(char *)malloc((input_len+1)*(sizeof(char))))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for buffer\n");
		return NULL;
	}
	
	k=number(input);
	if((arg=(char **)malloc((k+1)*sizeof(char *)))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return NULL;
	}
	
	for (i=0,j=0,k=0;i<=input_len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>'||input[i]=='	'||input[i]=='\0')
		{
			if(j == 0)
				continue;
			else 
			{
				buffer[j] = '\0';
				j++;
				arg[k] = (char *)malloc(sizeof(char)*j);
				strcpy(arg[k], buffer);
				j=0;  
				k++;
			}
		}
		else 
		{  
			if(input[i]== '&' && input[i+1]=='\0') 
			{
				is_back = 1;
				continue;
			}
			buffer[j]=input[i];
			j++;
		}
	}

	arg[k]=NULL;

	free(buffer);
	return arg;

}



int main(int argc,char **argv)
{
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
		printf("%s > $ ", path);
		all_order=read_order(buffer);		
		if(all_order==NULL)
			continue;
		number=order_number(all_order);
		if (number<0)
			continue;
		every_order=order_name(all_order);
		i=0;
		while (i<number)
		{	
			if(strlen(every_order[i])!=0)
			{
				k=pipe_number(every_order[i]);	
				if(k!=0)
					pipel(every_order[i]);
				else
					redirect(every_order[i]);
				//for debug
				//printf("%s\n",every_order[i]);	
			}	
			i++;
		}
		for(i=0;i<number;i++)
			free(every_order[i]);
		free(every_order);
		free(all_order);
		free(path);
	}
}




