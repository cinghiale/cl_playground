#define __CL_REFERENCE
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <GL/glx.h>
#include "ocl.h"
#include "utils.h"

namespace ocl {
    using std::vector;
    using std::string;

    using clGetGLContextInfoKHR_signature = cl_int (*)(cl_context_properties const*, cl_gl_context_info, size_t, void *, size_t *);

    auto clGetGLContextInfoKHR = (clGetGLContextInfoKHR_signature)clGetExtensionFunctionAddress("clGetGLContextInfoKHR");

    cl::Platform get_default_platform() {
        vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        return platforms[0];
    }
    cl::Context shared_cl_context() {
        return shared_cl_context(get_default_platform());
    }
    cl::Context shared_cl_context(cl::Platform const& p) {
        string check;
        p.getInfo(CL_PLATFORM_EXTENSIONS, &check);
        if(check.find("cl_khr_gl_sharing") == string::npos) {
            throw std::runtime_error("platform missing gl_sharing extension");
        }

        cl_context_properties props[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext( ),
            CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay( ),
            CL_CONTEXT_PLATFORM, (cl_context_properties) p(),
            0
        };
        cl_device_id current_device = 0;
        clGetGLContextInfoKHR(
            props,
            CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
            sizeof(current_device),
            &current_device,
            nullptr);
        if(!current_device) {
            // maybe we should check CL_DEVICES_FOR_GL_CONTEXT_KHR
            throw std::runtime_error("no device currently associated with gl");
        }

        auto ctx = cl::Context(
            vector<cl::Device>{ cl::Device(current_device) },
            props
        );
        return ctx;
    }

    std::tuple<cl::Context, cl::Program> shared_cl_context(std::vector<std::string> const& filenames, std::ostream *logger) {
        auto ctx = shared_cl_context();

        auto program = create_program_from_files(ctx, filenames, logger);
        return std::make_tuple(ctx, program);
    }

    cl::Program create_program_from_files(cl::Context const& ctx, vector<string> const& filenames, std::ostream *logger) {
        size_t count = filenames.size();

        vector<string> sources;
        for(auto& fname : filenames) {
            std::ifstream source(fname);
            if (!source.good()) {
                throw std::runtime_error(supplant("error opening file: %", fname));
            }
            std::stringstream buffer;
            buffer << source.rdbuf();
            sources.push_back(buffer.str());
        }
        return create_program_from_strings(ctx, sources, logger);
    }

    cl::Program create_program_from_strings(cl::Context const& ctx, vector<string> const& strings, std::ostream *logger) {
        cl::Program::Sources sources;

        for(auto& src : strings) {
            sources.push_back(std::make_pair(src.data(), src.size()));
        }
        auto program = cl::Program(ctx, sources);
        try {
            program.build({});
        } catch(cl::Error& e) {
            if(logger) {
                using std::endl;
                auto devices = program.getInfo<CL_PROGRAM_DEVICES>();
                int count = 0;
                for(auto& device_id : devices) {
                    *logger << supplant("Build info device #% %", count++, cl::Device(device_id).getInfo<CL_DEVICE_NAME>()) << endl;
                    *logger << "--------------------------------------------------------------------------------" << endl;

                    auto status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device_id);
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
                    *logger << " - LOG:" << endl;
                    *logger << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device_id) << endl;
                }
            }
            throw;
        }
        return program;
    }

    vector<cl::CommandQueue> create_queues(cl::Context const& ctx) {
        auto devices = ctx.getInfo<CL_CONTEXT_DEVICES>();
        vector<cl::CommandQueue> output;
        for(auto& device_id : devices) {
            output.push_back({ctx, device_id});
        }
        return output;
    }
};
