
device_files:= $(wildcard dev_*.c)
device_objs:= $(device_files:.c=.o)
devtypes:= $(device_files:dev_%.c=%)


# This is a list of commands the names of which can not be deduced from
# the presence of cmd_*.c files. Add any command names here which are
# implemented in other files. It is also possible to include aliases,
# by prefixing them to the canonical name separated by colons, as
# alias1:alias2:canonical.

nonobvious_commands:= s:step g:go:run x:examine d:deposit l:load


command_files:= $(wildcard cmd_*.c)
command_objs:= $(command_files:.c=.o)
commands:= $(filter-out $(subst :, ,$(nonobvious_commands)), \
                        $(command_files:cmd_%.c=%)) \
           $(nonobvious_commands)

fet_objs:= fet.o bus.o cpu_instr.o cpu_state.o cmdlex.o util.o runcmd.o \
           devtypes.o $(device_objs) \
           commands.o $(command_objs)

CC:=gcc
CFLAGS:=-Wall -Wextra -std=c99 -pedantic -Wno-unused-parameter
LIBS:=-lreadline

fet: $(fet_objs)
	$(CC) $(CFLAGS) -o fet $(fet_objs) $(LIBS)

$(fet_objs): fet.h

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
	@printf '#include"fet.h"\n\n' > commands.c
	@( $(foreach cmd,$(commands),\
	     printf 'int cmd_%s(int, char **);\n' \
	            $(lastword $(subst :, ,$(cmd))); \
	   ) ) >> commands.c
	@printf '\ncommand commands[]={\n' >> commands.c
	@( $(foreach cmd,$(commands),\
	     cmd=$(cmd) ; \
	     func_name=$(lastword $(subst :, ,$(cmd))); \
	     for cmd_name in $$(echo $$cmd | tr : ' '); do \
	       printf '  {"%s", cmd_%s},\n' $$cmd_name $$func_name ; \
	     done ; ) ) >> commands.c
	@printf '  };\n' >> commands.c
	@printf 'int ncommands=$(words $(subst :, ,$(commands)));\n' \
	  >> commands.c


clean:
	rm -f $(fet_objs)
