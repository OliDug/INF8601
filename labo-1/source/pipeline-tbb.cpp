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

int pipeline_tbb(image_dir_t* image_dir) {
    std::cout << "ok";

    parallel_pipeline(16, make_filter<void, image_t*>(tbb::filter::serial, [&](tbb::flow_control& fc) -> image_t* {
                              std::cout << "\n debut";
                              image_t* image;
                              image = image_dir_load_next((image_dir_t*)image_dir);
                              if (image == NULL) {
                                  fc.stop();
                              }
                              std::cout << ".";
                              return image;
                          }) 
                        //   & make_filter<image_t*, image_t*>(tbb::filter::serial, [&](image_t* image) {
                        //       std::cout << "deuxieme";
                        //       return 0;
                        //   })
                          & make_filter<image_t*, void>(tbb::filter::serial, [&](image_t* image) {
                              std::cout << "troisieme";
                              return 0;
                          })
                          );
    return -1;
}
