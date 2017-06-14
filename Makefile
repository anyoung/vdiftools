CC = gcc
INC = 
LIBS = 
CFLAGS = -g $(INC)

DEPS = r2dbe_vdif.o vdif_files.o vdif_frames.o ioutils.o

.PHONY: all clean

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(DBG_ARGS)

testsg: testsg.o $(DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

testf: testf.o $(DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

all: testsg testf

clean:
	rm -f *.o
	rm -f testsg
	rm -f testf
