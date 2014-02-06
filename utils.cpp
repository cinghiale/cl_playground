#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <memory>
#include "utils.h"

namespace cl1 { namespace utils {

cl_platform_id getFirstPlatform() {
    cl_platform_id buffer[1];
    clGetPlatformIDs(1, buffer, nullptr);
    return buffer[0];
}

cl_device_id getFirstDevice(cl_platform_id platform, cl_device_type type) {
    cl_device_id buffer[1];
    clGetDeviceIDs(platform, type, 1, buffer, nullptr);
    return buffer[0];
}

string getPlatformInfo(cl_platform_id platform, cl_platform_info param) {
    size_t size;
    clGetPlatformInfo(platform, param, 0, nullptr, &size);

    auto buffer = std::unique_ptr<char[]>(new char[size]);
    clGetPlatformInfo(platform, param, size, buffer.get(), nullptr);
    return buffer.get();
}

cl_program createProgramFromFiles(cl_context context, vector<string> const& filenames) {
    size_t count = filenames.size();

    vector<string> sources;
    auto it = sources.begin();
    for(auto& fname : filenames) {
        std::ifstream source(fname);
        if (!source.good()) {
            throw std::runtime_error("invalid file");
        }
        std::stringstream buffer;
        buffer << source.rdbuf();
        sources.push_back(buffer.str());
    }

    char const** strings = new char const*[count];
    size_t* lengths = new size_t[count];

    for(int ix=0; ix<count; ix++) {
        auto& s = sources[ix];
        strings[ix] = s.data();
        lengths[ix] = s.size();
    }

    cl_int error = 0;
    auto program = clCreateProgramWithSource(
        context,
        sources.size(),
        strings,
        lengths,
        &error);

    delete strings;
    delete lengths;
    if (error != CL_SUCCESS) {
        throw std::runtime_error("clCreateProgramWithSource");
    }
    return program;
}

void buildProgram(cl_program program, std::ostream *logger) {
    cl_int error;
    error = clBuildProgram(
        program,
        0,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
    if(error != CL_SUCCESS && logger) {
        cl_uint count;
        clGetProgramInfo(
            program,
            CL_PROGRAM_NUM_DEVICES,
            sizeof(count),
            &count,
            nullptr);

        auto devices = new cl_device_id[count];
        clGetProgramInfo(
            program,
            CL_PROGRAM_DEVICES,
            count*sizeof(cl_device_id),
            devices,
            nullptr);

        for(int ix=0; ix<count; ix++) {
            using std::endl;
            *logger << "Build info device #" << ix << endl;
            auto device = devices[ix];

            cl_build_status status;
            clGetProgramBuildInfo(
                program,
                device,
                CL_PROGRAM_BUILD_STATUS,
                sizeof(status),
                &status,
                nullptr);
            *logger << " - STATUS: ";
            if(status == CL_BUILD_NONE) {
                *logger << "NONE" << endl;
            }
            else if(status == CL_BUILD_ERROR) {
                *logger << "ERROR" << endl;
            }
            else if(status == CL_BUILD_SUCCESS) {
                *logger << "SUCCESS" << endl;
            }
            else if(status == CL_BUILD_IN_PROGRESS) {
                *logger << "IN PROGRESS" << endl;
            }

            size_t size;
            clGetProgramBuildInfo(
                program,
                device,
                CL_PROGRAM_BUILD_LOG,
                0,
                nullptr,
                &size);

            auto buffer = std::unique_ptr<char[]>(new char[size]);
            clGetProgramBuildInfo(
                program,
                device,
                CL_PROGRAM_BUILD_LOG,
                size,
                buffer.get(),
                nullptr);
            *logger << " - LOG: " << buffer.get() << endl;
        }
    }

    if (error != CL_SUCCESS) {
        throw std::runtime_error("clBuildProgram");
    }
}

}};
