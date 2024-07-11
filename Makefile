Tcam2Sram : main.o
	g++ -o Tcam2Sram main.o

main.o : main.cpp
	g++ -c -g -Wall -DDEBUG main.cpp

.PHONY : clean
clean :
	rm -f Tcam2Sram *.o

.PHONY : dist-clean
dist-clean :
	rm -f Tcam2Sram *.o *.dat *.txt