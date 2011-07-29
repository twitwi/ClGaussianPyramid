__kernel void
downsampledgauss5x5_cols(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[5][5] = {
        01.f/256.f, 04.f/256.f, 06.f/256.f, 04.f/256.f, 01.f/256.f,
        04.f/256.f, 16.f/256.f, 24.f/256.f, 16.f/256.f, 04.f/256.f,
        06.f/256.f, 24.f/256.f, 36.f/256.f, 24.f/256.f, 06.f/256.f,
        04.f/256.f, 16.f/256.f, 24.f/256.f, 16.f/256.f, 04.f/256.f,
        01.f/256.f, 04.f/256.f, 06.f/256.f, 04.f/256.f, 01.f/256.f,
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int2 outcoord = (int2)(get_global_id(0), get_global_id(1));

    float2 incoord = (float2)(
        (float)(2 * get_global_id(0) + (get_global_id(1) & 0x1)),
        (float)get_global_id(1));

    float4 c = 0.f;

    if (outcoord.x >= get_image_width(output_image)
            || outcoord.y >= get_image_height(output_image)) {
        return;
    }

    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -2.0f)) * mask[0][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -2.0f)) * mask[1][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -2.0f)) * mask[2][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -2.0f)) * mask[3][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, -2.0f)) * mask[4][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -1.0f)) * mask[0][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -1.0f)) * mask[1][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -1.0f)) * mask[2][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -1.0f)) * mask[3][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, -1.0f)) * mask[4][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +0.0f)) * mask[0][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +0.0f)) * mask[1][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +0.0f)) * mask[2][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +0.0f)) * mask[3][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +0.0f)) * mask[4][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +1.0f)) * mask[0][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +1.0f)) * mask[1][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +1.0f)) * mask[2][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +1.0f)) * mask[3][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +1.0f)) * mask[4][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +2.0f)) * mask[0][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +2.0f)) * mask[1][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +2.0f)) * mask[2][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +2.0f)) * mask[3][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +2.0f)) * mask[4][4];

    /* barrier(CLK_LOCAL_MEM_FENCE); */ /* Normaly not necessary */

    write_imagef(output_image, outcoord, c);
}

__kernel void
downsampledgauss5x5_rows(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[5][5] = {
        01.f/256.f, 04.f/256.f, 06.f/256.f, 04.f/256.f, 01.f/256.f,
        04.f/256.f, 16.f/256.f, 24.f/256.f, 16.f/256.f, 04.f/256.f,
        06.f/256.f, 24.f/256.f, 36.f/256.f, 24.f/256.f, 06.f/256.f,
        04.f/256.f, 16.f/256.f, 24.f/256.f, 16.f/256.f, 04.f/256.f,
        01.f/256.f, 04.f/256.f, 06.f/256.f, 04.f/256.f, 01.f/256.f,
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int2 outcoord = (int2)(get_global_id(0), get_global_id(1));

    float2 incoord = (float2)(
        (float)get_global_id(0),
        (float)(2 * get_global_id(1)));

    float4 c = 0.f;

    if (outcoord.x >= get_image_width(output_image)
            || outcoord.y >= get_image_height(output_image)) {
        return;
    }

    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -4.0f)) * mask[0][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -3.0f)) * mask[0][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -2.0f)) * mask[0][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -1.0f)) * mask[0][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +0.0f)) * mask[0][5];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -3.0f)) * mask[1][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -2.0f)) * mask[1][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -1.0f)) * mask[1][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +0.0f)) * mask[1][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +1.0f)) * mask[1][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -2.0f)) * mask[2][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -1.0f)) * mask[2][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +0.0f)) * mask[2][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +1.0f)) * mask[2][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +2.0f)) * mask[2][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -1.0f)) * mask[3][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +0.0f)) * mask[3][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +1.0f)) * mask[3][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +2.0f)) * mask[3][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +3.0f)) * mask[3][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +0.0f)) * mask[4][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +1.0f)) * mask[4][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +2.0f)) * mask[4][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +3.0f)) * mask[4][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +4.0f)) * mask[4][4];

    /* barrier(CLK_LOCAL_MEM_FENCE); */ /* Normaly not necessary */

    write_imagef(output_image, outcoord, c);
}

