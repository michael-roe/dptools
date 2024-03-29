all: dpfoot dphtml dptxt dpcomments

dphtml: dphtml.o output.o translit.o entity.o
	gcc -o dphtml dphtml.o output.o translit.o entity.o

dptxt: dptxt.o entity.o
	gcc -o dptxt dptxt.o entity.o

dpfoot: dpfoot.o footnote.o
	gcc -o dpfoot dpfoot.o footnote.o

dpcomments: dpcomments.o
	gcc -o dpcomments dpcomments.o

dpfoot.o: dpfoot.c
	gcc -c dpfoot.c

footnote.o: footnote.c
	gcc -c footnote.c

entity.o: entity.c entity.h
	gcc -c entity.c

translit.o: translit.c dptools.h
	gcc -c translit.c

output.o: output.c dptools.h
	gcc -c output.c

dptxt.o: dptxt.c
	gcc -c dptxt.c

dpcomments.o: dpcomments.c
	gcc -c dpcomments.c
