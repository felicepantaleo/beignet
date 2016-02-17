CC=g++ -O2

prova_felice: prova_felice.o
	$(CC) -o $@ $^ -lOpenCL

%.o: %.cpp
	$(CC) -o $@ $< -c 

clean:
	rm -f prova_felice *.o
