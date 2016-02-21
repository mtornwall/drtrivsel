
devtypes= mem uart
device_objs= $(addprefix dev_,$(addsuffix .o,$(devtypes)))

commands= help map
command_objs= $(addprefix cmd_,$(addsuffix .o,$(commands)))

fet_objs= fet.o bus.o cpu_instr.o cpu_state.o cmdlex.o \
          devtypes.o $(device_objs) \
          commands.o $(command_objs)

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Wno-unused-parameter
LIBS=-lreadline

fet: $(fet_objs)
	$(CC) $(CFLAGS) -o fet $(fet_objs) $(LIBS)

$(fet_objs): fet.h

$(command_objs) fet.o: commands.h

#devtypes.o: devtypes.c
#	$(CC) $(CFLAGS) -c -o devtypes.o devtypes.c

.PHONY: devtypes.c
devtypes.c:
	@echo Generating devtypes.c with: $(devtypes)
	@printf '#include"fet.h"\n\n' > devtypes.c
	@for dev in $(devtypes); do \
	   printf 'extern device dev_%s;\n' $$dev ; \
	 done >> devtypes.c
	@printf '\ndevice *devtypes[]={\n' >> devtypes.c
	@for dev in $(devtypes); do \
	   printf '  &dev_%s,\n' $$dev ; \
	 done >> devtypes.c
	@printf '  };\nint ndevtypes=$(words $(devtypes));\n' >> devtypes.c

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
