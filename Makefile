all: HW2_9962173_Ser HW2_9962173_Cli clean

HW2_9962173_Ser: HW2_9962173_Ser.o
	cc -o HW2_9962173_Ser -lm HW2_9962173_Ser.o

HW2_9962173_Ser.o: HW2_9962173_Ser.c
	cc -o HW2_9962173_Ser.o -c HW2_9962173_Ser.c

HW2_9962173_Cli: HW2_9962173_Cli.o
	cc -o HW2_9962173_Cli -lm HW2_9962173_Cli.o
	
HW2_9962173_Cli.o: HW2_9962173_Cli.c
	cc -o HW2_9962173_Cli.o -c HW2_9962173_Cli.c

clean:
	rm -rf *.o


