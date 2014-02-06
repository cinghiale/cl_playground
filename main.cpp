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
    return 0;
}
