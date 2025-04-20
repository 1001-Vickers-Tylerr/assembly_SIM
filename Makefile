project: driver.o functions.o
	g++ -o project driver.o functions.o

driver.o: driver.cpp functions.h
	g++ -c driver.cpp -g

functions.o: functions.cpp functions.h
	g++ -c functions.cpp -g

clean:
	rm *.o project