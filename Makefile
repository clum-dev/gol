CFLAGS=-Wall -Werror -pedantic -std=gnu99 -g
.PHONY=all clean
.DEFAULT_GOAL:=all

OBJS_gol=gol.c clum-lib/file.o clum-lib/strings.o clum-lib/errors.o

all: gol

gol: $(OBJS_gol)
	gcc $(CFLAGS) $(OBJS_gol) -o gol

clum-lib/file.o: clum-lib/file.c clum-lib/file.h
	gcc $(CFLAGS) -o clum-lib/file.o -c clum-lib/file.c

clum-lib/strings.o: clum-lib/strings.c clum-lib/strings.h
	gcc $(CFLAGS) -o clum-lib/strings.o -c clum-lib/strings.c

clum-lib/errors.o: clum-lib/errors.c clum-lib/errors.h
	gcc $(CFLAGS) -o clum-lib/errors.o -c clum-lib/errors.c


clean:
	rm -f gol
	rm -f clum-lib/file.o
	rm -f clum-lib/strings.o
	rm -f clum-lib/errors.o
