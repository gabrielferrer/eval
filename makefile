CC=gcc
INCLUDE=-Iinc
SRCDIR=src
BINDIR=bin
OBJDIR=obj
DEPENDENCIES=
OBJS="$(OBJDIR)\tests.o" "$(OBJDIR)\eval.o" "$(OBJDIR)\enum.o"
CFLAGS=$(INCLUDE)

ifdef DEBUG
CFLAGS+=-g
endif

tests: tests.o eval.o enum.o
	$(CC) $(LDFLAGS) $(OBJS) -o "$(BINDIR)\tests.exe"

tests.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\tests.c" -o "$(OBJDIR)\tests.o"

eval.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\eval.c" -o "$(OBJDIR)\eval.o"

enum.o:
	$(CC) $(CFLAGS) -c "$(SRCDIR)\enum.c" -o "$(OBJDIR)\enum.o"

clean:
	del /F /Q "$(OBJDIR)\*.*" "$(BINDIR)\tests.exe"
