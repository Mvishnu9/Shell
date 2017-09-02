CC = gcc
CFLAGS = -lm

default: shell

shell: shell.o color.o ls.o pinfo.o systemc.o builtin.o nightswatch.o
	$(CC) -o shell shell.o color.o ls.o pinfo.o systemc.o builtin.o nightswatch.o $(CFLAGS) 

shell.o: shell.c color.h ls.h pinfo.h systemc.h builtin.h nightswatch.h
	$(CC) -c shell.c

color.o: color.c color.h
	$(CC) -c color.c

ls.o: ls.c ls.h
	$(CC) -c ls.c

pinfo.o: pinfo.c pinfo.h
	$(CC) -c pinfo.c

systemc.o: systemc.c systemc.h
	$(CC) -c systemc.c

builtin.o: builtin.c builtin.h
	$(CC) -c builtin.c

nightswatch.o: nightswatch.c nightswatch.h
	$(CC) -c nightswatch.c

clean:
	$(RM) shell *.o