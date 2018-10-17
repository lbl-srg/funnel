
CC=gcc

.PHONY: algorithmRectangle.o readCSV.o tube.o tubeSize.o main.o funnel clean

algorithmRectangle.o: algorithmRectangle.c algorithmRectangle.h
	$(CC) -c algorithmRectangle.c

readCSV.o: readCSV.c readCSV.h
	$(CC) -c readCSV.c

tube.o: tube.c tube.h
	$(CC) -c tube.c

tubeSize.o: tubeSize.c tubeSize.h
	$(CC) -c tubeSize.c

mkdir_p.o: mkdir_p.c mkdir_p.h
	$(CC) -c mkdir_p.c

main.o: main.c data_structure.h readCSV.h algorithmRectangle.h tube.h tubeSize.h mkdir_p.h
	$(CC) -c main.c

build: algorithmRectangle.o readCSV.o tube.o tubeSize.o mkdir_p.o main.o
	$(CC) -o funnel algorithmRectangle.o readCSV.o tube.o tubeSize.o mkdir_p.o main.o
	mkdir -p ../bin
	mv funnel ../bin
	rm *.o

clean:
	rm -f *.o ../bin/funnel
