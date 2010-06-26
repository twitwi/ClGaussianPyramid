__kernel void
downscale(
        __write_only image2d_t downscaled_image, 
        __read_only image2d_t input_image)
{
    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x_in_output = get_global_id(0);
    int y_in_output = get_global_id(1);

    int x_in_input = x_in_output*2;
    int y_in_input = y_in_output*2;

    int4 c;

    c = read_imageui(input_image, sampler, (int2)(x_in_input, y_in_input));
    write_imageui(downscaled_image, (int2)(x_in_output, y_in_output), c);
}

