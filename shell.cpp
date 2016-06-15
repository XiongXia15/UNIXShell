#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<string.h>
#include<stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include<signal.h>

#define BUFLEN 100

char *read_order(char *buffer);		//从键盘中读取命令
int order_number(const char *input);	//统计命令数量,以;相隔
char **order_name(const char *input);	//返回每条命令
int pipe_number(const char *input);	//统计管道数量
int pipel(char * input);
int is_back(char *order);     //分析是否为后台进程,并且将字符&去掉
int redirect(char *input);    //输入输出重定向
void do_cd(char *argv[]);     //用于cd命令
char *is_order_exist(const char *order);   //判断命令是否存在
int number(const char *input);	//分析命令和参数数量，来划分相应字符串
char **analize(const char *input);    //分析键入的命令,获取命令和参数并保存在arg中
void *history(void);
int save_cmd(void);
void set_keypress(void);
void reset_keypress(void);
void output_jobs(void);
void fg_jobs(char *arg);
void bg_jobs(char *arg);
void ctrl_z(int CtrlZ);
void ctrl_c(int CtrlC);
void ctrl_q(int CtrlQ);

typedef struct node
{
	int num;
	pid_t pid;
	char state[10];
	char comand[40];
	struct node *next;
}NODE;

NODE *jobs_link(NODE *pi);

static struct termios stored_settings;
char save[10][20];
char cmd[100]="";
int turn_num;
int NUM;
NODE *head=NULL;

pid_t PID;

