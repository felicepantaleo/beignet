#include <vector>
#include <memory>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>
#include <CL/cl.h>
#include "../cl_helper.h"

using namespace std;


int main(int argc, char* argv[])
{
    cl_uint num_platforms;
    checkOclErrors(clGetPlatformIDs(0, NULL, &num_platforms));
    vector<cl_platform_id> platforms(num_platforms);
    checkOclErrors(clGetPlatformIDs(num_platforms, platforms.data(), NULL));
    
    for (const auto platform : platforms)
    {
        checkOclErrors(clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL));
        printf("CL_PLATFORM_NAME: %s\n", buffer);
    }
}