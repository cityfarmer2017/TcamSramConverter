Tcam2Sram : main.o utility.o
	g++ -o Tcam2Sram main.o utility.o

main.o : main.cpp utility.h
	g++ -c -g -Wall main.cpp

utility.o : utility.cpp
	g++ -c -g -Wall utility.cpp

.PHONY : clean
clean :
	rm -f Tcam2Sram *.o

.PHONY : dist-clean
dist-clean :
	rm -f Tcam2Sram *.o *.dat *.txt