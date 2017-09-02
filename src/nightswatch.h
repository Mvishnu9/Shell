#ifndef nightswatch_INCLUDED
#define nightswatch_INCLUDED

int Handle_nightswatch(char **Token);
int Handle_n_dirty( char **Token, int sec);
int Handle_n_interrupt(char **Token, int sec);
int Recur_dirty(char **Token, int sec);
int Recur_interrupt(int CPUnum, int sec);
char **getCPUlist(char *line);
int inputAvailable();
int strtoint(char input[]);
int check_numsec(char **Token);
void reset_input_mode();
void set_input_mode();

#endif