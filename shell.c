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

char TokenizeInp(char *Inp)
{
	char *token;
	token = strtok(Inp, " ");
	printf("%s\n",token);
	while((token = strtok(NULL, " ")) != NULL)
		printf("%s\n", token);
	return 'c';
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
		char *Inp, Tok;
		char CurrAbsPath[256], CurrPath[256];
		getcwd(CurrAbsPath, 256);
		printf("<%s@%s: %s> ",name, hname, CurrAbsPath);
		Inp = GetInput();
		Tok = TokenizeInp(Inp);
		printf("\n%s\n", Inp);
		breaker = 0;
	}while(breaker == 1);
	
	return 0;
}

int main(int argc, char *argv[])
{
	main_loop();

	exit(0);
}
