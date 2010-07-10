__kernel void
convolution(
        __write_only image2d_t convoluted_image, 
        __read_only image2d_t input_image)
{
/*
    const float mask[5][5] = {
        01.f, 04.f, 06.f, 04.f, 01.f,
        04.f, 16.f, 24.f, 16.f, 04.f,
        06.f, 24.f, 36.f, 24.f, 06.f,
        04.f, 16.f, 24.f, 16.f, 04.f,
        01.f, 04.f, 06.f, 04.f, 01.f
    };
*/
    const float mask[5][5] = {
        0.003906, 0.015625, 0.023438, 0.015625, 0.003906, 
        0.015625, 0.062500, 0.093750, 0.062500, 0.015625, 
        0.023438, 0.093750, 0.140625, 0.093750, 0.023438, 
        0.015625, 0.062500, 0.093750, 0.062500, 0.015625, 
        0.003906, 0.015625, 0.023438, 0.015625, 0.003906
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x = get_global_id(0);
    int y = get_global_id(1);
    int i, j;

    float4 c = (float4)0.f;

    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-2, y-2))) * mask[0][0];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-1, y-2))) * mask[1][0];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+0, y-2))) * mask[2][0];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+1, y-2))) * mask[3][0];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+2, y-2))) * mask[4][0];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-2, y-1))) * mask[0][1];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-1, y-1))) * mask[1][1];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+0, y-1))) * mask[2][1];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+1, y-1))) * mask[3][1];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+2, y-1))) * mask[4][1];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-2, y+0))) * mask[0][2];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-1, y+0))) * mask[1][2];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+0, y+0))) * mask[2][2];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+1, y+0))) * mask[3][2];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+2, y+0))) * mask[4][2];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-2, y+1))) * mask[0][3];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-1, y+1))) * mask[1][3];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+0, y+1))) * mask[2][3];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+1, y+1))) * mask[3][3];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+2, y+1))) * mask[4][3];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-2, y+2))) * mask[0][4];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x-1, y+2))) * mask[1][4];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+0, y+2))) * mask[2][4];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+1, y+2))) * mask[3][4];
    c += convert_float4(read_imageui(input_image, sampler, (int2)(x+2, y+2))) * mask[4][4];

    write_imageui(convoluted_image, (int2)(x, y), convert_int4(c));
}

