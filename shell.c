//#include <syscall.h>
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

#define RESET		0
#define BRIGHT 		1
#define RED			1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define	WHITE		7

char *BuiltIn[] = { "cd", "pwd", "echo", "exit"};

char *GetInput()
{
	char *read = NULL;
	ssize_t siz = 0;
	getline(&read, &siz, stdin);
	return read;
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
	// static char PermissionString[11];
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
//	printf("In builtin\n");
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
//			strcat(temp, dest);
//			printf("%s\n",temp);
			strcpy(Fin,temp);
		}
		if(chdir(Fin) != 0)
			perror("Error");
	}
	else if(strcmp(BuiltIn[Ind],"exit") == 0)
	{
//		printf("In exit part");
		return -1;
	}
	else if(strcmp(BuiltIn[Ind], "echo") == 0)
	{
		int i=1;
		char out[1024]="";
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
	ssize_t read;
	fd = fopen(Filename, "r");
	if (fd == NULL)
		perror("Error");
	while((read = getline(&line, &len, fd)) != -1)
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


int Execute(char **Token)
{
	int i, BuiltInCount = sizeof(BuiltIn)/sizeof(char*);
	for(i=0; i<BuiltInCount; i++)
	{
		if(strcmp(BuiltIn[i],Token[0]) == 0)
		{
			int ret = HandleBuiltIn(i, Token);
			if(ret == -1)
				return ret;
		}
	}
	if(strcmp(Token[0],"ls") == 0)
	{
		int ret = Handle_ls(Token);
	}
	else if(strcmp(Token[0],"pinfo") == 0)
	{
		Handle_pinfo(Token);
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
