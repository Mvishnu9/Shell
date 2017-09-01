#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include <sys/select.h>

#define RESET	0
#define BRIGHT 	1
#define RED	 	1
#define GREEN	2
#define YELLOW	3
#define BLUE	4
#define MAGENTA	5
#define	WHITE	7

char *BuiltIn[] = { "cd", "pwd", "echo", "exit"};
int prevInt[16];
struct termios saved_attributes;


char *GetInput()
{
	char *rd = NULL;
	ssize_t siz = 0;
	getline(&rd, &siz, stdin);
	return rd;
}

char *formatdate(char* str, time_t val)
{
    strftime(str, 36, "%d.%m.%Y %H:%M:%S", localtime(&val));
    return str;
}

char **TokenizeInp(char *Inp)
{
	char *token, **TokArr;
	int TokInd = 0;
	TokArr = malloc(32 * sizeof(char*));

	token = strtok(Inp, " ");
	TokArr[TokInd++] = token;

	while((token = strtok(NULL, " ")) != NULL)
	{
		TokArr[TokInd++] = token;
	}
	TokArr[TokInd++] = NULL;
	int len = strlen(TokArr[TokInd-2]);
	TokArr[TokInd-2][len-1] = '\0';
	return TokArr;
}

char *GetPermissionString(struct stat perm)
{	
	static char PermissionString[11];
	strcpy(PermissionString, "----------");

	if(S_ISDIR(perm.st_mode))
		PermissionString[0] = 'd';
	if(perm.st_mode & S_IRUSR)
		PermissionString[1] = 'r';
	if(perm.st_mode & S_IWUSR)
		PermissionString[2] = 'w';
	if(perm.st_mode & S_IXUSR)
		PermissionString[3] = 'x';
	if(perm.st_mode & S_IRGRP)
		PermissionString[4] = 'r';
	if(perm.st_mode & S_IWGRP)
		PermissionString[5] = 'w';
	if(perm.st_mode & S_IXGRP)
		PermissionString[6] = 'x';
	if(perm.st_mode & S_IROTH)
		PermissionString[7] = 'r';
	if(perm.st_mode & S_IWOTH)
		PermissionString[8] = 'w';
	if(perm.st_mode & S_IXOTH)
		PermissionString[9] = 'x';
	return PermissionString;

}

int HandleBuiltIn(int Ind, char **Token)
{
	if(strcmp(BuiltIn[Ind],"cd") == 0)
	{
		char dest[256], Fin[256], *first;
		strcpy(Fin, Token[1]);
		strcpy(dest, Token[1]);
		first = strtok(dest, "/");
		if(strcmp(first, "~") == 0)
		{
			struct passwd *pw = getpwuid(getuid());
			char *homedir= pw->pw_dir, temp[256];
			strcpy(temp, homedir);
			int TempL = strlen(temp);
			temp[TempL] = '/';
			temp[TempL+1] = '\0';
			while((first = strtok(NULL, "/")) != NULL)
			{
				strcat(temp,first);
				TempL = strlen(temp);
				temp[TempL] = '/';
				temp[TempL+1] = '\0';
			}
			strcpy(Fin,temp);
		}
		if(chdir(Fin) != 0)
			perror("Error");
	}
	else if(strcmp(BuiltIn[Ind],"exit") == 0)
	{
		return -1;
	}
	else if(strcmp(BuiltIn[Ind], "echo") == 0)
	{
		int i=1;
		char out[2048]="";
		while(Token[i] != NULL)	
		{
			strcat(out,Token[i]);
			strcat(out," ");
			i++;
		}
		strcat(out,"\n");
		write(STDOUT_FILENO, &out, strlen(out));
		return 0;
	}
	else if(strcmp(BuiltIn[Ind], "pwd") == 0)
	{
		char CurrPath[256];
		getcwd(CurrPath,256);
		printf("%s\n",CurrPath);
	}
	return 0;
}