void set_keypress(void)
{
    struct termios new_settings;

    tcgetattr(0,&stored_settings);

    new_settings = stored_settings;

    /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_lflag &= (~ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;

    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void reset_keypress(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

char *read_order(char *buffer)   //从键盘中读取命令
{
	int key,j=0,len;
	char *input,*path;
	int input_lenth=0,i;
	
	memset(cmd,0,100);
	path=getcwd(NULL,0);
//	lc_char=getchar();
	set_keypress();			//设置输入的模式
	while(1)				//输入命令
	{
		key=getchar();
		if(key == 27)		//输入的是光标，打印历史命令
		{
			history();
		}
		else if(key == 127)		//输入的是Backspace,删除命令字符
		{
			if(j>0)
				j=j-1;
			cmd[j] = '\0';
			printf("\r");
			printf("\r%s > $ %s",path,cmd);
		}
		else if(key == 10)	//键入Enter时，保存命令
		{
			save_cmd();
			break;
		}
		else			//保存命令到数组并显示在屏幕
		{
			printf("%c",key);
			cmd[j]=(char)key;
		//	printf("      j  %d   ",j);
			j++;
		}
	}
	len=j;
	j=0;
		printf("\n");	
		reset_keypress();		//恢复标准模式
//	printf("cmd %s \n",cmd);	 
	for(input_lenth=0;input_lenth<len;input_lenth++)
	{
		buffer[input_lenth] = cmd[input_lenth];  
	}
	
	if(input_lenth > BUFLEN)
	{
		fprintf(stderr,"The command is too long! Please reenter the command : \n");
		input_lenth = 0;
		return NULL;
	}
	else
		buffer[input_lenth] = '\0';    //加上结束符，形成字符串 
	if((input = (char *)malloc(sizeof(char)*(j+1))) == 0)
	{
		fprintf(stderr,"Error! Can't malloc enough space for input\n");
		return NULL;
	}
//	printf("input   %s\n",buffer);
	strcpy(input,buffer);
	return input;
}

int order_number(const char *input)	//统计命令数量,以;相隔
{
	int sum=0,i=0,len;
	len=strlen(input);
	while(i<len && (input[i] == ' ' || input[i] == '	'))
		i++;
	if(input[i] == ';')     //第一个输入字符为;则报错
	{
		fprintf(stderr,"Syntax Error. Unexpected token ';' \n");
		return -1;
	}
	if(i == len)    //输入只有空格或Tab
		return -1;
	for(i=0;i<len;i++)
		if(input[i] == ';')    //以;为命令分隔符
		{
			while(i<strlen(input) && (input[i+1] == ' ' || input[i+1] == '	'))   //跳过空格和Tab键
			i++;
			if(input[i+1] == ';')      //出现连续的;;
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

char **order_name(const char *input)	//返回每条命令
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
	for (i=0;i<=max_len;i++)   //取出以;为分隔符的命令作为返回值
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


int pipe_number(const char *input)	//统计管道数量
{
	int sum=0,i;
	for (i=0;i<strlen(input);i++)
	if(input[i]=='|')
		sum++;
	return sum;
}

int pipel(char *input)
{
	int k,len,status;
	char **order;
	int *child;
	int **fd; 
	int i=0,j=0,back=0; 
	
	len=strlen(input);
	k=pipe_number(input);
	order=(char **)malloc((k+1)*sizeof(char *));
	for(i=0;i<k+1;i++)
		order[i]=(char *)malloc((len+1)*sizeof(char));
	child=(int *)malloc((k+1)*sizeof(char *));   //子进程数组
	fd=(int **)malloc(k*sizeof(int *));        //管道数组
	for(i=0;i<k;i++)
		fd[i]=(int *)malloc(2*sizeof(int));
	
	k=0;
	for (i=0;i<=len;i++)    //取出被管道分隔符分开的命令
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

	for(i=0;i<k;i++)         //创建管道
		if(pipe(fd[i]) == -1) 
		{
			fprintf(stderr, "Open pipe error !\n");
			return 0;
		}
	i=0;
	if((child[i]=fork())==0)    //打开第一个子进程
	{
		close(fd[i][0]);	//关闭管道读入端，第一个管道没有管道输入
		if(fd[i][1] != STDOUT_FILENO) 
		{
			// 将标准输出重定向到管道的写入端，将该子进程的输出写入管道
			if(dup2(fd[i][1], STDOUT_FILENO) == -1) 
			{
				fprintf(stderr, "Redirect Standard Out error !\n");
				//printf("Redirect Standard Out error !\n");
				return -1;
			}
			close(fd[i][1]);	//关闭管道写入端
		}
		redirect(order[i]);	//执行命令
		exit(1);
	}
	else
	{
		waitpid(child[i],&status,0);
		close(fd[i][1]);       //退出子进程
	}
	i++;
	while(i<k)
	{
		if ((child[i]=fork())==0)
		{
			if(fd[i][0] != STDIN_FILENO) 
			{
				// 将标准的输入重定向到管道的读入端
				if(dup2(fd[i-1][0], STDIN_FILENO) == -1) 				
				{
					fprintf(stderr, "Redirect Standard In error !\n");
					//printf("Redirect Standard In Error !\n");
					return -1;
				}
				close(fd[i-1][0]);
				// 将标准输出重定向到管道的写入端，将该子进程的输出写入管道
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

void do_cd(char *argv[])	//用于cd命令
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

int redirect(char *input)	//输入输出重定向
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

/*	if(strcmp(analized_order[0], "q") == 0) //退出命令
	{
		printf("QUIT\n");
		// 释放申请的空间
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		exit (0);
		return 1;
	}
//	printf("analized_order[0] %s\n",analized_order[0]);
//	printf("analized_order[1] %s\n",analized_order[1]);
	if(strcmp(analized_order[0], "jobs") == 0) 
	{
		output_jobs();
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		return 1;
	}
	if(strcmp(analized_order[0], "fg") == 0) 
	{
		fg_jobs(analized_order[1]);
		return 1;
		
	}
	if(strcmp(analized_order[0], "bg") == 0) 
	{
		bg_jobs(analized_order[1]);
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		return 1;
	}


	/*如果输入的是cd命令*/
/*	if (strcmp(analized_order[0],"cd")==0)
	{
		do_cd(analized_order);
		// 释放申请的空间
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		return 1;
	}*/
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
//	if((pid = fork()) == 0) 
//	{
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
/*	}
	else
	{
	PID=pid;                     //父进程
	if(back==0)        // 并非后台执行指令
		pid=waitpid(pid, &status, 0);

	} */
	// 释放申请的空间
	free(out_filename);
	free(in_filename);
	free(order_path);
	for(i=0;i<k;i++)   
		free(analized_order[i]);
	free(analized_order);
	return 1;
}

char *is_order_exist(const char *order)	//判断命令是否存在
{
	char * path, * p;
	char *buffer;
	int i=0,max_length;

	path=getenv("PATH");	// 使用getenv函数来获取系统环境变量，用参数PATH表示获取路径
	p=path;
	max_length=strlen(path)+strlen(order)+2;
	if((buffer=(char *)malloc(max_length*(sizeof(char))))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for buffer\n");
		return NULL;
	}
	while(*p != '\0') 
	{
		if(*p != ':')  //路径列表使用":"来分隔路径
			buffer[i++] = *p;
		else 
		{
		//将指令和路径合成，形成pathname，并使用access函数来判断该文件是否存在
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
	return NULL;	//搜索完所有路径，依然没有找到则返回 NULL
}

int number(const char *input)	//分析命令和参数数量，来划分相应字符串
{
	int i=0,k=0;	////k记录命令和参数数量
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

char **analize(const char *input)	//分析键入的命令,获取命令和参数并保存在arg中
{
	int i,j,k;	//k记录命令和参数数量
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
	
	//将输入命令划分为相应命令和参数
	for (i=0,j=0,k=0;i<=input_len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>'||input[i]=='	'||input[i]=='\0')
		{
			if(j == 0)	//跳过连在一起的多个空格或者Tab
				continue;
			else 
			{
				buffer[j] = '\0';
				j++;
				arg[k] = (char *)malloc(sizeof(char)*j);
				strcpy(arg[k], buffer);
				j=0;  //处理下一个参数
				k++;
			}
		}
		else 
		{  
			//如果字符串最后是 '&'，则置后台运行标记为 1
			if(input[i]== '&' && input[i+1]=='\0') 
			{
				is_back = 1;
				continue;
			}
			buffer[j]=input[i];
			j++;
		}
	}

	//在使用exec执行命令的时候，最后的参数必须是NULL指针，所以将最后一个参数置成空值
	arg[k]=NULL;

	free(buffer);
	return arg;

}

void *history()
{
	int key=0;
	char *path;

	while(1)
	{
		getchar();
		key=getchar();
		if(key == 65)
		{
			path=getcwd(NULL,0);
			printf("\r                                                              ");
			printf("\r%s > $ %s",path,save[turn_num]);	
			strcpy(cmd,save[turn_num]);

			if(turn_num > 0)
			{
				turn_num--;
			}
			break;
		}
		else if(key == 66)
		{
			path=getcwd(NULL,0);
			printf("\r                                                              ");
			printf("\r%s > $ %s",path,save[turn_num]);

			strcpy(cmd,save[turn_num]);
			if(turn_num < 9)
			{
				turn_num++;
			}
			break;
		}	
	}
}

int save_cmd()
{
	int j;
	static int max_num;
	
	if(cmd[0] != '\0')
	{
		if(max_num == 9)
		{
			j=0;
			while(j<9)
			{
				strcpy(save[j],save[j+1]);
				j++;
			}
		}
		strcpy(save[max_num],cmd);
		max_num++;

		if(max_num == 10)
		{
			max_num=9;
		}
	}
	turn_num = max_num;
	return 0;
}

NODE *jobs_link(NODE *pi)
{
	NODE *pf,*pb;

	pb=head;

	if(head == NULL)
	{
		NUM=pi->num;
		head=pi;
		pi->next=NULL;
	}
	else
	{
		while(pb->next != NULL)
		{
			pf=pb;
			pb=pb->next;
		}
		NUM=pi->num;
		pb->next=pi;
		pi->next=NULL;
	}
	NUM++;
	return (head);
}


void output_jobs()
{
	NODE *p;

	p=head;

	if(head != NULL)
	{
		do
		{
			printf("[%d]  %d  %s  %s\n",(p->num)+1,p->pid,p->state,p->comand);
			p=p->next;
		}while(p != NULL);
	}
	else
		printf("no jobs in list!\n");
}


void ctrl_z(int CtrlZ)
{
	NODE *pi=NULL;
	
	printf("input ctrl_z\n");
	
	pi=(NODE *)malloc(sizeof(NODE))	;
	pi->num=NUM;
	pi->pid=PID;
	strcpy(pi->state,"stopped");
	strcpy(pi->comand," ");
	head=jobs_link(pi);
	kill(PID,SIGSTOP);
}

void ctrl_c(int CtrlC) 
{
	printf("input ctrl_c\n");
	return;
//	fflush(stdout);
}

void ctrl_q(int CtrlQ)
{
	printf("input ctrl_q\n");
	return;
//	fflush(stdout);
}

void fg_jobs(char *arg)
{
	int num,len;
	char *buffer;
	NODE *p=NULL;
	pid_t pid=0;

	p=head;

	if(arg != NULL)
	{
		sscanf(arg,"%d",&num);
	//	len=strlen(arg);
		num--;
		printf("fg arg %s %d\n",arg,num);
		if(p == NULL)
		{
			printf("no jobs!!\n");
		}
		while(p != NULL)
		{
			if(num == p->num)
			{
				pid=p->pid;
				break;
			}
			p=p->next;
		}
		kill(pid,SIGCONT);
		waitpid(pid,NULL,WUNTRACED);
	}
}

void bg_jobs(char *arg)
{
	int num;
	NODE *p=NULL;
	pid_t pid=0;

	p=head;

	if(arg != NULL)
	{
		sscanf(arg,"%d",&num);
		
		if(p == NULL)
		{
			printf("no jobs!!\n");
		}
		while(p != NULL)
		{
			if(num == p->num)
			{
				pid=p->pid;
				break;
			}
			p=p->next;
		}
		kill(pid,SIGCONT);
	}
}  

int main(int argc,char **argv)
{
	char *path,*buffer;
	char *all_order,**every_order,**analized_order;
	int i=0,pipe,k,number,len;
	NODE *pi=NULL;
	int flag=0;
	pid_t pid;

	if((buffer=(char *)malloc(BUFLEN*(sizeof(char))))==0)
	{
		printf("error! can't malloc enough space for buffer\n");
		return (0);
	}

	signal(SIGTSTP,ctrl_z);			
	signal(SIGINT,ctrl_c);			
	signal(SIGQUIT,ctrl_q);

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
				analized_order=analize(every_order[i]);//命令已经保存在*analized_order[]中
				if(strcmp(analized_order[0], "q") == 0) //退出命令
				{
					printf("QUIT\n");
					// 释放申请的空间
					for(i=0;i<k;i++)
						free(analized_order[i]);
					free(analized_order);
					exit (0);
					return 1;
				}
					
				else if(strcmp(analized_order[0], "jobs") == 0) 
				{
					output_jobs();
					for(i=0;i<k;i++)
						free(analized_order[i]);
					free(analized_order);
				}
				else if(strcmp(analized_order[0], "fg") == 0) 
				{
					fg_jobs(analized_order[1]);
		
				}
				else if(strcmp(analized_order[0], "bg") == 0) 
				{
					bg_jobs(analized_order[1]);
					for(i=0;i<k;i++)
						free(analized_order[i]);
					free(analized_order);
				}
				/*如果输入的是cd命令*/
				else if (strcmp(analized_order[0],"cd")==0)
				{
					do_cd(analized_order);
					// 释放申请的空间
					for(i=0;i<k;i++)
						free(analized_order[i]);
					free(analized_order);
				}
				else 
				{
				if(is_back(every_order[i])==1) 
				{
					len = strlen(every_order[i]);
					if(every_order[i][len] == '&')
						every_order[i][len]='\0';
					flag=1;
					pi=(NODE *)malloc(sizeof(NODE));
					pi->num=NUM;
					pi->pid=getpid();
					strcpy(pi->state,"stopped");
					strcpy(pi->comand,every_order[i]);
					head=jobs_link(pi);
				}
				if((pid=fork()) < 0)	//创建子进程
				{
					perror("fork error!\n");
					exit (1);
				}
				if(pid == 0)
				{					
					k=pipe_number(every_order[i]);	
					if(k!=0)
					pipel(every_order[i]);
					else
					redirect(every_order[i]);
				//	printf("redirect\n");
				
				//for debug
				//printf("%s\n",every_order[i]);
				}
				else
				{
					PID=pid;
					if(flag == 1)	//执行后台命令，父进程不等待子进程
					{
						;
					}
					else	        //执行前台命令，父进程等待子进程
					{
						waitpid(pid,NULL,WUNTRACED);
					}
				}	
			}
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

