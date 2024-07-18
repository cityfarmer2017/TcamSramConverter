Tcam2Sram : main.o match_action_id.o
	g++ -o Tcam2Sram main.o match_action_id.o

main.o : main.cpp match_action_id.h
	g++ -c -g -Wall main.cpp

match_action_id.o : match_action_id.cpp
	g++ -c -g -Wall match_action_id.cpp

.PHONY : clean
clean :
	rm -f Tcam2Sram *.o

.PHONY : dist-clean
dist-clean :
	rm -f Tcam2Sram *.o *.dat *.txt