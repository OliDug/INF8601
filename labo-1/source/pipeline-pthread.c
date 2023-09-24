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
	}
	// printf("\n Fin de thread_load\n");
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
		// printf("Scale-up");
	}
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
		// printf("Sharpen");
	}
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
		// printf("Sobel");
	}
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
		// printf("save");
		printf(".");
        fflush(stdout);
	}
	pthread_exit(NULL);
}

int pipeline_pthread(image_dir_t* image_dir) {
	int i = 0;
	int num_thLoad = 2;
	int num_thScaleUp = 6;
	int num_thSharpen = 12;
	int num_thSobel = 16;
	int num_thSave = 12;

	pthread_t thLoad[num_thLoad];
	pthread_t thScaleUp[num_thScaleUp];
	pthread_t thSharpen[num_thSharpen];
	pthread_t thSobel[num_thSobel];
	pthread_t thSave[num_thSave];

	load_queue = queue_create(69120000); //100 images max (691200 bytes par image)
	scale_up_queue = queue_create(6912000);
	sharpen_queue = queue_create(6912000);
	sobel_queue = queue_create(6912000);

	for (i = 0; i < num_thLoad; i++){
		pthread_create(&thLoad[i], NULL, thread_load, (void*) image_dir);
	}
	for (i = 0; i < num_thScaleUp; i++){
		pthread_create(&thScaleUp[i], NULL, thread_scale_up, (void*) image_dir);
	}
	for (i = 0; i < num_thSharpen; i++){
		pthread_create(&thSharpen[i], NULL, thread_sharpen, (void*) image_dir);
	}
	for (i = 0; i < num_thSobel; i++){
		pthread_create(&thSobel[i], NULL, thread_sobel, (void*) image_dir);
	}
	for (i = 0; i < num_thSave; i++){
		pthread_create(&thSave[i], NULL, thread_save, (void*) image_dir);
	}

	for (i = 0; i < num_thLoad; i++){
		pthread_join(thLoad[i], NULL);
	}
	for (i = 0; i < num_thScaleUp; i++){
		pthread_join(thScaleUp[i], NULL);
	}
	for (i = 0; i < num_thSharpen; i++){
		pthread_join(thSharpen[i], NULL);
	}
	for (i = 0; i < num_thSobel; i++){
		pthread_join(thSobel[i], NULL);
	}
	for (i = 0; i < num_thSave; i++){
		pthread_join(thSave[i], NULL);
	}


// 	pthread_t thLoad, 
// 			thScaleUp1, thScaleUp2, 
// 			thSharpen1, thSharpen2, 
// 			thSobel1, thSobel2, thSobel3, thSobel4, 
// 			thSave1, thSave2, thSave3, thSave4, thSave5;

// 	load_queue = queue_create(69120000); //100 images max (691200 bytes par image)
// 	scale_up_queue = queue_create(6912000);
// 	sharpen_queue = queue_create(6912000);
// 	sobel_queue = queue_create(6912000);

// 	pthread_create(&thLoad, NULL, thread_load, (void*) image_dir);
// 	pthread_create(&thScaleUp1, NULL, thread_scale_up, NULL);
// 	pthread_create(&thScaleUp2, NULL, thread_scale_up, NULL);
// 	pthread_create(&thSharpen1, NULL, thread_sharpen, NULL);
// 	pthread_create(&thSharpen2, NULL, thread_sharpen, NULL);
// 	pthread_create(&thSobel1, NULL, thread_sobel, NULL);
// 	pthread_create(&thSobel2, NULL, thread_sobel, NULL);
// 	pthread_create(&thSobel3, NULL, thread_sobel, NULL);
// 	pthread_create(&thSobel4, NULL, thread_sobel, NULL);
// 	pthread_create(&thSave1, NULL, thread_save, (void*) image_dir);
// 	pthread_create(&thSave2, NULL, thread_save, (void*) image_dir);
// 	pthread_create(&thSave3, NULL, thread_save, (void*) image_dir);
// 	pthread_create(&thSave4, NULL, thread_save, (void*) image_dir);
// 	pthread_create(&thSave5, NULL, thread_save, (void*) image_dir);

// /*
// */
// 	pthread_join(thLoad, NULL);
// 	pthread_join(thScaleUp1, NULL);
// 	pthread_join(thScaleUp2, NULL);
// 	pthread_join(thSharpen1, NULL);
// 	pthread_join(thSharpen2, NULL);
// 	pthread_join(thSobel1, NULL);
// 	pthread_join(thSobel2, NULL);
// 	pthread_join(thSobel3, NULL);
// 	pthread_join(thSobel4, NULL);
// 	pthread_join(thSave1, NULL);
// 	pthread_join(thSave2, NULL);
// 	pthread_join(thSave3, NULL);
// 	pthread_join(thSave4, NULL);
// 	pthread_join(thSave5, NULL);
/*
Pratiquement la même performance entre 7 threads et les 14 threads actuels (10 secondes plus rapide seulement)
*/
	return 0;
}

