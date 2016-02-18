#include "FKDTree.h"
#include "KDPoint.h"
#include <chrono>
#include "KDTreeLinkerAlgoT.h"
#include <sstream>
#include <unistd.h>
#include <thread>

#include <string.h>
#include <CL/cl.h>
#include "cl_helper.h"
#define __CL_ENABLE_EXCEPTIONS
#include <fstream>

#include <stdlib.h>
#include <sys/time.h>

double shrDeltaT()
{
#if defined(_WIN32)
	static LARGE_INTEGER old_time;
	LARGE_INTEGER new_time, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&new_time);
	const double DeltaT = ((double)new_time.QuadPart - (double)old_time.QuadPart) / (double)freq.QuadPart;
	old_time = new_time;
	return DeltaT;
#elif defined(__unix__)
	static struct timeval old_time;
	struct timeval new_time;
	gettimeofday(&new_time, NULL);
	const double DeltaT = ((double) new_time.tv_sec
			+ 1.0e-6 * (double) new_time.tv_usec)
			- ((double) old_time.tv_sec + 1.0e-6 * (double) old_time.tv_usec);
	old_time.tv_sec = new_time.tv_sec;
	old_time.tv_usec = new_time.tv_usec;
	return DeltaT;
#elif defined (__APPLE__) || defined (MACOSX)
	static time_t old_time;
	time_t new_time;
	new_time = clock();
	const double DeltaT = double(new_time - old_time) / CLOCKS_PER_SEC;
	old_time.tv_sec = new_time.tv_sec;
	old_time.tv_usec = new_time.tv_usec;
	return DeltaT;
#else
	return 0;
#endif
}

void bandwidth()
{
	printf(
			"device,platform name,device name,size (B),memory,access,direction,time (s),bandwidth (MB/s)\n");
	int g = 0;
	cl_int error;
	cl_uint num_platforms;
	checkOclErrors(clGetPlatformIDs(0, NULL, &num_platforms));
	cl_platform_id* platforms = (cl_platform_id*) malloc(
			sizeof(cl_platform_id) * num_platforms);
	checkOclErrors(clGetPlatformIDs(num_platforms, platforms, NULL));
	for (cl_uint p = 0; p < 1; ++p)
	{
		cl_platform_id platform = platforms[p];
		char platform_name[256];
		checkOclErrors(
				clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL));
//		if (strcmp(platform_name, "NVIDIA CUDA")) continue;
		cl_uint num_devices;
		checkOclErrors(
				clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices));
		cl_device_id* devices = (cl_device_id*) malloc(
				sizeof(cl_device_id) * num_devices);
		checkOclErrors(
				clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL));
		for (cl_uint d = 0; d < num_devices; ++d, ++g)
		{
			cl_device_id device = devices[d];
			char device_name[256];
			checkOclErrors(
					clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL));
			cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL,
					&error);
			checkOclErrors(error);
			cl_command_queue command_queue = clCreateCommandQueue(context,
					device, 0/*CL_QUEUE_PROFILING_ENABLE*/, &error);
			checkOclErrors(error);

			const size_t size = 100000 * sizeof(float);
			const double bandwidth_unit = (double) size / (1 << 20);
			void* h_p = NULL;
			void* d_p = NULL;
			cl_mem h_b;
			cl_mem d_b;
			double time;
			double bandwidth;
			d_b = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL,
					&error);
			checkOclErrors(error);

			// allocate pinned h_b
			h_b = clCreateBuffer(context, /*CL_MEM_READ_WRITE | */
			CL_MEM_ALLOC_HOST_PTR, size, NULL, &error);
			checkOclErrors(error);
			// --memory=pinned --access=mapped --htod
			shrDeltaT();
			h_p = clEnqueueMapBuffer(command_queue, h_b, CL_TRUE, CL_MAP_READ,
					0, size, 0, NULL, NULL, &error);
			checkOclErrors(error);
			d_p = clEnqueueMapBuffer(command_queue, d_b, CL_TRUE,
					CL_MAP_WRITE_INVALIDATE_REGION, 0, size, 0, NULL, NULL,
					&error);
			checkOclErrors(error);

			memcpy(d_p, h_p, size);
			checkOclErrors(
					clEnqueueUnmapMemObject(command_queue, d_b, d_p, 0, NULL, NULL));
			checkOclErrors(
					clEnqueueUnmapMemObject(command_queue, h_b, h_p, 0, NULL, NULL));
			checkOclErrors(clFinish(command_queue));
			time = shrDeltaT();
			bandwidth = bandwidth_unit / time;
			printf("%d,%s,%s,%lu,%s,%s,%s,%.3f,%.0f\n", g, platform_name,
					device_name, size, "pinned", "mapped", "HtoD", time,
					bandwidth);
			// --memory=pinned --access=mapped --dtoh
			shrDeltaT();
			h_p = clEnqueueMapBuffer(command_queue, h_b, CL_TRUE,
					CL_MAP_WRITE_INVALIDATE_REGION, 0, size, 0, NULL, NULL,
					&error);
			checkOclErrors(error);
			d_p = clEnqueueMapBuffer(command_queue, d_b, CL_TRUE, CL_MAP_READ,
					0, size, 0, NULL, NULL, &error);
			checkOclErrors(error);

			memcpy(h_p, d_p, size);

			checkOclErrors(
					clEnqueueUnmapMemObject(command_queue, d_b, d_p, 0, NULL, NULL));
			checkOclErrors(
					clEnqueueUnmapMemObject(command_queue, h_b, h_p, 0, NULL, NULL));
			checkOclErrors(clFinish(command_queue));
			time = shrDeltaT();
			bandwidth = bandwidth_unit / time;
			printf("%d,%s,%s,%lu,%s,%s,%s,%.3f,%.0f\n", g, platform_name,
					device_name, size, "pinned", "mapped", "DtoH", time,
					bandwidth);

			// deallocate pinned h_b
			checkOclErrors(clReleaseMemObject(h_b));

			checkOclErrors(clReleaseMemObject(d_b));

			checkOclErrors(clReleaseCommandQueue(command_queue));
			checkOclErrors(clReleaseContext(context));
		}
		free(devices);
	}
	free(platforms);
}

