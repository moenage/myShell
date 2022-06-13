all: my_cshell

my_cshell:
	gcc -Wall -o cshell cshell.c

clean:
	$(RM) my_cshell