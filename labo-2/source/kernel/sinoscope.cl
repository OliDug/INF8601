#include "helpers.cl"

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

    value = (atan(value) - atan(-value)) / M_PI;
    value = (value + 1) * 100;

    pixel_t pixel;
    color_value(&pixel, value, interval, interval_inverse);

    int index = (i * 3) + (j * 3) * width;

    output[index + 0] = pixel.bytes[0];
    output[index + 1] = pixel.bytes[1];
    output[index + 2] = pixel.bytes[2];
    
    
}

