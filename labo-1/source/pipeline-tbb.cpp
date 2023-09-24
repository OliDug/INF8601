#include <stdio.h>

extern "C" {
#include "filter.h"
#include "pipeline.h"
}

#include <iostream>
#include "tbb/pipeline.h"

float* GenerateCoucou(tbb::flow_control& fc) {
    while (true) {
        std::cout << "coucou" << std::endl;
        // Ajoutez un délai si nécessaire pour contrôler la fréquence d'affichage
        // tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(1));
        // if (fc.is_cancelled()) {
        //     return nullptr;
        // }
    }
}

/*
void RunPipeline( int ntoken, FILE* input_file, FILE* output_file ) {
    oneapi::tbb::parallel_pipeline(
        ntoken,
        oneapi::tbb::make_filter<void,TextSlice*>(
            oneapi::tbb::filter::serial_in_order, MyInputFunc(input_file) )
    &
        oneapi::tbb::make_filter<TextSlice*,TextSlice*>(
            oneapi::tbb::filter::parallel, MyTransformFunc() )
    &
        oneapi::tbb::make_filter<TextSlice*,void>(
            oneapi::tbb::filter::serial_in_order, MyOutputFunc(output_file) ) );
}
*/

int pipeline_tbb(image_dir_t* image_dir) {
    std::cout << "ok";

    tbb::parallel_pipeline(16, 
        make_filter<void, image_t*>(tbb::filter::serial_in_order, [&](tbb::flow_control& fc) -> image_t* {
            std::cout << "\n Début load";
            image_t* image;
            image = image_dir_load_next((image_dir_t*)image_dir);
            if (image == NULL) {
                fc.stop();
                return NULL;
            }
            std::cout << ".";
            return image;
        })
        & 
        make_filter<image_t*, image_t*>(tbb::filter::parallel, [&](image_t* image1) -> image_t* {
            std::cout << "\n Début scale up";
            if (image1 == NULL) {
                fc.stop();
                return NULL;
            }
            image_t* image2 = filter_scale_up(image1, 2);
            image_destroy(image1);
            return image2;
        })
        /*
        & 
        make_filter<image_t*, void>(tbb::filter::serial, [&](image_t* image) {
            std::cout << "troisieme";
            return 0;
        })
        */
    );
    return -1;
}

