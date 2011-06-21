__kernel void
gauss9x9_rows(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[9] = { 
        01.f/256.f,
        08.f/256.f,
        28.f/256.f,
        56.f/256.f,
        70.f/256.f,
        56.f/256.f,
        28.f/256.f,
        08.f/256.f,
        01.f/256.f,
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;
    
    int2 outcoord = (int2)(get_global_id(0), get_global_id(1));
    float2 incoord = (float2)((float)get_global_id(0), (float)get_global_id(1));

    float4 c = 0.f;

    if (outcoord.x >= get_image_width(output_image)
            || outcoord.y >= get_image_height(output_image)) {
        return;
    }

    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-4.f, 0.f))) * mask[0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-3.f, 0.f))) * mask[1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-2.f, 0.f))) * mask[2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.f, 0.f))) * mask[3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.f, 0.f))) * mask[4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.f, 0.f))) * mask[5];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.f, 0.f))) * mask[6];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+3.f, 0.f))) * mask[7];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+4.f, 0.f))) * mask[8];

    /* barrier(CLK_LOCAL_MEM_FENCE); */ /* Normally not necessary */

    write_imageui(output_image, outcoord, convert_uint4(c));
}

__kernel void
gauss9x9_cols(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[9] = {
        01.f/256.f,
        08.f/256.f,
        28.f/256.f,
        56.f/256.f,
        70.f/256.f,
        56.f/256.f,
        28.f/256.f,
        08.f/256.f,
        01.f/256.f,
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int2 outcoord = (int2)(get_global_id(0), get_global_id(1));
    float2 incoord = (float2)((float)get_global_id(0), (float)get_global_id(1));

    float4 c = 0.f;

    if (outcoord.x >= get_image_width(output_image)
            || outcoord.y >= get_image_height(output_image)) {
        return;
    }

    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, -4.f))) * mask[0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, -3.f))) * mask[1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, -2.f))) * mask[2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, -1.f))) * mask[3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, +0.f))) * mask[4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, +1.f))) * mask[5];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, +2.f))) * mask[6];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, +3.f))) * mask[7];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(0.f, +4.f))) * mask[8];

    barrier(CLK_LOCAL_MEM_FENCE);

    write_imageui(output_image, outcoord, convert_uint4(c));
}

