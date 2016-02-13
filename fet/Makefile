
fet_objs=fet.o bus.o cpu_instr.o cpu_state.o

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic

fet: $(fet_objs)
	$(CC) $(CFLAGS) -o fet $(fet_objs)

$(fet_objs): fet.h

clean:
	rm -f $(fet_objs)
