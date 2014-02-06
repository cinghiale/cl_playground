#pragma once
#include <ostream>
#include <string>
#include <vector>
#include <CL/cl.h>

namespace cl1 { namespace utils {

using std::string;
using std::vector;

cl_platform_id getFirstPlatform();
cl_device_id getFirstDevice(cl_platform_id platform, cl_device_type type=CL_DEVICE_TYPE_ALL);

string getPlatformInfo(cl_platform_id platform, cl_platform_info param);

cl_program createProgramFromFiles(cl_context context, vector<string> const& filenames);

void buildProgram(cl_program program, std::ostream *logger=nullptr);

}};
