#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>
#include <dirent.h>

#include "ls.h"

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

char *formatdate(char* str, time_t val)
{
    strftime(str, 36, "%d.%m.%Y %H:%M:%S", localtime(&val));
    return str;
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
