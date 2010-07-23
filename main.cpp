#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <SDKUtil/SDKFile.hpp>
#include <SDKUtil/SDKCommon.hpp>

#define __NO_STD_VECTOR
#define __NO_STD_STRING

#include <CL/cl.hpp>
#include "sha1.h"
SHA1Context sha;
int
main()
{
    cl_int err;

    // Platform info
    cl::vector<cl::Platform> platforms;
    std::cout<<"HelloCL!\nGetting Platform Information\n";
    err = cl::Platform::get(&platforms);
    if(err != CL_SUCCESS)
    {
        std::cerr << "Platform::get() failed (" << err << ")" << std::endl;
        return SDK_FAILURE;
    }

    cl::vector<cl::Platform>::iterator i;
    if(platforms.size() > 0)
    {
        for(i = platforms.begin(); i != platforms.end(); ++i)
        {
            if(!strcmp((*i).getInfo<CL_PLATFORM_VENDOR>(&err).c_str(), "Advanced Micro Devices, Inc."))
            {
                break;
            }
        }
    }
    if(err != CL_SUCCESS)
    {
        std::cerr << "Platform::getInfo() failed (" << err << ")" << std::endl;
        return SDK_FAILURE;
    }

    /* 
     * If we could find our platform, use it. Otherwise pass a NULL and get whatever the
     * implementation thinks we should be using.
     */

    cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(*i)(), 0 };

    std::cout<<"Creating a context AMD platform\n";
    cl::Context context(CL_DEVICE_TYPE_CPU, cps, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Context::Context() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    std::cout<<"Getting device info\n";
    cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    if (err != CL_SUCCESS) {
        std::cerr << "Context::getInfo() failed (" << err << ")\n";
        return SDK_FAILURE;
    }
    if (devices.size() == 0) {
        std::cerr << "No device available\n";
        return SDK_FAILURE;
    }

    std::cout<<"Loading and compiling CL source\n";
	streamsdk::SDKFile file;
    if (!file.open("SHAKernels.cl")) {
         std::cerr << "We couldn't load CL source code\n";
         return SDK_FAILURE;
    }
	cl::Program::Sources sources(1, std::make_pair(file.source().data(), file.source().size()));

    cl::Program*    pProgram;
	pProgram = new cl::Program(context, sources);
    if (err != CL_SUCCESS) {
        std::cerr << "Program::Program() failed (" << err << ")\n";
        return SDK_FAILURE;
    }
    cl::Program&    program = *pProgram;
    err = program.build(devices);
    if (err != CL_SUCCESS) {
        std::cerr << "Program::build() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    cl::Kernel kernel(program, "SHAKernel", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Kernel::Kernel() failed (" << err << ")\n";
        return SDK_FAILURE;
    }
    
    //kernel.setArg(0,&sha);
    if (err != CL_SUCCESS) {
        std::cerr << "Kernel::setArg() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    cl::CommandQueue queue(context, devices[0], 0, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "CommandQueue::CommandQueue() failed (" << err << ")\n";
    }

    std::cout<<"Running CL program\n";
    err = queue.enqueueNDRangeKernel(
        kernel, cl::NullRange, cl::NDRange(32, 32), cl::NDRange(32, 32)
    );

    if (err != CL_SUCCESS) {
        std::cerr << "CommandQueue::enqueueNDRangeKernel()" \
            " failed (" << err << ")\n";
       return SDK_FAILURE;
    }

    err = queue.finish();
    if (err != CL_SUCCESS) {
        std::cerr << "Event::wait() failed (" << err << ")\n";
    }

    delete pProgram;

    std::cout<<"Done\nPassed!\n";
    return SDK_SUCCESS;
}
