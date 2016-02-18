CFLAGS= -std=c++11 -ftree-vectorize -march=native -ffast-math -Ofast -march=native -mtune=native
LIBS = -ltbb

CUDAOBJECT =
USE_CUDA = YES
USE_OPENCL = YES

CC=g++

###CUDA TEST
NVCC_RESULT := $(shell which nvcc 2> NULL)
NVCC_TEST := $(notdir $(NVCC_RESULT))


ifeq ($(USE_CUDA),YES)

ifeq ($(NVCC_TEST),nvcc)

    CUDALIBS = -lcuda -lcudart -L/usr/local/cuda/lib64
    CUDAFLAGS = -D__USE_CUDA__
    CUDACFLAGS = -std=c++11 -ftree-vectorize -march=native -ffast-math -Ofast -march=native -mtune=native
    CUDAHEADERS = -I/usr/local/cuda/include

	searchInTheBoxCuda.o: searchInTheBoxCuda.cu
		$(NVCC) $(CUDAHEADERS) -c -XCompiler $(CUDACFLAGS) searchInTheBoxCuda.cu

    CUDAOBJECT = searchInTheBoxCuda.o

endif

endif

###OPENCL TEST
ifeq ($(USE_OPENCL),YES)
    OPENCLLIBS = -lOpenCL
    OPENCLFLAGS = -D__USE_OPENCL__
    OPENCLHEADERS =

endif

    all: kdtree

    kdtree: $(CUDAOBJECT)
    $(CC) $(CFLAGS) $(CUDAFLAGS) $(OPENCLFLAGS)  -o kdtree $(CUDAHEADERS) $(LIBS) $(CUDALIBS) $(OPENCLLIBS) main.cpp 	searchInTheBoxCuda.o

    clean: rm -rf *.o kdtree



