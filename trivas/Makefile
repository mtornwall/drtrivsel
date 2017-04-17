

trivas_objs:= trivas.o

CC:=gcc
CFLAGS:=-Wall -Wextra -std=c99 -pedantic -Wno-unused-parameter
LIBS:=

trivas: $(trivas_objs)
	$(CC) $(CFLAGS) -o trivas $(trivas_objs) $(LIBS)

$(trivas_objs): trivas.h

clean:
	rm -f $(trivas_objs)