typedef struct float4
{
	float x;
	float y;
	float z;
	float w;
} float4;

static void show_usage(std::string name)
{
	std::cerr << "\nUsage: " << name << " <option(s)>" << " Options:\n"
			<< "\t-h,--help\t\tShow this help message\n"
			<< "\t-n <number of points>\tSpecify the number of points to use for the kdtree\n"
			<< "\t-t \tRun the validity tests\n"
			<< "\t-s \tRun the sequential algo\n"
			<< "\t-c \tRun the vanilla cmssw algo\n"
			<< "\t-f \tRun FKDtree algo\n" << "\t-a \tRun all the algos\n"
			<< "\t-ocl \tRun OpenCL search algo\n" << std::endl;

}
int main(int argc, char* argv[])
{

	if (argc < 3)
	{
		show_usage(argv[0]);
		return 1;
	}

	int nPoints = 100000;
	bool runTheTests = false;
	bool runSequential = false;
	bool runFKDTree = false;
	bool runOldKDTree = false;
	bool runOpenCL = false;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help"))
		{
			show_usage(argv[0]);
			return 0;
		}

		else if (arg == "-n")
		{
			if (i + 1 < argc) // Make sure we aren't at the end of argv!
			{
				i++;
				std::istringstream ss(argv[i]);
				if (!(ss >> nPoints))
					std::cerr << "Invalid number " << argv[i] << '\n';
			}
		}
		else if (arg == "-t")
		{
			runTheTests = true;
		}
		else if (arg == "-s")
		{
			runSequential = true;
		}
		else if (arg == "-f")
		{
			runFKDTree = true;
		}
		else if (arg == "-c")
		{
			runOldKDTree = true;
		}
		else if (arg == "-a")
		{
			runOldKDTree = true;
			runFKDTree = true;
			runSequential = true;
		}
		else if (arg == "-ocl")
		{
			runOpenCL = true;
			runFKDTree = true;

		}
	}

	std::vector<KDPoint<float, 3> > points;
	std::vector<KDPoint<float, 3> > minPoints;
	std::vector<KDPoint<float, 3> > maxPoints;

	float range_x = 0.1;
	float range_y = 0.1;
	float range_z = 0.1;

	KDPoint<float, 3> minPoint(0, 1, 8);
	KDPoint<float, 3> maxPoint(0.4, 1.2, 8.3);
	for (int i = 0; i < nPoints; ++i)
	{
		float x = static_cast<float>(rand())
				/ (static_cast<float>(RAND_MAX / 10.1));
		;
		float y = static_cast<float>(rand())
				/ (static_cast<float>(RAND_MAX / 10.1));
		;
		float z = static_cast<float>(rand())
				/ (static_cast<float>(RAND_MAX / 10.1));
		KDPoint<float, 3> Point(x, y, z);
		Point.setId(i);

		points.push_back(Point);
		KDPoint<float, 3> m(x - range_x, y - range_y, z - range_z);
		minPoints.push_back(m);
		KDPoint<float, 3> M(x + range_x, y + range_y, z + range_z);
		maxPoints.push_back(M);

	}
