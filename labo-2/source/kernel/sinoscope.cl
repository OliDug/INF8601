#include "helpers.cl"

<<<<<<< HEAD
__kernel void worker_id_example(int width, 
                                int height,
                                float dx,
                                float dy,
                                int taylor,
                                float phase0,
                                float phase1,
                                float time,
                                int interval,
                                float interval_inverse,
                                __global char* output)
    {
    int i = get_global_id(0);
    int j = get_global_id(1);

    float px    = dx * j - 2 * M_PI;
    float py    = dy * i - 2 * M_PI;
    float value = 0;

    for (int k = 1; k <= taylor; k += 2) {
        value += sin(px * k * phase1 + time) / k;
            value += cos(py * k * phase0) / k;
        }
=======
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

__kernel void worker_id_example(__global char* output, param_t parametre) {
    int i = get_global_id(0)/512;
    int j = get_global_id(0)%512;

    float px    = parametre.dx * j - 2 * M_PI;
    float py    = parametre.dy * i - 2 * M_PI;
    float value = 0;

    for (int k = 1; k <= parametre.taylor; k += 2) {
        value += sin(px * k * parametre.phase1 + parametre.time) / k;
        value += cos(py * k * parametre.phase0) / k;
     }
>>>>>>> refs/remotes/origin/main

    value = (atan(value) - atan(-value)) / M_PI;
    value = (value + 1) * 100;

    pixel_t pixel;
<<<<<<< HEAD
    color_value(&pixel, value, interval, interval_inverse);

    int index = (i * 3) + (j * 3) * width;
=======
    color_value(&pixel, value, parametre.interval, parametre.interval_inverse);

    int index = (i * 3) + (j * 3) * parametre.width;
>>>>>>> refs/remotes/origin/main

    output[index + 0] = pixel.bytes[0];
    output[index + 1] = pixel.bytes[1];
    output[index + 2] = pixel.bytes[2];
    
    
<<<<<<< HEAD
}

=======
}
>>>>>>> refs/remotes/origin/main
