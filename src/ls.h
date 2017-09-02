#ifndef ls_INCLUDED
#define ls_INCLUDED

int Handle_ls(char **Token);
char *GetPermissionString(struct stat perm);
char *formatdate(char* str, time_t val);

#endif