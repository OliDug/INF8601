#include <stdio.h>

#include "filter.h"
#include "pipeline.h"
#include <pthread.h>
#include "queue.h"

queue_t* load_queue;
queue_t* scale_up_queue;
queue_t* sharpen_queue;
queue_t* sobel_queue;

bool load_done;
bool scale_up_done;
bool sharpen_done;
bool sobel_done;

void *thread_load(void *image_dir) {
	image_t* image;
	while(1) {
		image = image_dir_load_next((image_dir_t*) image_dir);
		if (image == NULL) {
			load_done = true;
        	break;
        }
		queue_push(load_queue, image);
		printf(".");
	}
	printf("\n Fin de thread_load\n");
	pthread_exit(NULL);
}

void *thread_scale_up(void *inutilise) {
	image_t* image1;
	while(1) {
		if (load_queue->used == 0) {
			if (load_done == true) {
				scale_up_done = true;
				break;
			}
			continue;
		}
		image1 = queue_pop(load_queue);
		image_t* image2 = filter_scale_up(image1, 2);
		queue_push(scale_up_queue, image2);
		image_destroy(image1);
	}
	printf("\n Fin de thread_scale_up\n");
	pthread_exit(NULL);
}

void *thread_sharpen(void *inutilise) {
	image_t* image2;
	image_t* image3;
	while(1) {
		if (scale_up_queue->used == 0) {
			if (scale_up_done == true) {
				sharpen_done = true;
				break;
			}
			continue;
		}
		image2 = queue_pop(scale_up_queue);
		image3 = filter_sharpen(image2);
		queue_push(sharpen_queue, image3);
		image_destroy(image2);
	}
	printf("\n Fin de thread_sharpen\n");
	pthread_exit(NULL);
}

void *thread_sobel(void *inutilise) {
	image_t* image3;
	image_t* image4;
	while(1) {
		if (sharpen_queue->used == 0) {
			if (sharpen_done == true) {
				sobel_done = true;
				break;
			}
			continue;
		}
		image3 = queue_pop(sharpen_queue);
		image4 = filter_sobel(image3);
		queue_push(sobel_queue, image4);
		image_destroy(image3);
	}
	printf("\n Fin de thread_sobel\n");
	pthread_exit(NULL);
}

void *thread_save(void *image_dir) {
	image_t* image4; 
	while(1) {
		if (sobel_queue->used == 0) {
			if (sobel_done == true)
				break;
			continue;
		}
		image4 = queue_pop(sobel_queue);
		image_dir_save(image_dir, image4);
		image_destroy(image4);
	}
	printf("\n Fin de thread_save\n");
	pthread_exit(NULL);
}

int pipeline_pthread(image_dir_t* image_dir) {
	pthread_t thLoad, 
			thScaleUp1, thScaleUp2, 
			thSharpen1, thSharpen2, 
			thSobel1, thSobel2, thSobel3, thSobel4, 
			thSave1, thSave2, thSave3, thSave4, thSave5;

	load_queue = queue_create(69120000); //100 images max (691200 bytes par image)
	scale_up_queue = queue_create(6912000);
	sharpen_queue = queue_create(6912000);
	sobel_queue = queue_create(6912000);

	pthread_create(&thLoad, NULL, thread_load, (void*) image_dir);
	pthread_create(&thScaleUp1, NULL, thread_scale_up, NULL);
	pthread_create(&thScaleUp2, NULL, thread_scale_up, NULL);
	pthread_create(&thSharpen1, NULL, thread_sharpen, NULL);
	pthread_create(&thSharpen2, NULL, thread_sharpen, NULL);
	pthread_create(&thSobel1, NULL, thread_sobel, NULL);
	pthread_create(&thSobel2, NULL, thread_sobel, NULL);
	pthread_create(&thSobel3, NULL, thread_sobel, NULL);
	pthread_create(&thSobel4, NULL, thread_sobel, NULL);
	pthread_create(&thSave1, NULL, thread_save, (void*) image_dir);
	pthread_create(&thSave2, NULL, thread_save, (void*) image_dir);
	pthread_create(&thSave3, NULL, thread_save, (void*) image_dir);
	pthread_create(&thSave4, NULL, thread_save, (void*) image_dir);
	pthread_create(&thSave5, NULL, thread_save, (void*) image_dir);

/*
*/
	pthread_join(thLoad, NULL);
	pthread_join(thScaleUp1, NULL);
	pthread_join(thScaleUp2, NULL);
	pthread_join(thSharpen1, NULL);
	pthread_join(thSharpen2, NULL);
	pthread_join(thSobel1, NULL);
	pthread_join(thSobel2, NULL);
	pthread_join(thSobel3, NULL);
	pthread_join(thSobel4, NULL);
	pthread_join(thSave1, NULL);
	pthread_join(thSave2, NULL);
	pthread_join(thSave3, NULL);
	pthread_join(thSave4, NULL);
	pthread_join(thSave5, NULL);
/*
Pratiquement la même performance entre 7 threads et les 14 threads actuels (10 secondes plus rapide seulement)
*/
	return 0;
}
