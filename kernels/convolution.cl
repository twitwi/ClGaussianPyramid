__kernel void
convolution_rows(
        __write_only image2d_t output_image, 
        int output_origin_x,
        int output_origin_y,
        __read_only image2d_t input_image,
        int input_origin_x,
        int input_origin_y)
{
    const float mask[5] = { 
        0.062500f, 0.250000f, 0.375000f, 0.250000f, 0.062500f
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x_in_output = output_origin_x + get_global_id(0);
    int y_in_output = output_origin_y + get_global_id(1);

    float x_in_input = (float)(input_origin_x + get_global_id(0));
    float y_in_input = (float)(input_origin_y + get_global_id(1));

    float4 c = 0.f;

    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-2.f, y_in_input))) * mask[0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.f, y_in_input))) * mask[1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.f, y_in_input))) * mask[2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+1.f, y_in_input))) * mask[3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.f, y_in_input))) * mask[4];

    barrier(CLK_LOCAL_MEM_FENCE);

    write_imageui(output_image, (int2)(x_in_output, y_in_output), convert_int4(c));
}

__kernel void
convolution_cols(
        __write_only image2d_t output_image, 
        int output_origin_x,
        int output_origin_y,
        __read_only image2d_t input_image,
        int input_origin_x,
        int input_origin_y)
{
    const float mask[5] = {
        0.062500f, 0.250000f, 0.375000f, 0.250000f, 0.062500f
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x_in_output = output_origin_x + get_global_id(0);
    int y_in_output = output_origin_y + get_global_id(1);

    float x_in_input = (float)(input_origin_x + get_global_id(0));
    float y_in_input = (float)(input_origin_y + get_global_id(1));

    float4 c = 0.f;

    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input-2.f))) * mask[0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input-1.f))) * mask[1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+0.f))) * mask[2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+1.f))) * mask[3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+2.f))) * mask[4];

    barrier(CLK_LOCAL_MEM_FENCE);

    write_imageui(output_image, (int2)(x_in_output, y_in_output), convert_int4(c));
}

