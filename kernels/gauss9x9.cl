__kernel void
gauss9x9_rows(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[9] = { 
        3.906250e-03f,
        3.125000e-02f,
        1.093750e-01f,
        2.187500e-01f,
        2.734375e-01f,
        2.187500e-01f,
        1.093750e-01f,
        3.125000e-02f,
        3.906250e-03f
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x_in_output = get_global_id(0);
    int y_in_output = get_global_id(1);

    float x_in_input = (float)get_global_id(0);
    float y_in_input = (float)get_global_id(1);

    float4 c = 0.f;

    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-4.f, y_in_input))) * mask[0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-3.f, y_in_input))) * mask[1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-2.f, y_in_input))) * mask[2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.f, y_in_input))) * mask[3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.f, y_in_input))) * mask[4];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+1.f, y_in_input))) * mask[5];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.f, y_in_input))) * mask[6];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+3.f, y_in_input))) * mask[7];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+4.f, y_in_input))) * mask[8];

    /* barrier(CLK_LOCAL_MEM_FENCE); */ /* Normally not necessary */

    if (x_in_output < get_image_width(output_image)
            && y_in_output < get_image_height(output_image)) {
        write_imageui(
                output_image, 
                (int2)(x_in_output, y_in_output), 
                convert_uint4(c));
    }
}

__kernel void
gauss9x9_cols(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[9] = {
        3.906250e-03f,
        3.125000e-02f,
        1.093750e-01f,
        2.187500e-01f,
        2.734375e-01f,
        2.187500e-01f,
        1.093750e-01f,
        3.125000e-02f,
        3.906250e-03f
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x_in_output = get_global_id(0);
    int y_in_output = get_global_id(1);

    float x_in_input = (float)get_global_id(0);
    float y_in_input = (float)get_global_id(1);

    float4 c = 0.f;

    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input-4.f))) * mask[0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input-3.f))) * mask[1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input-2.f))) * mask[2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input-1.f))) * mask[3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+0.f))) * mask[4];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+1.f))) * mask[5];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+2.f))) * mask[6];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+3.f))) * mask[7];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input, y_in_input+4.f))) * mask[8];

    barrier(CLK_LOCAL_MEM_FENCE);

    if (x_in_output < get_image_width(output_image)
            && y_in_output < get_image_height(output_image)) {
        write_imageui(
                output_image, 
                (int2)(x_in_output, y_in_output), 
                convert_uint4(c));
    }
}

