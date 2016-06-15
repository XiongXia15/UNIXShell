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

char *read_order(char *buffer);		//�Ӽ����ж�ȡ����
int order_number(const char *input);	//ͳ����������,��;���
char **order_name(const char *input);	//����ÿ������
int pipe_number(const char *input);	//ͳ�ƹܵ�����
int pipel(char * input);
int is_back(char *order);     //�����Ƿ�Ϊ��̨����,���ҽ��ַ�&ȥ��
int redirect(char *input);    //��������ض���
void do_cd(char *argv[]);     //����cd����
char *is_order_exist(const char *order);   //�ж������Ƿ����
int number(const char *input);	//��������Ͳ�����������������Ӧ�ַ���
char **analize(const char *input);    //�������������,��ȡ����Ͳ�����������arg��
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

char *read_order(char *buffer)   //�Ӽ����ж�ȡ����
{
	int key,j=0,len;
	char *input,*path;
	int input_lenth=0,i;
	
	memset(cmd,0,100);
	path=getcwd(NULL,0);
//	lc_char=getchar();
	set_keypress();			//���������ģʽ
	while(1)				//��������
	{
		key=getchar();
		if(key == 27)		//������ǹ�꣬��ӡ��ʷ����
		{
			history();
		}
		else if(key == 127)		//�������Backspace,ɾ�������ַ�
		{
			if(j>0)
				j=j-1;
			cmd[j] = '\0';
			printf("\r");
			printf("\r%s > $ %s",path,cmd);
		}
		else if(key == 10)	//����Enterʱ����������
		{
			save_cmd();
			break;
		}
		else			//����������鲢��ʾ����Ļ
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
		reset_keypress();		//�ָ���׼ģʽ
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
		buffer[input_lenth] = '\0';    //���Ͻ��������γ��ַ��� 
	if((input = (char *)malloc(sizeof(char)*(j+1))) == 0)
	{
		fprintf(stderr,"Error! Can't malloc enough space for input\n");
		return NULL;
	}
//	printf("input   %s\n",buffer);
	strcpy(input,buffer);
	return input;
}

int order_number(const char *input)	//ͳ����������,��;���
{
	int sum=0,i=0,len;
	len=strlen(input);
	while(i<len && (input[i] == ' ' || input[i] == '	'))
		i++;
	if(input[i] == ';')     //��һ�������ַ�Ϊ;�򱨴�
	{
		fprintf(stderr,"Syntax Error. Unexpected token ';' \n");
		return -1;
	}
	if(i == len)    //����ֻ�пո��Tab
		return -1;
	for(i=0;i<len;i++)
		if(input[i] == ';')    //��;Ϊ����ָ���
		{
			while(i<strlen(input) && (input[i+1] == ' ' || input[i+1] == '	'))   //�����ո��Tab��
			i++;
			if(input[i+1] == ';')      //����������;;
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

char **order_name(const char *input)	//����ÿ������
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
	for (i=0;i<=max_len;i++)   //ȡ����;Ϊ�ָ�����������Ϊ����ֵ
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


int pipe_number(const char *input)	//ͳ�ƹܵ�����
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
	child=(int *)malloc((k+1)*sizeof(char *));   //�ӽ�������
	fd=(int **)malloc(k*sizeof(int *));        //�ܵ�����
	for(i=0;i<k;i++)
		fd[i]=(int *)malloc(2*sizeof(int));
	
	k=0;
	for (i=0;i<=len;i++)    //ȡ�����ܵ��ָ����ֿ�������
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

	for(i=0;i<k;i++)         //�����ܵ�
		if(pipe(fd[i]) == -1) 
		{
			fprintf(stderr, "Open pipe error !\n");
			return 0;
		}
	i=0;
	if((child[i]=fork())==0)    //�򿪵�һ���ӽ���
	{
		close(fd[i][0]);	//�رչܵ�����ˣ���һ���ܵ�û�йܵ�����
		if(fd[i][1] != STDOUT_FILENO) 
		{
			// ����׼����ض��򵽹ܵ���д��ˣ������ӽ��̵����д��ܵ�
			if(dup2(fd[i][1], STDOUT_FILENO) == -1) 
			{
				fprintf(stderr, "Redirect Standard Out error !\n");
				//printf("Redirect Standard Out error !\n");
				return -1;
			}
			close(fd[i][1]);	//�رչܵ�д���
		}
		redirect(order[i]);	//ִ������
		exit(1);
	}
	else
	{
		waitpid(child[i],&status,0);
		close(fd[i][1]);       //�˳��ӽ���
	}
	i++;
	while(i<k)
	{
		if ((child[i]=fork())==0)
		{
			if(fd[i][0] != STDIN_FILENO) 
			{
				// ����׼�������ض��򵽹ܵ��Ķ����
				if(dup2(fd[i-1][0], STDIN_FILENO) == -1) 				
				{
					fprintf(stderr, "Redirect Standard In error !\n");
					//printf("Redirect Standard In Error !\n");
					return -1;
				}
				close(fd[i-1][0]);
				// ����׼����ض��򵽹ܵ���д��ˣ������ӽ��̵����д��ܵ�
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
			// ����׼�������ض��򵽹ܵ��Ķ����
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

void do_cd(char *argv[])	//����cd����
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

int is_back(char *order)	//�����Ƿ�Ϊ��̨����,���ҽ��ַ�&ȥ��
{
	int len=strlen(order);
	if(order[len]=='&')
	{
		order[len]='\0';
		return 1;
	}
	else return 0;
}

int redirect(char *input)	//��������ض���
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

	//��ȡ�ַ����е�����,�������real_order��
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
	//����ǰ��Ŀո�
	while ((input[i]==' '||input[i]=='	')&&i<len)
		i++;
	j=0;
	out_filename[0]='\0';
	in_filename[0]='\0';
	//��ȡ���������������ļ�
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
	//�ȴ�������ض���Ҳ���������ض���
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
			//����ǰ��Ŀո�
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
			//����ǰ��Ŀո�
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
	analized_order=analize(real_order);//�����Ѿ�������*analized_order[]��

/*	if(strcmp(analized_order[0], "q") == 0) //�˳�����
	{
		printf("QUIT\n");
		// �ͷ�����Ŀռ�
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


	/*����������cd����*/
/*	if (strcmp(analized_order[0],"cd")==0)
	{
		do_cd(analized_order);
		// �ͷ�����Ŀռ�
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
		// �ͷ�����Ŀռ�
		for(i=0;i<k;i++)
			free(analized_order[i]);
		free(analized_order);
		free(real_order);
		return -1;
	}
		//�����ӽ�������ִ������
//	if((pid = fork()) == 0) 
//	{
		/* ������������ض���*/
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
		  //ʹ��dup2��������׼����ض���fd_out��
		if(flag_out>0)
			if(dup2(fd_out, STDOUT_FILENO) == -1)
			{
				fprintf(stderr,"Redirect Standard Out Error !\n");
				exit(1);
			}
		  //ʹ��dup2��������׼�����ض���fd_in��
		if(flag_in>0)
			if (dup2(fd_in,STDIN_FILENO)==-1)
			{
				fprintf(stderr,"Redirect Standard Out Error !\n");
				exit(1);
			}
		execv(order_path,analized_order);
		exit(1);		//�ӽ����Ƴ�
/*	}
	else
	{
	PID=pid;                     //������
	if(back==0)        // ���Ǻ�ִ̨��ָ��
		pid=waitpid(pid, &status, 0);

	} */
	// �ͷ�����Ŀռ�
	free(out_filename);
	free(in_filename);
	free(order_path);
	for(i=0;i<k;i++)   
		free(analized_order[i]);
	free(analized_order);
	return 1;
}

char *is_order_exist(const char *order)	//�ж������Ƿ����
{
	char * path, * p;
	char *buffer;
	int i=0,max_length;

	path=getenv("PATH");	// ʹ��getenv��������ȡϵͳ�����������ò���PATH��ʾ��ȡ·��
	p=path;
	max_length=strlen(path)+strlen(order)+2;
	if((buffer=(char *)malloc(max_length*(sizeof(char))))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for buffer\n");
		return NULL;
	}
	while(*p != '\0') 
	{
		if(*p != ':')  //·���б�ʹ��":"���ָ�·��
			buffer[i++] = *p;
		else 
		{
		//��ָ���·���ϳɣ��γ�pathname����ʹ��access�������жϸ��ļ��Ƿ����
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
	return NULL;	//����������·������Ȼû���ҵ��򷵻� NULL
}

int number(const char *input)	//��������Ͳ�����������������Ӧ�ַ���
{
	int i=0,k=0;	////k��¼����Ͳ�������
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

char **analize(const char *input)	//�������������,��ȡ����Ͳ�����������arg��
{
	int i,j,k;	//k��¼����Ͳ�������
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
	
	//�����������Ϊ��Ӧ����Ͳ���
	for (i=0,j=0,k=0;i<=input_len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>'||input[i]=='	'||input[i]=='\0')
		{
			if(j == 0)	//��������һ��Ķ���ո����Tab
				continue;
			else 
			{
				buffer[j] = '\0';
				j++;
				arg[k] = (char *)malloc(sizeof(char)*j);
				strcpy(arg[k], buffer);
				j=0;  //������һ������
				k++;
			}
		}
		else 
		{  
			//����ַ�������� '&'�����ú�̨���б��Ϊ 1
			if(input[i]== '&' && input[i+1]=='\0') 
			{
				is_back = 1;
				continue;
			}
			buffer[j]=input[i];
			j++;
		}
	}

	//��ʹ��execִ�������ʱ�����Ĳ���������NULLָ�룬���Խ����һ�������óɿ�ֵ
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
				analized_order=analize(every_order[i]);//�����Ѿ�������*analized_order[]��
				if(strcmp(analized_order[0], "q") == 0) //�˳�����
				{
					printf("QUIT\n");
					// �ͷ�����Ŀռ�
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
				/*����������cd����*/
				else if (strcmp(analized_order[0],"cd")==0)
				{
					do_cd(analized_order);
					// �ͷ�����Ŀռ�
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
				if((pid=fork()) < 0)	//�����ӽ���
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
					if(flag == 1)	//ִ�к�̨��������̲��ȴ��ӽ���
					{
						;
					}
					else	        //ִ��ǰ̨��������̵ȴ��ӽ���
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

