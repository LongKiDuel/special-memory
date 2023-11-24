// Image blur kernel using a simple box filter
__kernel void blur(__global uchar* inputImage, __global uchar* outputImage, const int width, const int height) {
    int2 gid = (int2)(get_global_id(0), get_global_id(1));

    int radius = @radius@; // Adjust this for the desired blur radius

    float pixel = 0;
    float total_effect = 0;
    for (int x = -radius; x <= radius; ++x) {
        int nx = gid.x + x;
        if (nx < 0 || nx >= width){
            continue;
        }
        for (int y = -radius; y <= radius; ++y) {
            int ny = gid.y + y; 
            if(ny<0 || ny >= height){
                continue;
            }

            int index = nx + ny * width;
            uchar color = inputImage[index];

            float distance =  (x * x + y * y);
            distance += 1;
            float effect = (1/sqrt(distance));
            total_effect += effect;
            pixel += (float)(color) * effect ;
        
        }
    }

    pixel /= total_effect;

    int index = gid.x + gid.y * width;
    outputImage[index] = pixel;
}
