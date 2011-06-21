__kernel void
downscaledgauss5x5(
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
    float2 incoord = (float2)((float)get_global_id(0)*2.f, (float)get_global_id(1)*2.f);

    float4 c = 0.f;

    if (outcoord.x >= get_image_width(output_image)
            || outcoord.y >= get_image_height(output_image)) {
        return;
    }

    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-2.f, -2.f))) * mask[0][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.f, -2.f))) * mask[1][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.f, -2.f))) * mask[2][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.f, -2.f))) * mask[3][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.f, -2.f))) * mask[4][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-2.f, -1.f))) * mask[0][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.f, -1.f))) * mask[1][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.f, -1.f))) * mask[2][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.f, -1.f))) * mask[3][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.f, -1.f))) * mask[4][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-2.f, +0.f))) * mask[0][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.f, +0.f))) * mask[1][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.f, +0.f))) * mask[2][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.f, +0.f))) * mask[3][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.f, +0.f))) * mask[4][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-2.f, +1.f))) * mask[0][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.f, +1.f))) * mask[1][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.f, +1.f))) * mask[2][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.f, +1.f))) * mask[3][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.f, +1.f))) * mask[4][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-2.f, +2.f))) * mask[0][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.f, +2.f))) * mask[1][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.f, +2.f))) * mask[2][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.f, +2.f))) * mask[3][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.f, +2.f))) * mask[4][4];

    write_imageui(output_image, outcoord, convert_uint4(c));
}

