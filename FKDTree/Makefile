CFLAGS= -std=c++11 -ftree-vectorize -march=native -ffast-math -Ofast -march=native -mtune=native 

if [ ! -f /usr/include/CL/cl.h ]; then
	echo “No OpenCL Installed”
else
	CFLAGS+= DOPENCL
fi


CC=g++
NVCC=nvcc

        NVCC_RESULT := $(shell which nvcc 2> NULL)
        NVCC_TEST := $(notdir $(NVCC_RESULT))
ifeq ($(NVCC_TEST),nvcc)
	
	all: kdtree

	searchInTheBoxCuda.o:
		$(NVCC) -I/usr/local/cuda/include -L/usr/local/cuda/lib64 -lcuda -lcudart -c -arch=sm_20 searchInTheBoxCuda.cu

	kdtree: searchInTheBoxCuda.o
		$(CC) $(CFLAGS) -o kdtree -L/usr/local/cuda/lib64 -I/usr/local/cuda/include -lcuda -lcudart -lOpenCL -ltbb main.cpp 	searchInTheBoxCuda.o

	clean: rm -rf *o kdtree
else
	kdtree: main.cpp
		$(CC) $(CFLAGS) main.cpp -o kdtree -lOpenCL -ltbb

	clean:
		rm kdtree mem.log*