//needed by the vanilla algo
	float4* cmssw_points;
	cmssw_points = new float4[nPoints];
	for (int j = 0; j < nPoints; j++)
	{
		cmssw_points[j].x = points[j][0];
		cmssw_points[j].y = points[j][1];
		cmssw_points[j].z = points[j][2];
		cmssw_points[j].w = j; //Use this to save the index

	}

	std::cout << "Cloud of points generated.\n" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));

	if (runFKDTree)
	{
		long int pointsFound = 0;
		std::cout << "FKDTree run will start in 1 second.\n" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));

		std::chrono::steady_clock::time_point start_building =
				std::chrono::steady_clock::now();
		FKDTree<float, 3> kdtree(nPoints, points);

		kdtree.build();
		std::chrono::steady_clock::time_point end_building =
				std::chrono::steady_clock::now();
		std::cout << "building FKDTree with " << nPoints << " points took "
				<< std::chrono::duration_cast < std::chrono::milliseconds
				> (end_building - start_building).count() << "ms" << std::endl;
		if (runTheTests)
		{
			if (kdtree.test_correct_build())
				std::cout << "FKDTree built correctly" << std::endl;
			else
				std::cerr << "FKDTree wrong" << std::endl;
		}

		if (runOpenCL)
		{

			int g = 0;
			cl_int error;
			cl_uint num_platforms;
			checkOclErrors(clGetPlatformIDs(0, NULL, &num_platforms));
			cl_platform_id* platforms = (cl_platform_id*) malloc(
					sizeof(cl_platform_id) * num_platforms);
			checkOclErrors(clGetPlatformIDs(num_platforms, platforms, NULL));
			for (cl_uint p = 0; p < 1; ++p)
			{
				cl_platform_id platform = platforms[p];
				char platform_name[256];
				checkOclErrors(
						clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL));
				//		if (strcmp(platform_name, "NVIDIA CUDA")) continue;
				cl_uint num_devices;
				checkOclErrors(
						clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices));
				cl_device_id* devices = (cl_device_id*) malloc(
						sizeof(cl_device_id) * num_devices);
				checkOclErrors(
						clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL));
				for (cl_uint d = 0; d < num_devices; ++d, ++g)
				{
					cl_device_id device = devices[d];
					char device_name[256];
					checkOclErrors(
							clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL));
					cl_context context = clCreateContext(NULL, 1, &device, NULL,
					NULL, &error);
					checkOclErrors(error);
					cl_command_queue command_queue = clCreateCommandQueue(
							context, device, 0/*CL_QUEUE_PROFILING_ENABLE*/,
							&error);
					checkOclErrors(error);

					std::chrono::steady_clock::time_point start_opencl =
							std::chrono::steady_clock::now();

					cl_mem d_dimensions_mem;
					cl_mem h_dimensions_mem;

					void* d_dimensions;
					void* h_dimensions;

					void* d_ids = nullptr;
					void* h_ids = nullptr;
					cl_mem d_ids_mem;
					cl_mem h_ids_mem;

					void* d_results;
					void* h_results;
					cl_mem d_results_mem;
					cl_mem h_results_mem;

					const size_t maxResultSize = 100;

					//allocating device memory block
					d_dimensions_mem = clCreateBuffer(context,
							CL_MEM_READ_WRITE, 3 * nPoints * sizeof(float),
							NULL, &error);
					checkOclErrors(error);

					//allocating host memory block
					h_dimensions_mem = clCreateBuffer(context, /*CL_MEM_READ_WRITE | */
					CL_MEM_ALLOC_HOST_PTR, 3 * nPoints * sizeof(float), NULL,
							&error);
					checkOclErrors(error);

					h_dimensions = clEnqueueMapBuffer(command_queue,
							h_dimensions_mem, CL_TRUE,
							CL_MAP_READ | CL_MAP_WRITE, 0,
							3 * nPoints * sizeof(float), 0, NULL, NULL, &error);
					checkOclErrors(error);
					d_dimensions = clEnqueueMapBuffer(command_queue,
							d_dimensions_mem, CL_TRUE,
							CL_MAP_READ | CL_MAP_WRITE, 0,
							3 * nPoints * sizeof(float), 0, NULL, NULL, &error);
					checkOclErrors(error);

					d_ids_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
							nPoints * sizeof(unsigned int), NULL, &error);
					checkOclErrors(error);
					h_ids_mem = clCreateBuffer(context, /*CL_MEM_READ_WRITE | */
					CL_MEM_ALLOC_HOST_PTR, nPoints * sizeof(unsigned int), NULL,
							&error);
					checkOclErrors(error);

					h_ids = clEnqueueMapBuffer(command_queue, h_ids_mem,
							CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0,
							nPoints * sizeof(unsigned int), 0, NULL, NULL,
							&error);
					checkOclErrors(error);
					d_ids = clEnqueueMapBuffer(command_queue, d_ids_mem,
							CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0,
							nPoints * sizeof(unsigned int), 0, NULL, NULL,
							&error);
					checkOclErrors(error);

					d_results_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
							(nPoints + nPoints * maxResultSize)
									* sizeof(unsigned int), NULL, &error);
					checkOclErrors(error);
					h_results_mem = clCreateBuffer(context, /*CL_MEM_READ_WRITE | */
					CL_MEM_ALLOC_HOST_PTR,
							(nPoints + nPoints * maxResultSize)
									* sizeof(unsigned int), NULL, &error);
					checkOclErrors(error);

					h_results = clEnqueueMapBuffer(command_queue, h_results_mem,
							CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0,
							(nPoints + nPoints * maxResultSize)
									* sizeof(unsigned int), 0, NULL, NULL,
							&error);
					checkOclErrors(error);
					d_results = clEnqueueMapBuffer(command_queue, d_results_mem,
							CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0,
							(nPoints + nPoints * maxResultSize)
									* sizeof(unsigned int), 0, NULL, NULL,
							&error);
					checkOclErrors(error);

					for (int dim = 0; dim < 3; dim++)
					{
						float* dummy_dim = (float*) (h_dimensions);
						memcpy(&dummy_dim[nPoints * dim],
								kdtree.getDimensionVector(dim).data(),
								nPoints * sizeof(float));

					}
					memcpy(d_dimensions, h_dimensions,
							3 * nPoints * sizeof(float));
					memcpy(h_ids, kdtree.getIdVector().data(),
							nPoints * sizeof(unsigned int));

					memcpy(d_ids, h_ids, nPoints * sizeof(unsigned int));

					//memcpy(h_results, d_results,nPoints * sizeof(unsigned int));

					std::chrono::steady_clock::time_point end_opencl =
							std::chrono::steady_clock::now();

					//unsigned int* risultati = (unsigned int*) h_results;
					/*for (int i = 0; i < nPoints; ++i)
					 {
					 std::cout << risultati[i] << std::endl;

					 }*/

					const size_t lws = 256;
					const size_t gws = 32 * lws;
					std::ifstream ifs("searchInTheBox.cl");
					std::string source((std::istreambuf_iterator<char>(ifs)),
							std::istreambuf_iterator<char>());
					const char* sources[] =
					{ source.data() };
					const size_t source_length = source.length();

					cl_program program = clCreateProgramWithSource(context, 1,
							sources, &source_length, &error);
					checkOclErrors(error);

					checkOclErrors(
							clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
					size_t len;
					char *buffer;
					clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							0, NULL, &len);
					buffer = (char*) calloc(len, sizeof(char));
					clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							len, buffer, NULL);
					printf("%s\n", buffer);
					cl_kernel kernel = clCreateKernel(program,
							"SearchInTheKDBox", &error);
					checkOclErrors(error);

					checkOclErrors(
							clSetKernelArg(kernel, 0, sizeof(unsigned int),
									&nPoints));
					checkOclErrors(
							clSetKernelArg(kernel, 1, sizeof(cl_mem),
									&d_dimensions_mem));
					checkOclErrors(
							clSetKernelArg(kernel, 2, sizeof(cl_mem),
									&d_ids_mem));
					checkOclErrors(
							clSetKernelArg(kernel, 3, sizeof(cl_mem),
									&d_results_mem));

					cl_event kernel_event;
					checkOclErrors(
							clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &gws, &lws, 0, NULL, &kernel_event));



					std::this_thread::sleep_for(std::chrono::seconds(2));
					memcpy(h_results, d_results,
							(nPoints + nPoints * maxResultSize)
									* sizeof(unsigned int));

					std::cout
							<< "initialization of buffers using opencl device "
							<< platform_name << " " << device_name << " for "
							<< nPoints << " points took "
							<< std::chrono::duration_cast
							< std::chrono::milliseconds
							> (end_opencl - start_opencl).count() << "ms"
									<< std::endl;

					unsigned int* result = (unsigned int*)h_results;
					unsigned int totalNumberOfPointsFound = 0;

					for(int v=0; v< nPoints + nPoints * maxResultSize; v++ )
					{
						std::cout << result[v] << std::endl;
					}
					for(int p = 0; p<nPoints; p++)
					{
						unsigned int length = result[p];
						totalNumberOfPointsFound += length;
						int firstIndex = nPoints + maxResultSize*p;
						std::cout << "searching neighbor for point id " << p << " found " << length << " points" <<  std::endl;
						for (int r = 0; r< length; ++r)
						{
							std::cout << r << "\tpoint id " << result[firstIndex + r] << std::endl;

						}



					}
					std::cout << "GPU found " << totalNumberOfPointsFound << " points." << std::endl;


					checkOclErrors(
							clEnqueueUnmapMemObject(command_queue, d_dimensions_mem, d_dimensions, 0, NULL, NULL));
					checkOclErrors(error);
					checkOclErrors(
							clEnqueueUnmapMemObject(command_queue, h_dimensions_mem, h_dimensions, 0, NULL, NULL));
					checkOclErrors(error);

					checkOclErrors(
							clEnqueueUnmapMemObject(command_queue, d_ids_mem, d_ids, 0, NULL, NULL));
					checkOclErrors(error);
					checkOclErrors(
							clEnqueueUnmapMemObject(command_queue, h_ids_mem, h_ids, 0, NULL, NULL));
					checkOclErrors(error);
					checkOclErrors(clFinish(command_queue));

					checkOclErrors(
							clEnqueueUnmapMemObject(command_queue, d_results_mem, d_results, 0, NULL, NULL));
					checkOclErrors(error);
					checkOclErrors(
							clEnqueueUnmapMemObject(command_queue, h_results_mem, h_results, 0, NULL, NULL));
					checkOclErrors(error);
					checkOclErrors(clFinish(command_queue));

					// deallocate pinned h_b

					checkOclErrors(clReleaseMemObject(d_dimensions_mem));
					checkOclErrors(clReleaseMemObject(h_dimensions_mem));

					checkOclErrors(clReleaseMemObject(h_ids_mem));
					checkOclErrors(clReleaseMemObject(d_ids_mem));

					checkOclErrors(clReleaseCommandQueue(command_queue));

					checkOclErrors(clReleaseKernel(kernel));
					checkOclErrors(clReleaseProgram(program));
					checkOclErrors(clReleaseContext(context));
				}
				free(devices);
			}
			free(platforms);

		}

		std::chrono::steady_clock::time_point start_searching =
				std::chrono::steady_clock::now();
		for (int i = 0; i < nPoints; ++i)
			pointsFound +=
					kdtree.search_in_the_box(minPoints[i], maxPoints[i]).size();
		std::chrono::steady_clock::time_point end_searching =
				std::chrono::steady_clock::now();

		std::cout << "searching points using FKDTree took "
				<< std::chrono::duration_cast < std::chrono::milliseconds
				> (end_searching - start_searching).count() << "ms\n"
						<< " found points: " << pointsFound
						<< "\n******************************\n" << std::endl;
	}
