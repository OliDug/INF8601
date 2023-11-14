#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "log.h"
#include "sinoscope.h"

#define M_PI 3.14159265358979323846264338328

int sinoscope_opencl_init(sinoscope_opencl_t* opencl, cl_device_id opencl_device_id, unsigned int width,
			  unsigned int height) {
	
	cl_int error = 0;
	cl_context context;
	cl_command_queue queue;
	cl_mem buffer;
	cl_kernel sinoscope_kernel;
	size_t buffer_size;
	size_t kernel_size;
	char* kernel_source;

	context = clCreateContext(0, 1, &opencl_device_id, NULL, NULL, &error);
	if (error != CL_SUCCESS) { return error; }

	queue = clCreateCommandQueue(context, opencl_device_id, 0, &error);
	if (error != CL_SUCCESS) { return error; }

	buffer_size = sizeof(char) * width * height * 3;
	buffer = clCreateBuffer(context,  CL_MEM_READ_ONLY, buffer_size, NULL, &error);

	opencl_load_kernel_code(&kernel_source, &kernel_size);
	const char* const_src = kernel_source;
	cl_program pgm = clCreateProgramWithSource(context, 1, &const_src, &kernel_size, &error);
	if(error != CL_SUCCESS) { printf("pgm no created"); return error; }

	error = clBuildProgram(pgm, 1, &opencl_device_id, "-I " __OPENCL_INCLUDE__, NULL, NULL) ;
	if(error != CL_SUCCESS) { size_t len;
        char buffer[2048];
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(pgm, opencl_device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);return error; 
	}

	sinoscope_kernel = clCreateKernel(pgm,"worker_id_example",&error);
	if(error != CL_SUCCESS) { printf("kernel not created \n"); return error; }

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
	
typedef struct parametres {
    int width;
    int height;
    float dx;
    float dy;
    int taylor;
    float phase0;
    float phase1;
    float time;
    int interval;
    float interval_inverse;
} param_t;

int sinoscope_image_opencl(sinoscope_t* sinoscope) {
	cl_int error = 0;
	param_t param = {sinoscope->width,
					sinoscope->height,
					sinoscope->dx,
					sinoscope->dy,
					sinoscope->taylor,
					sinoscope->phase0,
					sinoscope->phase1,
					sinoscope->time,
					sinoscope->interval,
					sinoscope->interval_inverse};

	error = clSetKernelArg(sinoscope->opencl->kernel, 0, sizeof(cl_mem), (void *)&sinoscope->opencl->buffer);
	error = clSetKernelArg(sinoscope->opencl->kernel, 1, sizeof(param_t), (void *)&param);
	if (error != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", error);
        return error;
    }

	size_t global = 512*512;

    error = clEnqueueNDRangeKernel(sinoscope->opencl->queue, sinoscope->opencl->kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
    if (error) {
        printf("Error: Failed to execute kernel! %d\n", error);
        return error;
    }

	clFinish(sinoscope->opencl->queue);

	error = clEnqueueReadBuffer(sinoscope->opencl->queue, sinoscope->opencl->buffer, 
							CL_TRUE, 0, sinoscope->buffer_size, sinoscope->buffer, 0, NULL, NULL );  
    if (error != CL_SUCCESS) {
        printf("Error: Failed to read output array! %d\n", error);
        exit(1);
    }

	return 0;
}
