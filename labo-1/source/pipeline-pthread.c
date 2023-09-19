#include <stdio.h>

#include "filter.h"
#include "pipeline.h"
#include <pthread.h>
#include "queue.h"

queue_t* load_queue;
queue_t* scale_up_queue;
queue_t* sharpen_queue;
queue_t* sobel_queue;

void *thread_load(void *image_dir) {
	image_t* image;
	while(1) {
		image = image_dir_load_next(image_dir);
		if (image == NULL) {
         		break;
        	}
		queue_push(load_queue, image);
	}
	image_destroy(image);
	printf("\n Fin de thread_load\n");
	pthread_exit(NULL);
}

void *thread_scale_up(void *inutilise) {
	image_t* image1 = queue_pop(load_queue);
	image_t* image2 = filter_scale_up(image1, 2);
	queue_push(scale_up_queue, image2);
        image_destroy(image1);
        image_destroy(image2);
	pthread_exit(NULL);
}

void *thread_sharpen(void *inutilise) {
	image_t* image3 = filter_sharpen(image2);
        image_destroy(image2);
	pthread_exit(NULL);
}

void *thread_sobel(void *inutilise) {
	//image_t* image4 = filter_sobel(image3);
        //image_destroy(image3);
	pthread_exit(NULL);
}

void *thread_save(void *inutilise) {
	//image_dir_save(image_dir, image4);
	//image_destroy(image4);
	pthread_exit(NULL);
}

int pipeline_pthread(image_dir_t* image_dir) {
	pthread_t thLoad, thScaleUp, thSharpen, thSobel, thSave;

	load_queue = queue_create(6912000); //10 images max (691200 bytes par image)
	scale_up_queue = queue_create(6912000);
	sharpen_queue = queue_create(6912000);
	sobel_queue = queue_create(6912000);

	pthread_create(&thLoad, NULL, thread_load, (void*) image_dir);
	pthread_create(&thScaleUp, NULL, thread_scale_up, NULL);
	pthread_create(&thSharpen, NULL, thread_sharpen, NULL);
	pthread_create(&thSobel, NULL, thread_sobel, NULL);
	pthread_create(&thSave, NULL, thread_save, NULL);

	pthread_join(thLoad, NULL);
	pthread_join(thScaleUp, NULL);
	pthread_join(thSharpen, NULL);
	pthread_join(thSobel, NULL);
	pthread_join(thSave, NULL);
	return 0;
}

