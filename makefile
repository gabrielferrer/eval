CC=gcc
INCLUDE=-Iinc
SRCDIR=src
BINDIR=bin
OBJDIR=obj
DEPENDENCIES=
OBJDIRS="$(OBJDIR)\tests.o" "$(OBJDIR)\eval.o" "$(OBJDIR)\enum.o" "$(OBJDIR)\misc.o" "$(OBJDIR)\fsm.o" "$(OBJDIR)\cmbntn.o"
OBJS=tests.o eval.o enum.o misc.o fsm.o cmbntn.o
CFLAGS=$(INCLUDE)

ifdef DEBUG
CFLAGS+=-g -DDEBUG
OBJDIRS+="$(OBJDIR)\debug.o"
OBJS+=debug.o
endif

tests: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJDIRS) -o "$(BINDIR)\tests.exe"

tests.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\tests.c" -o "$(OBJDIR)\tests.o"

eval.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\eval.c" -o "$(OBJDIR)\eval.o"

enum.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\enum.c" -o "$(OBJDIR)\enum.o"
	
misc.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\misc.c" -o "$(OBJDIR)\misc.o"

fsm.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\fsm.c" -o "$(OBJDIR)\fsm.o"

cmbntn.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\cmbntn.c" -o "$(OBJDIR)\cmbntn.o"

debug.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\debug.c" -o "$(OBJDIR)\debug.o"

clean:
	del /F /Q "$(OBJDIR)\*.*" "$(BINDIR)\tests.exe"
