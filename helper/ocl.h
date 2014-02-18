#pragma once

#define __CL_ENABLE_EXCEPTIONS
#if defined(__CL_REFERENCE)
#include <CL/cl_gl.h>
#include "cl.hpp"
#elif defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <ostream>
#include <string>
#include <tuple>
#include <vector>
namespace ocl {
    cl::Platform get_default_platform();

    cl::Context shared_cl_context();
    cl::Context shared_cl_context(cl::Platform const&);
    std::tuple<cl::Context, cl::Program> shared_cl_context(std::vector<std::string> const&, std::ostream *logger=nullptr);

    cl::Program create_program_from_files(cl::Context const&, std::vector<std::string> const&, std::ostream *logger=nullptr);
    cl::Program create_program_from_strings(cl::Context const&, std::vector<std::string> const&, std::ostream *logger=nullptr);
    std::vector<cl::CommandQueue> create_queues(cl::Context const&);
};
