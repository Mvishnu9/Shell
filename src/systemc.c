#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>

#include "systemc.h"

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