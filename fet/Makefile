
device_files:= $(wildcard dev_*.c)
device_objs:= $(device_files:.c=.o)
devtypes:= $(device_files:dev_%.c=%)


# This is a list of commands the names of which can not be deduced from
# the presence of cmd_*.c files. Add any command names here which are
# implemented in other files. It is also possible to include aliases,
# by prefixing them to the canonical name separated by colons, as
# alias1:alias2:canonical. If the canonical name happens to also be
# non-obvious, add a colon to the end to include the canonical name too.

nonobvious_commands:= cpu s:step g:go:run


command_files:= $(wildcard cmd_*.c)
command_objs:= $(command_files:.c=.o)
commands:= $(command_files:cmd_%.c=%) $(nonobvious_commands)

fet_objs:= fet.o bus.o cpu_instr.o cpu_state.o cmdlex.o util.o \
           devtypes.o $(device_objs) \
           commands.o $(command_objs)

CC:=gcc
CFLAGS:=-Wall -Wextra -std=c99 -pedantic -Wno-unused-parameter
LIBS:=-lreadline

fet: $(fet_objs)
	$(CC) $(CFLAGS) -o fet $(fet_objs) $(LIBS)

$(fet_objs): fet.h

$(command_objs) fet.o: commands.h

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
	   printf 'int cmd_%s(char **);\n' $${cmd##*:} ; \
	 done >> commands.c
	@printf '\ncommand commands[]={\n' >> commands.c
	@( $(foreach cmd,$(commands),\
	     cmd=$(cmd) ; \
	     names=$${cmd%:*} ; \
	     func_name=$(lastword $(subst :, ,$(cmd))); \
	     for cmd_name in $$(echo $$names | tr : ' '); do \
	       printf '  {"%s", cmd_%s},\n' $$cmd_name $$func_name ; \
	     done ; ) ) >> commands.c
	@printf '  };\nint ncommands=$(words $(commands));\n' >> commands.c


clean:
	rm -f $(fet_objs)
