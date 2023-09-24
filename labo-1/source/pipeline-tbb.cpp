#include <stdio.h>

extern "C" {
#include "filter.h"
#include "pipeline.h"
}

#include <iostream>
#include "tbb/pipeline.h"

int pipeline_tbb(image_dir_t* image_dir) {
    std::cout << "TBB";

    parallel_pipeline(48, 
        make_filter<void, image_t*>(tbb::filter::parallel, [&](tbb::flow_control& fc) -> image_t* {
            image_t* image;
            image = image_dir_load_next((image_dir_t*)image_dir);
            if (image == NULL) {
                fc.stop();
            }
            return image;
        })

        & make_filter<image_t*, image_t*>(tbb::filter::parallel, [](image_t* image) {
            // std::cout << "_scale_up";
            image_t* image2 = filter_scale_up(image, 2);
            image_destroy(image);
            return image2;
        })

        & make_filter<image_t*, image_t*>(tbb::filter::parallel, [](image_t* image) {
            // std::cout << "_sharpen";
            image_t* image3 = filter_sharpen(image);
            image_destroy(image);
            return image3;
        })

        & make_filter<image_t*, image_t*>(tbb::filter::parallel, [](image_t* image) {
            // std::cout << "_sobel";
            image_t* image4 = filter_sobel(image);
            image_destroy(image);
            return image4;
        })

        & make_filter<image_t*, void>(tbb::filter::parallel, [&](image_t* image) {
            image_dir_save(image_dir, image);
            // std::cout << "_save";
            std::cout << ".";
            image_destroy(image);
            return 0;
        })
    );
    return -1;
}
