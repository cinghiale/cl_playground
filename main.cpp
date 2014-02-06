#include <array>
#include <iostream>
#include <fstream>
#include <memory>
#include "utils.h"

int main() {
    using namespace std;
    using namespace cl1::utils;

    auto platform = getFirstPlatform();
    cout << "PLATFORM NAME: " << getPlatformInfo(platform, CL_PLATFORM_NAME) << endl;
    cout << "PLATFORM VERSION: " << getPlatformInfo(platform, CL_PLATFORM_VERSION) << endl;

    cl_context_properties properties[3] = {
        (cl_context_properties)CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        0};
    auto context = clCreateContextFromType(
        properties,
        CL_DEVICE_TYPE_GPU,
        nullptr,
        nullptr,
        nullptr);

    cl_device_id device;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(device), &device, nullptr);

    auto queue = clCreateCommandQueue(context, device, 0, nullptr);

    const int size = 1234567;
    auto src1 = new std::array<float, size>;
    auto src2 = new std::array<float, size>;
    auto result = new std::array<float, size>;
    for (int i = 0; i < size; i++) {
        (*src1)[i] = (*src2)[i] = static_cast<float>(i);
    }

    cl_int error = 0;
    auto cl_src1 = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * src1->size(), src1, &error);

    auto cl_src2 = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * src2->size(), src2, &error);

    auto cl_result = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(float) * result->size(), nullptr, &error);

    auto program = createProgramFromFiles(context, {"../kernel1.cl"});
    buildProgram(program, &cout);
    cout << "program compiled" << endl;

    auto kernel = clCreateKernel(program, "vector_add_gpu", &error);
    if (error != CL_SUCCESS) {
        throw std::runtime_error("clCreateKernel");
    }

    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &cl_src1);
    error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &cl_src2);
    error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &cl_result);
    error |= clSetKernelArg(kernel, 3, sizeof(size), &size);

    if (error != CL_SUCCESS) {
        throw std::runtime_error("clSetKernelArg");
    }

    size_t const local_ws = 512;
    size_t const global_ws = 1234944;
    error = clEnqueueNDRangeKernel(
        queue, kernel, 1, nullptr, &global_ws, &local_ws, 0, nullptr, nullptr);

    if (error != CL_SUCCESS) {
        throw std::runtime_error("clEnqueueNDRangeKernel");
    }

    clEnqueueReadBuffer(
        queue,
        cl_result,
        CL_TRUE,
        0,
        sizeof(float) * result->size(),
        result,
        0,
        nullptr,
        nullptr);

    for (int i = 0; i < size; i++) {
        if ((*result)[i] != (*src1)[i] + (*src2)[i]) {
            cout << i << " - " << (*src1)[i] << " + " << (*src2)[i] << " = " << (*result)[i] << endl;
        }
    }
    return 0;
}
