##########################################
# Makefile to build project
#########################################

.PHONY: algorithmRectangle.o readCSV.o tube.o tubeSize.o main.o funnel clean

algorithmRectangle.o: algorithmRectangle.c algorithmRectangle.h
	gcc -c algorithmRectangle.c

readCSV.o: readCSV.c readCSV.h
	gcc -c readCSV.c

tube.o: tube.c tube.h
	gcc -c tube.c

tubeSize.o: tubeSize.c tubeSize.h
	gcc -c tubeSize.c

main.o: main.c data_structure.h readCSV.h algorithmRectangle.h tube.h tubeSize.h
	gcc -c main.c

funnel: algorithmRectangle.o readCSV.o tube.o tubeSize.o main.o
	gcc -o funnel algorithmRectangle.o readCSV.o tube.o tubeSize.o main.o

clean:
	rm *.o funnel

