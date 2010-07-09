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
    
    for (j = 0; j < 5; j++) { /* Manual unloop? */
        for (i = 0; i < 5; i++) { /* Manual unloop? */
            c += convert_float4(read_imageui(input_image, sampler, (float2)(x-2.5+i, y+2.5+j))) * mask[i][j];
        }
    }

    write_imageui(convoluted_image, (int2)(x, y), convert_int4(c));
}

