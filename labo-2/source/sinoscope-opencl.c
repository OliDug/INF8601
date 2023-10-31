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
	clReleaseMemObject(opencl->buffer);
	clReleaseKernel(opencl->kernel);
	clReleaseCommandQueue(opencl->queue);
	clReleaseContext(opencl->context);
}

int sinoscope_image_opencl(sinoscope_t* sinoscope) {
	cl_int error = 0;

	error = clSetKernelArg(sinoscope->opencl->kernel, 0, sizeof(int), (void *)&sinoscope->width);
	error = clSetKernelArg(sinoscope->opencl->kernel, 1, sizeof(int), (void *)&sinoscope->height);
	error = clSetKernelArg(sinoscope->opencl->kernel, 2, sizeof(float), (void *)&sinoscope->dx);
	error = clSetKernelArg(sinoscope->opencl->kernel, 3, sizeof(float), (void *)&sinoscope->dy);
	error = clSetKernelArg(sinoscope->opencl->kernel, 4, sizeof(int), (void *)&sinoscope->taylor);
	error = clSetKernelArg(sinoscope->opencl->kernel, 5, sizeof(float), (void *)&sinoscope->phase0);
	error = clSetKernelArg(sinoscope->opencl->kernel, 6, sizeof(float), (void *)&sinoscope->phase1);
	error = clSetKernelArg(sinoscope->opencl->kernel, 7, sizeof(float), (void *)&sinoscope->time);
	error = clSetKernelArg(sinoscope->opencl->kernel, 8, sizeof(int), (void *)&sinoscope->interval);
	error = clSetKernelArg(sinoscope->opencl->kernel, 9, sizeof(float), (void *)&sinoscope->interval_inverse);
	error = clSetKernelArg(sinoscope->opencl->kernel, 10, sizeof(cl_mem), (void *)&sinoscope->opencl->buffer);

	/*for (int i = 0; i < sinoscope->width; i++) {
		for (int j = 0; j < sinoscope->height; j++){
			error = clSetKernelArg(sinoscope->opencl->kernel, 0, sizeof(int), (void *)&i);
			error = clSetKernelArg(sinoscope->opencl->kernel, 0, sizeof(int), (void *)&j);

		}
	}*/

	// Execute the OpenCL kernel on the list
    size_t global_item_size = sinoscope->height * sinoscope->width; // Process the entire image in one go
    size_t local_item_size = 1; // Divide work items into groups of 1
    error = clEnqueueNDRangeKernel(sinoscope->opencl->queue, sinoscope->opencl->kernel,
        1 /* work dimensions */, NULL /* global work offset */,
        &global_item_size /* global work size */, &local_item_size /* local work size */,
        0 /* number of events in wait list */, NULL /* event wait list */, NULL /* event */
	);

	// Read the memory buffer C on the device to the local variable C
    error = clEnqueueReadBuffer(sinoscope->opencl->queue,
        sinoscope->opencl->buffer,
        CL_TRUE,
        0,
        sinoscope->buffer_size,
        sinoscope->buffer,
        0,
        NULL,
        NULL);
}