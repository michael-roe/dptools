all: dpfoot

dpfoot: dpfoot.o footnote.o
	gcc -o dpfoot dpfoot.o footnote.o


dpfoot.o: dpfoot.c
	gcc -c dpfoot.c

footnote.o: footnote.c
	gcc -c footnote.c
