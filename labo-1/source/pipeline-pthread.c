#include <stdio.h>

#include <pthread.h>
#include "filter.h"
#include "pipeline.h"
#include "queue.h"

queue_t* load_queue;
queue_t* scale_up_queue;
queue_t* sharpen_queue;
queue_t* sobel_queue;

bool load_done = false;
bool scale_up_done = false;
bool sharpen_done = false;
bool sobel_done = false;

pthread_mutex_t mutex_load;
pthread_mutex_t mutex_scale_up;
pthread_mutex_t mutex_sharpen;
pthread_mutex_t mutex_sobel;

void* thread_load(void* image_dir) {
    image_t* image;
    while (1) {
        image = image_dir_load_next((image_dir_t*)image_dir);
        if (image == NULL) {
            break;
        }
		queue_push(load_queue, image);
	}
	printf("\n Fin de thread_load\n");
	pthread_exit(NULL);
}

void *thread_scale_up(void *inutilise) {
	image_t* image1;
	size_t queue_used;
	bool canStop = false;
	while(1) {
		pthread_mutex_lock(&load_queue->mutex);
		queue_used = load_queue->used;
		pthread_mutex_unlock(&load_queue->mutex);
		if (queue_used == 0) {
			pthread_mutex_lock(&mutex_load);
			canStop = load_done;
			pthread_mutex_unlock(&mutex_load);
			if (canStop == true) {
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
	size_t queue_used;
	bool canStop = false;
	while(1) {
		pthread_mutex_lock(&scale_up_queue->mutex);
		queue_used = scale_up_queue->used;
		pthread_mutex_unlock(&scale_up_queue->mutex);
		if (queue_used == 0) {
			pthread_mutex_lock(&mutex_scale_up);
			canStop = scale_up_done;
			pthread_mutex_unlock(&mutex_scale_up);
			if (canStop == true) {
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
	size_t queue_used;
	bool canStop = false;
	while(1) {
		pthread_mutex_lock(&sharpen_queue->mutex);
		queue_used = sharpen_queue->used;
		pthread_mutex_unlock(&sharpen_queue->mutex);
		if (queue_used == 0) {
			pthread_mutex_lock(&mutex_sharpen);
			canStop = sharpen_done;
			pthread_mutex_unlock(&mutex_sharpen);
			if (canStop == true) {
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
	size_t queue_used;
	bool canStop = false;
	while(1) {
		pthread_mutex_lock(&sobel_queue->mutex);
		queue_used = sobel_queue->used;
		pthread_mutex_unlock(&sobel_queue->mutex);
		if (queue_used == 0) {
			pthread_mutex_lock(&mutex_sobel);
			canStop = sobel_done;
			pthread_mutex_unlock(&mutex_sobel);
			if (canStop == true) {
				break;
			}
			continue;
		}
		image4 = queue_pop(sobel_queue);
		image_dir_save(image_dir, image4);
		image_destroy(image4);
		printf(".");
        fflush(stdout);
	}
	printf("\n Fin de thread_save\n");
	pthread_exit(NULL);
}

int pipeline_pthread(image_dir_t* image_dir) {
    int i = 0;

	const int num_thLoad    = 2;
	const int num_thScaleUp = 6;
	const int num_thSharpen = 15;
	const int num_thSobel   = 16;
	const int num_thSave    = 12;

    pthread_t thLoad[num_thLoad];
    pthread_t thScaleUp[num_thScaleUp];
    pthread_t thSharpen[num_thSharpen];
    pthread_t thSobel[num_thSobel];
    pthread_t thSave[num_thSave];

	pthread_mutex_init(&mutex_load, NULL);
	pthread_mutex_init(&mutex_scale_up, NULL);
	pthread_mutex_init(&mutex_sharpen, NULL);
	pthread_mutex_init(&mutex_sobel, NULL);

    load_queue     = queue_create(69120000);  // 100 images max (691200 bytes par image)
    scale_up_queue = queue_create(6912000);
    sharpen_queue  = queue_create(6912000);
    sobel_queue    = queue_create(6912000);

    for (i = 0; i < num_thLoad; i++) {
        pthread_create(&thLoad[i], NULL, thread_load, (void*)image_dir);
    }
    for (i = 0; i < num_thScaleUp; i++) {
        pthread_create(&thScaleUp[i], NULL, thread_scale_up, (void*)image_dir);
    }
    for (i = 0; i < num_thSharpen; i++) {
        pthread_create(&thSharpen[i], NULL, thread_sharpen, (void*)image_dir);
    }
    for (i = 0; i < num_thSobel; i++) {
        pthread_create(&thSobel[i], NULL, thread_sobel, (void*)image_dir);
    }
    for (i = 0; i < num_thSave; i++) {
        pthread_create(&thSave[i], NULL, thread_save, (void*)image_dir);
    }

    for (i = 0; i < num_thLoad; i++) {
        pthread_join(thLoad[i], NULL);
    }
	pthread_mutex_lock(&mutex_load);
	load_done = true;
	pthread_mutex_unlock(&mutex_load);

    for (i = 0; i < num_thScaleUp; i++) {
        pthread_join(thScaleUp[i], NULL);
    }
	pthread_mutex_lock(&mutex_scale_up);
	scale_up_done = true;
	pthread_mutex_unlock(&mutex_scale_up);

    for (i = 0; i < num_thSharpen; i++) {
        pthread_join(thSharpen[i], NULL);
    }
	pthread_mutex_lock(&mutex_sharpen);
	sharpen_done = true;
	pthread_mutex_unlock(&mutex_sharpen);

    for (i = 0; i < num_thSobel; i++) {
        pthread_join(thSobel[i], NULL);
    }
	pthread_mutex_lock(&mutex_sobel);
	sobel_done = true;
	pthread_mutex_unlock(&mutex_sobel);

    for (i = 0; i < num_thSave; i++) {
        pthread_join(thSave[i], NULL);
    }

    // Pratiquement la même performance entre 7 threads et les 14 threads actuels(10 secondes plus rapide seulement) * /
    return 0;
}