//	int pointsFoundNaive = 0;
//
	if (runSequential)
	{
		std::cout << "Sequential run will start in 1 second.\n" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::chrono::steady_clock::time_point start_sequential =
				std::chrono::steady_clock::now();
		long int pointsFound = 0;
		for (int i = 0; i < nPoints; ++i)
		{
			for (auto p : points)
			{

				bool inTheBox = true;

				for (int d = 0; d < 3; ++d)
				{

					inTheBox &= (p[d] <= maxPoints[i][d]
							&& p[d] >= minPoints[i][d]);

				}
				pointsFound += inTheBox;
			}

		}

		std::chrono::steady_clock::time_point end_sequential =
				std::chrono::steady_clock::now();
		std::cout << "Sequential search algorithm took "
				<< std::chrono::duration_cast < std::chrono::milliseconds
				> (end_sequential - start_sequential).count() << "ms\n"
						<< " found points: " << pointsFound
						<< "\n******************************\n" << std::endl;
	}

	if (runOldKDTree)
	{

		std::cout << "Vanilla CMSSW KDTree run will start in 1 second.\n"
				<< std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::chrono::steady_clock::time_point start_building =
				std::chrono::steady_clock::now();

		KDTreeLinkerAlgo<unsigned, 3> vanilla_tree;
		std::vector<KDTreeNodeInfoT<unsigned, 3> > vanilla_nodes;
		std::vector<KDTreeNodeInfoT<unsigned, 3> > vanilla_founds;

		std::array<float, 3> minpos
		{
		{ 0.0f, 0.0f, 0.0f } }, maxpos
		{
		{ 0.0f, 0.0f, 0.0f } };

		vanilla_tree.clear();
		vanilla_founds.clear();
		for (unsigned i = 0; i < nPoints; ++i)
		{
			float4 pos = cmssw_points[i];
			vanilla_nodes.emplace_back(i, (float) pos.x, (float) pos.y,
					(float) pos.z);
			if (i == 0)
			{
				minpos[0] = pos.x;
				minpos[1] = pos.y;
				minpos[2] = pos.z;
				maxpos[0] = pos.x;
				maxpos[1] = pos.y;
				maxpos[2] = pos.z;
			}
			else
			{
				minpos[0] = std::min((float) pos.x, minpos[0]);
				minpos[1] = std::min((float) pos.y, minpos[1]);
				minpos[2] = std::min((float) pos.z, minpos[2]);
				maxpos[0] = std::max((float) pos.x, maxpos[0]);
				maxpos[1] = std::max((float) pos.y, maxpos[1]);
				maxpos[2] = std::max((float) pos.z, maxpos[2]);
			}
		}

		KDTreeCube cluster_bounds = KDTreeCube(minpos[0], maxpos[0], minpos[1],
				maxpos[1], minpos[2], maxpos[2]);

		vanilla_tree.build(vanilla_nodes, cluster_bounds);
		std::chrono::steady_clock::time_point end_building =
				std::chrono::steady_clock::now();
		long int pointsFound = 0;
		std::chrono::steady_clock::time_point start_searching =
				std::chrono::steady_clock::now();
		for (int i = 0; i < nPoints; ++i)
		{
			KDTreeCube kd_searchcube(minPoints[i][0], maxPoints[i][0],
					minPoints[i][1], maxPoints[i][1], minPoints[i][2],
					maxPoints[i][2]);
			vanilla_tree.search(kd_searchcube, vanilla_founds);
			pointsFound += vanilla_founds.size();
			vanilla_founds.clear();
		}
		std::chrono::steady_clock::time_point end_searching =
				std::chrono::steady_clock::now();

		std::cout << "building Vanilla CMSSW KDTree with " << nPoints
				<< " points took " << std::chrono::duration_cast
				< std::chrono::milliseconds
				> (end_building - start_building).count() << "ms" << std::endl;
		std::cout << "searching points using Vanilla CMSSW KDTree took "
				<< std::chrono::duration_cast < std::chrono::milliseconds
				> (end_searching - start_searching).count() << "ms"
						<< std::endl;
		std::cout << pointsFound
				<< " points found using Vanilla CMSSW KDTree\n******************************\n"
				<< std::endl;

		delete[] cmssw_points;
	}
	return 0;
}
