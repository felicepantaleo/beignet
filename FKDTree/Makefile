CFLAGS= -std=c++11 -ftree-vectorize -march=native -ffast-math -Ofast -march=native -mtune=native
LIBS = -ltbb

CUDAOBJECT =
USE_CUDA = NO
USE_OPENCL = YES

CC=g++

###CUDA TEST
NVCC_RESULT := $(shell which nvcc 2> NULL)
NVCC_TEST := $(notdir $(NVCC_RESULT))


ifeq ($(USE_CUDA),YES)

ifeq (!$(NVCC_TEST),nvcc)

    CUDALIBS = -lcuda -lcudart -L/usr/local/cuda/lib64
    CUDAFLAGS = -D__USE_CUDA__
    CUDACFLAGS = -std=c++11 -ftree-vectorize -march=native -ffast-math -Ofast -march=native -mtune=native
    CUDAHEADERS = -I/usr/local/cuda/include

	searchInTheBoxCuda.o: searchInTheBoxCuda.cu
		$(NVCC) $(CUDAHEADERS) -c -XCompiler $(CUDACFLAGS) searchInTheBoxCuda.cu

    CUDAOBJECT = searchInTheBoxCuda.o
else
	all:
		$(error Unable to provide CUDA support. NVCC missing.)
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
	$(CC) main.cpp $(CFLAGS) $(CUDAFLAGS) $(OPENCLFLAGS) -o kdtree $(CUDAHEADERS) $(CUDALIBS) $(LIBS) $(OPENCLLIBS) $(CUDAOBJECT)

clean:
	rm -rf *.o kdtree



