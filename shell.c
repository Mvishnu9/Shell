#include <syscall.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>

char *GetInput()
{
	char *read = NULL;
	ssize_t siz = 0;
	getline(&read, &siz, stdin);
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
	return TokArr;
}

int Execute(char **Token)
{
	return 0;

}

int main_loop()
{
	struct passwd *pass;
	pass = getpwuid(getuid());
	char *name = pass->pw_name;
	char hname[32];
	gethostname(hname, 32);
	int breaker = 1;
	do
	{
		int ret;
		char *Inp, **Tok;
		char CurrAbsPath[256], CurrPath[256];
		getcwd(CurrAbsPath, 256);
		printf("<%s@%s: %s> ",name, hname, CurrAbsPath);
		Inp = GetInput();
		Tok = TokenizeInp(Inp);
		ret = Execute(Tok);
		
		breaker = 0;
		free(Tok);
	}while(breaker == 1);
	
	return 0;
}

int main(int argc, char *argv[])
{
	main_loop();

	exit(0);
}