int Handle_ls(char **Token)
{
	char path[512]="";
	char ArgPath[512]= "";
	struct dirent *FileName;
	DIR *CurrDir;
	getcwd(path, 512);
	int ct=0, i, flags = 0; 			// Flags-> stores Cumulative flag variable
	while(Token[ct] != NULL)
		ct++;
	for(i=1; i<ct; i++)
	{
		if(Token[i][0] == '-')
		{
			int len = strlen(Token[i]);
			int j = 0;
			for(j=1; j<len; j++)
			{
				if(Token[i][j] == 'l')
					flags += 10;
				else if(Token[i][j] == 'a')
					flags += 1;
			}
		}
		else
		{
			strcat(path,"/");
			if(Token[i][0] != '/')
			{
				if(Token[i][0] != '~')
					strcat(path, Token[i]);
				else
				{
					struct passwd *temp = getpwuid(getuid());
					const char *homedir = temp->pw_dir;
					strcpy(path, homedir);
					if(Token[i][1]!= '\0')
						strcat(path,Token[i]+1);
				}
			}
			else
				strcpy(path, Token[i]);
		}
	}

	CurrDir = opendir(path);
	while((FileName = readdir(CurrDir)) != NULL)
	{
		char *buf;
		buf = FileName->d_name;

		if(flags%10 == 0)
		{
			if(buf[0] != '.')
			{
				if(flags < 10)
					printf("%s\n", buf);
				else
				{
					struct stat file_stat;
					char filePath[512] = "";
					strcpy(filePath, path);
					strcat(filePath, "/");
					strcat(filePath, buf);
					if(stat(filePath, &file_stat) == 0)
					{
						struct passwd *pwuser = getpwuid(file_stat.st_uid);
						struct group *gruser = getgrgid(file_stat.st_gid);
						printf("%s\t%d %s\t%s\t",GetPermissionString(file_stat), file_stat.st_nlink, pwuser->pw_name, gruser->gr_name);
						char date[36];

						char temp[36];
						strcpy(temp, formatdate(date, file_stat.st_mtime));
						printf("%ld\t%s\t%s\n", file_stat.st_size, temp, buf);
					}
				}
			}
		}
		else
		{
			if(flags < 10)
					printf("%s\n", buf);
			else
			{
				struct stat file_stat;
				char filePath[512] = "";
				strcpy(filePath, path);
				strcat(filePath, "/");
				strcat(filePath, buf);
				if(stat(filePath, &file_stat) == 0)
				{
					struct passwd *pwuser = getpwuid(file_stat.st_uid);
					struct group *gruser = getgrgid(file_stat.st_gid);
					printf("%s\t%d %s\t%s\t",GetPermissionString(file_stat), file_stat.st_nlink, pwuser->pw_name, gruser->gr_name);
					char date[36];
					char temp[36];
					strcpy(temp, formatdate(date, file_stat.st_mtime));
					printf("%ld\t%s\t%s\n", file_stat.st_size, temp, buf);
				}
			}
		}
	}
	closedir(CurrDir);
	return 0;

}


char *ReadFileLine(char *Filename, char *search)
{
	static char out[128];
	FILE * fd;
	char * line = NULL;
	size_t len = 0;
	ssize_t rd;
	fd = fopen(Filename, "r");
	if (fd == NULL)
		perror("Error");
	while((rd = getline(&line, &len, fd)) != -1)
	{
		if(strstr(line, search))
		{
			strcpy(out, line);
			break;
		}

	}
	fclose(fd);
	if(line)
		free(line);
	return out;
}

char *Readsym(char *filename)
{
	static char out[128];
	memset(out, 0, sizeof(out));

	if(readlink(filename, out, sizeof(out) - 1) < 0)
	{
		perror("Error");
	}
	return out;
}

int Handle_pinfo(char **Token)
{
	int ct = 0;
	int pid = getpid();
	char Proc_statF[512], Proc_exe[512], Proc_Dir[512], pid_string[16], state[128], exe_path[256], VmSize[128];
	sprintf(pid_string, "%d", pid);

	while(Token[ct] != NULL)
		ct++;

	if(ct == 2)
	{
		char temp[512];

		strcpy(temp, "/proc/");	
		strcat(temp, Token[1]);
		strcat(temp, "/");
		DIR *dir = opendir(temp);
		if (dir)
		{	
			strcpy(pid_string, Token[1]);
			closedir(dir);
		}
		else
		{
			perror("Process Doesnt exist");
			return -1;
		}

	}

	int size = (int)((ceil(log10(pid))+1)*sizeof(char));

	strcpy(Proc_Dir, "/proc/");	
	strcat(Proc_Dir, pid_string);
	strcat(Proc_Dir, "/");

	strcpy(Proc_statF, Proc_Dir);
	strcat(Proc_statF, "status");
	strcpy(state, ReadFileLine(Proc_statF, "State"));
	strcpy(VmSize, ReadFileLine(Proc_statF, "VmSize"));

	strcpy(Proc_exe, Proc_Dir);
	strcat(Proc_exe, "exe");
	strcpy(exe_path, Readsym(Proc_exe));

	printf("PID -- %s\n", pid_string);
	printf("%s", state);
	printf("%s", VmSize);
	printf("Executable path -- %s\n", exe_path);
	return 0;

}

