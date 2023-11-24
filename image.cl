// Image blur kernel using a simple box filter
__kernel void blur(__global uchar4* inputImage, __global uchar4* outputImage, const int width, const int height) {
    int2 gid = (int2)(get_global_id(0), get_global_id(1));

    int radius = @radius@; // Adjust this for the desired blur radius

    float4 pixel = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
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
            uchar4 color = inputImage[index];

            float distance =  (x * x + y * y);
            distance += 1;
            float effect = (1/sqrt(distance));
            total_effect += effect;
            pixel += (float4)(color.x, color.y, color.z, color.w) * effect ;
        
        }
    }

    if(total_effect == 0){
        pixel = (float4)(255.f,0.f,255.f,255.f);
    }else{
        pixel /= total_effect;
    }

    int index = gid.x + gid.y * width;
    outputImage[index] = (uchar4)(pixel.x, pixel.y, pixel.z, pixel.w);
}
