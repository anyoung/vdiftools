CC = gcc
INC = 
LIBS = 
CFLAGS = $(INC)

DEPS = r2dbe_vdif.o vdif_files.o ioutils.o

.PHONY: all clean

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(DBG_ARGS)

testsg: testsg.o $(DEPS)
	$(CC) -o $@ $^ $(LIBS)

testf: testf.o $(DEPS)
	$(CC) -o $@ $^ $(LIBS)

all: testsg testf

clean:
	rm -f *.o
	rm -f testsg
	rm -f testf
