#include <stdio.h>

#include <pthread.h>
#include "filter.h"
#include "pipeline.h"
#include "queue.h"

queue_t* load_queue;
queue_t* scale_up_queue;
queue_t* sharpen_queue;
queue_t* sobel_queue;

void* thread_load(void* image_dir) {
    image_t* image;
    while (1) {
        image = image_dir_load_next((image_dir_t*)image_dir);
		queue_push(load_queue, image);
        if (image == NULL) {
            break;
        }
	}
	pthread_exit(NULL);
}

void *thread_scale_up(void *inutilise) {
	image_t* image1;
	
	while(1) {
		image1 = queue_pop(load_queue);
		if (image1 == NULL) {
			queue_push(load_queue, image1);
			break;
		}
		image_t* image2 = filter_scale_up(image1, 2);
		queue_push(scale_up_queue, image2);
		image_destroy(image1);
	}
	pthread_exit(NULL);
}

void *thread_sharpen(void *inutilise) {
	image_t* image2;
	image_t* image3;
	while(1) {
		image2 = queue_pop(scale_up_queue);
		if (image2 == NULL) {
			printf("Sharpen got NULL\n");
			queue_push(scale_up_queue, image2);
			break;
		}
		image3 = filter_sharpen(image2);
		queue_push(sharpen_queue, image3);
		image_destroy(image2);
	}
	pthread_exit(NULL);
}

void *thread_sobel(void *inutilise) {
	image_t* image3;
	image_t* image4;
	while(1) {
		image3 = queue_pop(sharpen_queue);
		if (image3 == NULL) {
			queue_push(sharpen_queue, image3);
			break;
		}
		image4 = filter_sobel(image3);
		queue_push(sobel_queue, image4);
		image_destroy(image3);
	}
	pthread_exit(NULL);
}

void *thread_save(void *image_dir) {
	image_t* image4;
	while(1) {
		image4 = queue_pop(sobel_queue);
		if (image4 == NULL) {
			queue_push(sobel_queue, image4);
			break;
		}
		image_dir_save(image_dir, image4);
		image_destroy(image4);
		printf(".");
        fflush(stdout);
	}
	pthread_exit(NULL);
}

int pipeline_pthread(image_dir_t* image_dir) {
    int i = 0;

	const int num_thLoad    = 1;
	const int num_thScaleUp = 13;
	const int num_thSharpen = 26;
	const int num_thSobel   = 32;
	const int num_thSave    = 24;

    pthread_t thLoad[num_thLoad];
    pthread_t thScaleUp[num_thScaleUp];
    pthread_t thSharpen[num_thSharpen];
    pthread_t thSobel[num_thSobel];
    pthread_t thSave[num_thSave];

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
		printf("Fin %de thread load\n", i+1);
    }

    for (i = 0; i < num_thScaleUp; i++) {
        pthread_join(thScaleUp[i], NULL);
		printf("Fin %de thread scale up\n", i+1);
    }
	queue_push(scale_up_queue, NULL);

    for (i = 0; i < num_thSharpen; i++) {
        pthread_join(thSharpen[i], NULL);
		printf("Fin %de thread sharpen\n", i+1);
    }
	queue_push(sharpen_queue, NULL);

    for (i = 0; i < num_thSobel; i++) {
        pthread_join(thSobel[i], NULL);
		printf("Fin %de thread sobel\n", i+1);
    }
	queue_push(sobel_queue, NULL);

    for (i = 0; i < num_thSave; i++) {
        pthread_join(thSave[i], NULL);
		printf("Fin %de thread save\n", i+1);
    }

    return 0;
}
