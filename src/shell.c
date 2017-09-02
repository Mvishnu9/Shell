#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <pwd.h>
#include <fcntl.h>

#include "systemc.h"
#include "builtin.h"
#include "ls.h"
#include "nightswatch.h"
#include "color.h"
#include "pinfo.h"

char *GetInput()
{
	char *rd = NULL;
	ssize_t siz = 0;
	getline(&rd, &siz, stdin);
	return rd;
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

int Execute(char **Token)
{
	int i;
	int flag = 0;
	flag = BuiltIn_checker(Token);
	if(flag == -1)
		return flag;
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