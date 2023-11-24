// Image blur kernel using a simple box filter
__kernel void blur(__global uchar4* inputImage, __global uchar4* outputImage, const int width, const int height) {
    int2 gid = (int2)(get_global_id(0), get_global_id(1));

    int radius = 3; // Adjust this for the desired blur radius

    float4 pixel = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float total_effect = 0;
    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            int2 neighbor = gid + (int2)(x, y);

            if (neighbor.x >= 0 && neighbor.x < width && neighbor.y >= 0 && neighbor.y < height) {
                int index = neighbor.x + neighbor.y * width;
                uchar4 color = inputImage[index];

                float distance =  (x * x + y * y);
                distance += 1;
                float effect = (1/sqrt(distance));
                total_effect += effect;
                pixel += (float4)(color.x, color.y, color.z, color.w) * effect ;
            }
        }
    }

    pixel /= total_effect;

    int index = gid.x + gid.y * width;
    outputImage[index] = (uchar4)(pixel.x, pixel.y, pixel.z, pixel.w);
}
