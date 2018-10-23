
CC=gcc
CFLAGS=-Wall -c

LIB=libfunnel.so

SRCS=$(foreach fil,*.c, $(fil))
OBJS = algorithmRectangle.o compare.o main.o mkdir_p.o readCSV.o tube.o tubeSize.o


%.o : %.o
	$(CC) -c $(CFLAGS) $< -o $@

test:
	echo $(SRCS)
	echo $(OBJS)

compile:
	$(CC) $(CC_FLAGS_$(ARCH)) -fPIC -c $(SRCS)

build: compile
	$(CC) -shared -fPIC -Wl,-soname,$(LIB) -o $(LIB) $(OBJS) -lc
	mkdir -p ../lib64
	mv $(LIB) ../lib64
	$(CC) -o funnel $(OBJS)
	mkdir -p ../bin
	mv funnel ../bin
	rm *.o

clean:
	rm -f *.o ../bin/funnel ../lib64/$(LIB)
