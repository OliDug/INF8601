#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "log.h"
#include "sinoscope.h"

int sinoscope_opencl_init(sinoscope_opencl_t* opencl, cl_device_id opencl_device_id, unsigned int width,
			  unsigned int height) {
	cl_int error = 0;
	cl_context context;
	cl_command_queue queue;
	cl_mem buffer;
	cl_kernel sinoscope_kernel;
	size_t buffer_size;
	size_t kernel_size;
	char* bld_info;
	char* kernel_source;

	context = clCreateContext(0, 1, &opencl_device_id, NULL, NULL, &error);
	if (error != CL_SUCCESS) { return error; }

	queue = clCreateCommandQueue(context, opencl_device_id, 0, &error);
	if (error != CL_SUCCESS) { return error; }

	buffer_size = sizeof(unsigned char) * width * height * 3;
	buffer = clCreateBuffer(context,  CL_MEM_READ_ONLY, buffer_size, NULL, &error);

	opencl_load_kernel_code(&kernel_source, &kernel_size);
	const char* const_src = kernel_source;
	cl_program pgm = clCreateProgramWithSource(context, 1, &const_src, &kernel_size, &error);
	if(error != CL_SUCCESS) { return error; }

	error = clBuildProgram(pgm, 1, &opencl_device_id, NULL, NULL, NULL);
	if(error != CL_SUCCESS) { return error; }

	clGetProgramBuildInfo(pgm, opencl_device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &kernel_size);
	bld_info = (char*) malloc(sizeof(char) * (kernel_size + 1)); //new char[size+1]; 
	bld_info[kernel_size] = '\0';

	clGetProgramBuildInfo(pgm, opencl_device_id, CL_PROGRAM_BUILD_LOG, kernel_size, bld_info, NULL);
	sinoscope_kernel = clCreateKernel(pgm,"sinoscope",&error);
	if(error != CL_SUCCESS) { return error; }

	opencl->device_id = opencl_device_id;
	opencl->context = context;
	opencl->queue = queue;
	opencl->buffer = buffer;
	opencl->kernel = sinoscope_kernel;

	return 0;
}

void sinoscope_opencl_cleanup(sinoscope_opencl_t* opencl)
{

}

int sinoscope_image_opencl(sinoscope_t* sinoscope) {
	return -1;
}
