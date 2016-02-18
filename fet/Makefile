
devices= mem uart
device_objs= $(addprefix dev_,$(addsuffix .o,$(devices)))

commands= help map
command_objs= $(addprefix cmd_,$(addsuffix .o,$(commands)))

fet_objs= fet.o bus.o cpu_instr.o cpu_state.o \
          devices.o $(device_objs) \
          commands.o $(command_objs)

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Wno-unused-parameter
LIBS=-lreadline

fet: $(fet_objs)
	$(CC) $(CFLAGS) -o fet $(fet_objs) $(LIBS)

$(fet_objs): fet.h

$(command_objs) fet.o: commands.h

#devices.o: devices.c
#	$(CC) $(CFLAGS) -c -o devices.o devices.c

.PHONY: devices.c
devices.c:
	@echo Generating devices.c with: $(devices)
	@printf '#include"fet.h"\n\n' > devices.c
	@for dev in $(devices); do \
	   printf 'extern device dev_%s;\n' $$dev ; \
	 done >> devices.c
	@printf '\ndevice *devices[]={\n' >> devices.c
	@for dev in $(devices); do \
	   printf '  &dev_%s,\n' $$dev ; \
	 done >> devices.c
	@printf '  };\nint ndevices=$(words $(devices));\n' >> devices.c

.PHONY: commands.c
commands.c:
	@echo Generating commands.c with: $(commands)
	@printf '#include"commands.h"\n\n' > commands.c
	@for cmd in $(commands); do \
	   printf 'int cmd_%s(char **);\n' $$cmd ; \
	 done >> commands.c
	@printf '\ncommand commands[]={\n' >> commands.c
	@for cmd in $(commands); do \
	   printf '  {"%s", cmd_%s},\n' $$cmd $$cmd ; \
	 done >> commands.c
	@printf '  };\nint ncommands=$(words $(commands));\n' >> commands.c


clean:
	rm -f $(fet_objs)
