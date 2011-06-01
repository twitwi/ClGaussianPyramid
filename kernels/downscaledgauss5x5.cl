__kernel void
downscaledgauss5x5(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    const float mask[5][5] = {
        3.90625e-03f, 1.56250e-02f, 2.34375e-02f, 1.56250e-02f, 3.90625e-03f, 
        1.56250e-02f, 6.25000e-02f, 9.37500e-02f, 6.25000e-02f, 1.56250e-02f, 
        2.34375e-02f, 9.37500e-02f, 1.40625e-01f, 9.37500e-02f, 2.34375e-02f, 
        1.56250e-02f, 6.25000e-02f, 9.37500e-02f, 6.25000e-02f, 1.56250e-02f, 
        3.90625e-03f, 1.56250e-02f, 2.34375e-02f, 1.56250e-02f, 3.90625e-03f,
    };

    const sampler_t sampler = 
        CLK_FILTER_LINEAR|CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE;

    int2 outcoord = (int2)(get_global_id(0), get_global_id(1));
    float2 incoord = (float2)((float)get_global_id(0)*2.f, (float)get_global_id(1)*2.f);

    float4 c = 0.f;

    if (outcoord.x >= get_image_width(output_image)
            || outcoord.y >= get_image_height(output_image)) {
        return;
    }

    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.5f, -1.5f))) * mask[0][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-0.5f, -1.5f))) * mask[1][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.5f, -1.5f))) * mask[2][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.5f, -1.5f))) * mask[3][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.5f, -1.5f))) * mask[4][0];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.5f, -0.5f))) * mask[0][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-0.5f, -0.5f))) * mask[1][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.5f, -0.5f))) * mask[2][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.5f, -0.5f))) * mask[3][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.5f, -0.5f))) * mask[4][1];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.5f, +0.5f))) * mask[0][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-0.5f, +0.5f))) * mask[1][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.5f, +0.5f))) * mask[2][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.5f, +0.5f))) * mask[3][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.5f, +0.5f))) * mask[4][2];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.5f, +1.5f))) * mask[0][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-0.5f, +1.5f))) * mask[1][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.5f, +1.5f))) * mask[2][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.5f, +1.5f))) * mask[3][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.5f, +1.5f))) * mask[4][3];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-1.5f, +2.5f))) * mask[0][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(-0.5f, +2.5f))) * mask[1][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+0.5f, +2.5f))) * mask[2][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+1.5f, +2.5f))) * mask[3][4];
    c += convert_float4(read_imageui(input_image, sampler, incoord+(float2)(+2.5f, +2.5f))) * mask[4][4];

    write_imageui(output_image, outcoord, convert_uint4(c));
}

