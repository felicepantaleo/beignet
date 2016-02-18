CFLAGS= -std=c++11 -ftree-vectorize -march=native -ffast-math -Ofast -march=native -mtune=native 

CC=g++
NVCC=nvcc

all: kdtree

kdtree: searchInTheBoxCuda.o
	$(CC) $(CFLAGS) -o kdtree -L/usr/local/cuda/lib64 -I/usr/local/cuda/include -lcuda -lcudart main.cpp searchInTheBoxCuda.o

searchInTheBoxCuda.o:
	$(NVCC) -I/usr/local/cuda/include -L/usr/local/cuda/lib64 -lcuda -lcudart -c -arch=sm_20 searchInTheBoxCuda.cu

clean: rm -rf *o kdtree


#kdtree: main.cpp
#	$(CC) $(CFLAGS) main.cpp -o kdtree -lOpenCL -ltbb

#clean:
#	rm kdtree mem.log*
