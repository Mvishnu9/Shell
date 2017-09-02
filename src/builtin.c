#include "builtin.h"
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>

char *BuiltIn[] = { "cd", "pwd", "echo", "exit"};

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

int BuiltIn_checker(char **Token)
{
	int i, flag = 0,  BuiltInCount = sizeof(BuiltIn)/sizeof(char*);;
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
	return flag;
}