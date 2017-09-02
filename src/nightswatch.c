#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include <sys/select.h>
#include "nightswatch.h"

int prevInt[16];
struct termios saved_attributes;

int Handle_nightswatch(char **Token)
{
	int ct = 0, num_sec, flag = 0;
	char numstr[32];

	flag = check_numsec(Token);

	if(flag != -1)
	{		
		num_sec = strtoint(Token[flag+1]);
	}
	else
		num_sec = 2;

	while(Token[ct]!=NULL)
		ct++;
	if(strcmp(Token[ct-1],"interrupt") == 0)
	{
		Handle_n_interrupt(Token,num_sec);
	}
	else if(strcmp(Token[ct-1],"dirty") == 0)
	{
		Handle_n_dirty(Token, num_sec);
	}
	return 0;
}

int Handle_n_dirty( char **Token, int sec)
{
	pid_t pid, wpid;
	int status;
	set_input_mode();
	pid = fork();
	if(pid == 0)
	{
		Recur_dirty(Token,sec);
		exit(1);
	}
	else if(pid < 0)
		perror("FORK ERROR");
	else
		wpid = waitpid(pid, &status, WUNTRACED);
	reset_input_mode();

}

int Handle_n_interrupt(char **Token, int sec)
{
	FILE * fd;
	char * line = NULL;
	size_t len = 0;
	ssize_t rd;
	int  i = 0, status;
	fd = fopen("/proc/interrupts", "r");
	if (fd == NULL)
		perror("fopen");
	rd = getline(&line, &len, fd);

	fclose(fd);
	char **CPUlist = getCPUlist(line);
	while(CPUlist[i]!=NULL)
		printf("%s\t",CPUlist[i++]);
	printf("\n");
	free(CPUlist);
	int  k = 0;
	for(k=0;k<16;k++)
		prevInt[k] = 0;
	set_input_mode ();
	pid_t pid, wpid;
	pid = fork();
	if(pid == 0)
	{
		Recur_interrupt(i-1, sec);
		exit(1);
	}
	else if (pid < 0)
	{
		perror("FORK ERROR");
	}
	else
	{
		wpid = waitpid(pid, &status, WUNTRACED);
	}
	reset_input_mode();
}


int Recur_dirty(char **Token, int sec)
{
	char c;
	while(1)
	{
		FILE * fd;
		char * line = NULL;
		size_t len = 0;
		ssize_t rd;
		int  i = 0;
		fd = fopen("/proc/meminfo", "r");
		if (fd == NULL)
			perror("fopen");

		while((rd = getline(&line, &len, fd)) != -1)
		{
		 	if(strstr(line, "Dirty:"))
		 	{
		 		char **param = getCPUlist(line);
			 	printf("%s kB\n", param[1]);
				free(param);
				break;
			}
		}
		fclose(fd);
		long timctr = 0;
		long max = (long)sec*1000;
		int flag = 0;
		while(timctr < max)
		{
			timctr++;
			usleep(1000);
			c = '/';
			if(inputAvailable())
				read(STDIN_FILENO, &c, 1);
			if( c=='q')
			{
				printf("Quitting Process...\n");
				flag = 1;
				break;
			}
		}
		if(flag == 1)
			break;
	}
	return 0;
}

int Recur_interrupt(int CPUnum, int sec)
{
	char c;
	
	while(1)
	{
		FILE * fd;
		char * line = NULL;
		size_t len = 0;
		ssize_t rd;
		int  i = 0;
		fd = fopen("/proc/interrupts", "r");
		if (fd == NULL)
			perror("fopen");
		rd = getline(&line, &len, fd);
		while((rd = getline(&line, &len, fd)) != -1)
		{
		 	if(strstr(line, "i8042") && strstr(line, "1:"))
		 	{
		 		char **param = getCPUlist(line);
		 		int j = 0;
		 		for(j=0; j<CPUnum; j++)
		 		{
			 		int num = strtoint(param[1+j]);
			 		if(prevInt[j] == 0)
			 		{
			 			prevInt[j] = num;
			 		}
			 		else
			 		{
			 			printf("%d\t", num - prevInt[j]);
			 			prevInt[j] = num;
			 		}
				}
				printf("\n");
				free(param);
				break;
			}
		}

		fclose(fd);

		long timctr = 0;
		long max = (long)sec*1000;
		int flag = 0;
		while(timctr < max)
		{
			timctr++;
			usleep(1000);
			c = '/';
			if(inputAvailable())
				read(STDIN_FILENO, &c, 1);
			if( c=='q')
			{
				printf("Quitting Process...\n");
				flag = 1;
				break;
			}
		}
		if(flag == 1)
			break;
	}
	return 0;
}

char **getCPUlist(char *line)
{
	char *field, **CPUlist;
	int Ind = 0;
	CPUlist = malloc(32 * sizeof(char*));

	field = strtok(line, " ");
	CPUlist[Ind++] = field;
	while((field = strtok(NULL, " ")) != NULL)
	{
		CPUlist[Ind++] = field;
	}
	CPUlist[Ind++] = NULL;
	int len = strlen(CPUlist[Ind-2]);
	CPUlist[Ind-2][len-1] = '\0';
	return CPUlist;
}

int inputAvailable()  
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

int strtoint(char input[])
{

	int i=0, num=0;
	while(input[i]!='\0')
	{
		num *= 10;
		int temp = input[i] - '0';
		num += temp;
		i++;
	}
	return num;
}

int check_numsec(char **Token)
{
	int ct = 0;
	char numstr[32];
	while(Token[ct]!=NULL)
		ct++;
	int i = 0;
	while(Token[i] != NULL && (strcmp(Token[i],"-n") != 0))
		i++;

	if(ct != i)
		return i;
	else
		return -1;
}

void reset_input_mode()
{
	tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode()
{
	struct termios tattr;
	char *name;

/* Make sure stdin is a terminal. */
	if (!isatty (STDIN_FILENO))
	{
		fprintf (stderr, "Not a terminal.\n");
		exit (EXIT_FAILURE);
	}

	tcgetattr (STDIN_FILENO, &saved_attributes);
	atexit (reset_input_mode);

	tcgetattr (STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON | ECHO);	
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}