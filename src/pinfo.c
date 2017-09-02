#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <stdlib.h>
#include "pinfo.h"


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

