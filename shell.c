#include <syscall.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>

char *BuiltIn[] = { "cd", "pwd", "echo", "exit"};

char *GetInput()
{
	char *read = NULL;
	ssize_t siz = 0;
	getline(&read, &siz,stdin);
	return read;
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

int Execute(char **Token)
{
	int i, BuiltInCount = sizeof(BuiltIn)/sizeof(char*);
	for(i=0; i<BuiltInCount; i++)
	{
//		printf("%s\n", BuiltIn[i]);
//		printf("%s\n", Token[0]);
		if(strcmp(BuiltIn[i],Token[0]) == 0)
		{
//			printf("in Builtin part\n");
			int ret = HandleBuiltIn(i, Token);
			if(ret == -1)
				return ret;
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
		printf("<%s@%s: %s> ",name, hname, CurrAbsPath);
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
