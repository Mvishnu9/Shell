#include <stdio.h>
#include "color.h"



void textcolor(int attr, int fg)
{	char command[13];

	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, 40);
	printf("%s", command);
}