void Handle_childSIG(int Sig)
{
    pid_t childpid;
    int childstatus;
    while ((childpid = waitpid( -1, &childstatus, WNOHANG)) > 0)
    {
        if (WIFEXITED(childstatus))
        {
            printf("\nPID %d exited normally.\n", childpid);
        }
        else
        {
            if (WIFSTOPPED(childstatus))
            {
                printf("\nPID %d was stopped by %d\n", childpid, WSTOPSIG(childstatus));
            }
            else
            {
                if (WIFSIGNALED(childstatus))
                {
                    printf("\nPID %d exited due to signal %d\n", childpid, WTERMSIG(childstatus));
                }
                else
                {
                    perror("\nERROR in waitpid");
                }
            }
        }
    }
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

int Recur_interrupt(char **Token, int CPUnum, int sec)
{
	int  k = 0;
	char c;
	
	while(1)
	{
		k++;
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

/* Save the terminal attributes so we can restore them later. */
	tcgetattr (STDIN_FILENO, &saved_attributes);
	atexit (reset_input_mode);

/* Set the funny terminal modes. */
	tcgetattr (STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON | ECHO);	/* Clear ICANON and ECHO. */
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
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
	char c, password[10000];
	set_input_mode ();
	pid_t pid, wpid;
	pid = fork();
	if(pid == 0)
	{
		Recur_interrupt(Token, i-1, sec);
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

	}
	return 0;
}

int systemcommand(char **Token)
{
	int ct = 0, status;
	pid_t pid, wpid;
	while(Token[ct] != NULL)
		ct++;
	if(strcmp(Token[ct-1],"&") != 0)
	{
		signal(SIGCHLD, Handle_childSIG);
		pid = fork();
		if(pid == 0)
		{
			if(execvp(Token[0], Token) == -1)
			{
				perror("EXECVP ERROR");
			}
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
	}
	else
	{
		Token[ct - 1] = NULL;
		pid = fork();
		if(pid == 0)
		{
			FILE *fd;
			fclose(stdin); 
			fd = fopen("/dev/null", "r"); 
			if(strcmp(Token[0],"vim") == 0)
			{
				fopen(STDIN_FILENO, "w");
				fclose(fd);
			}

			if(execvp(Token[0], Token) == -1)
			{
				perror("EXECVP ERROR");
			}
			exit(1);
		}
		else if (pid < 0)
		{
			perror("FORK ERROR");
		}
		else
		{
			printf("Child process in Background\n");
		}
	}	
	return 0;
}

int Execute(char **Token)
{
	int i, BuiltInCount = sizeof(BuiltIn)/sizeof(char*);
	int flag = 0;
	for(i=0; i<BuiltInCount; i++)
	{
		if(strcmp(BuiltIn[i],Token[0]) == 0)
		{
			flag = 1;
			int ret = HandleBuiltIn(i, Token);
			if(ret == -1)
				return ret;
		}
	}
	if(flag == 0)
	{
		if(strcmp(Token[0],"ls") == 0)
		{
		 	int ct = 0, ret;
		 	while(Token[ct]!= NULL)
		 		ct++;
		 	if(strcmp(Token[ct-1],"&") != 0)
		 	{
		 		ret = Handle_ls(Token);
		 	}
		 	else
		 	{
		 		Token[ct -1] = NULL;
		 		signal(SIGCHLD, Handle_childSIG);
				pid_t pid = fork();
				if(pid == 0)
				{
					fclose(stdin); 
					fopen("/dev/null", "r"); 
					Handle_ls(Token);
					exit(1);
				}
				else if (pid < 0)
				{
					perror("FORK ERROR");
				}
				else
				{
					printf("Child process in Background\n");
				}
		 	}
		}
		else if(strcmp(Token[0],"pinfo") == 0)
		{
			Handle_pinfo(Token);
		}
		else if(strcmp(Token[0],"nightswatch") == 0)
		{
			Handle_nightswatch(Token);
		}
		else
		{
			systemcommand(Token);
		}
	}
	return 0;
}

void textcolor(int attr, int fg)
{	char command[13];

	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, 40);
	printf("%s", command);
}

int main_loop()
{
	struct passwd *pass;
	pass = getpwuid(getuid());
	char *name = pass->pw_name;
	char hname[32];
	gethostname(hname, 32);
	int ret = 1;
	do
	{
		char *Inp, **Tok;
		char CurrAbsPath[256], CurrPath[256];
		getcwd(CurrAbsPath, 256);
		printf("<");
		textcolor(BRIGHT, RED);
		printf("%s@%s:",name, hname);
		textcolor(BRIGHT, YELLOW);
		printf("%s" , CurrAbsPath);
		textcolor(RESET, WHITE);
		printf("> ");
		Inp = GetInput();
		Tok = TokenizeInp(Inp);
		if(strlen(Tok[0]) == 0)
			continue;
		ret = Execute(Tok);
		
		free(Tok);
	}while(ret != -1);
	return 0;
}

int main(int argc, char *argv[])
{
	main_loop();
	exit(0);
}