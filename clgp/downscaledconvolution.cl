__kernel void
downscaledconvolution(
        __write_only image2d_t output_image, 
        int output_origin_x,
        int output_origin_y,
        __read_only image2d_t input_image,
        int input_origin_x,
        int input_origin_y)
{
    const float mask[5][5] = {
        3.90625e-03f, 1.56250e-02f, 2.34375e-02f, 1.56250e-02f, 3.90625e-03f, 
        1.56250e-02f, 6.25000e-02f, 9.37500e-02f, 6.25000e-02f, 1.56250e-02f, 
        2.34375e-02f, 9.37500e-02f, 1.40625e-01f, 9.37500e-02f, 2.34375e-02f, 
        1.56250e-02f, 6.25000e-02f, 9.37500e-02f, 6.25000e-02f, 1.56250e-02f, 
        3.90625e-03f, 1.56250e-02f, 2.34375e-02f, 1.56250e-02f, 3.90625e-03f,
    };

    const sampler_t sampler = 
        CLK_FILTER_NEAREST|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int x_in_output = output_origin_x + get_global_id(0);
    int y_in_output = output_origin_y + get_global_id(1);

    float x_in_input = (float)(input_origin_x + get_global_id(0)*2);
    float y_in_input = (float)(input_origin_y + get_global_id(1)*2);

    float4 c = 0.f;

    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-3.5f, y_in_input-3.5f))) * mask[0][0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.5f, y_in_input-3.5f))) * mask[1][0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.5f, y_in_input-3.5f))) * mask[2][0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.5f, y_in_input-3.5f))) * mask[3][0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+4.5f, y_in_input-3.5f))) * mask[4][0];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-3.5f, y_in_input-1.5f))) * mask[0][1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.5f, y_in_input-1.5f))) * mask[1][1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.5f, y_in_input-1.5f))) * mask[2][1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.5f, y_in_input-1.5f))) * mask[3][1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+4.5f, y_in_input-1.5f))) * mask[4][1];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-3.5f, y_in_input+0.5f))) * mask[0][2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.5f, y_in_input+0.5f))) * mask[1][2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.5f, y_in_input+0.5f))) * mask[2][2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.5f, y_in_input+0.5f))) * mask[3][2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+4.5f, y_in_input+0.5f))) * mask[4][2];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-3.5f, y_in_input+2.5f))) * mask[0][3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.5f, y_in_input+2.5f))) * mask[1][3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.5f, y_in_input+2.5f))) * mask[2][3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.5f, y_in_input+2.5f))) * mask[3][3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+4.5f, y_in_input+2.5f))) * mask[4][3];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-3.5f, y_in_input+4.5f))) * mask[0][4];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input-1.5f, y_in_input+4.5f))) * mask[1][4];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+0.5f, y_in_input+4.5f))) * mask[2][4];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+2.5f, y_in_input+4.5f))) * mask[3][4];
    c += convert_float4(read_imageui(input_image, sampler, (float2)(x_in_input+4.5f, y_in_input+4.5f))) * mask[4][4];

    barrier(CLK_LOCAL_MEM_FENCE);

    write_imageui(output_image, (int2)(x_in_output, y_in_output), convert_int4(c));
}

