CC=gcc
INCLUDE=-Iinc
SRCDIR=src
BINDIR=bin
OBJDIR=obj
DEPENDENCIES=
TESTSOBJDIRS="$(OBJDIR)\tests.o" "$(OBJDIR)\eval.o" "$(OBJDIR)\enum.o" "$(OBJDIR)\misc.o" "$(OBJDIR)\fsm.o" "$(OBJDIR)\cmbntn.o"
COMBSOBJDIRS="$(OBJDIR)\combs.o"
TESTSOBJS=tests.o eval.o enum.o misc.o fsm.o cmbntn.o
COMBSOBJS=combs.o
BINS=tests combs
CFLAGS=$(INCLUDE)

ifdef DEBUG
CFLAGS+=-g -DDEBUG
TESTSOBJDIRS+="$(OBJDIR)\debug.o"
COMBSOBJDIRS+="$(OBJDIR)\debug.o" "$(OBJDIR)\misc.o"
TESTSOBJS+=debug.o
COMBSOBJS+=debug.o misc.o
endif

all: $(BINS)

tests.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\tests.c" -o "$(OBJDIR)\tests.o"

combs.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\combs.c" -o "$(OBJDIR)\combs.o"

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

tests: $(TESTSOBJS)
	$(CC) $(LDFLAGS) $(TESTSOBJDIRS) -o "$(BINDIR)\tests.exe"

combs: $(COMBSOBJS)
	$(CC) $(LDFLAGS) $(COMBSOBJDIRS) -o "$(BINDIR)\combs.exe"

clean:
	del /F /Q "$(OBJDIR)\*.*" "$(BINDIR)\*.*"
