/* Copyright (c) 2009-2012 Matthieu Volat and Remi Emonet. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
__kernel void
downsampledgauss5x5_cols(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    float mask[5][5] = {
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

    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -4.0f)) * mask[0][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -3.0f)) * mask[1][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, -2.0f)) * mask[2][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+3.0f, -1.0f)) * mask[3][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+4.0f, +0.0f)) * mask[4][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -3.0f)) * mask[0][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -2.0f)) * mask[1][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -1.0f)) * mask[2][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +0.0f)) * mask[3][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+3.0f, +1.0f)) * mask[4][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -2.0f)) * mask[0][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -1.0f)) * mask[1][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +0.0f)) * mask[2][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +1.0f)) * mask[3][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +2.0f)) * mask[4][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-3.0f, -1.0f)) * mask[0][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +0.0f)) * mask[1][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +1.0f)) * mask[2][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +2.0f)) * mask[3][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +3.0f)) * mask[4][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-4.0f, +0.0f)) * mask[0][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-3.0f, +1.0f)) * mask[1][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +2.0f)) * mask[2][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +3.0f)) * mask[3][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +4.0f)) * mask[4][4];

    /* barrier(CLK_LOCAL_MEM_FENCE); */ /* Normaly not necessary */

    write_imagef(output_image, outcoord, c);
}

__kernel void
downsampledgauss5x5_rows(
        __write_only image2d_t output_image, 
        __read_only image2d_t input_image)
{
    float mask[5][5] = {
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

    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -4.0f)) * mask[0][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -3.0f)) * mask[1][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, -2.0f)) * mask[2][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+3.0f, -1.0f)) * mask[3][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(+4.0f, +0.0f)) * mask[4][0];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -3.0f)) * mask[0][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, -2.0f)) * mask[1][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, -1.0f)) * mask[2][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +0.0f)) * mask[3][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(+3.0f, +1.0f)) * mask[4][1];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, -2.0f)) * mask[0][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, -1.0f)) * mask[1][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +0.0f)) * mask[2][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +1.0f)) * mask[3][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(+2.0f, +2.0f)) * mask[4][2];
    c += read_imagef(input_image, sampler, incoord+(float2)(-3.0f, -1.0f)) * mask[0][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +0.0f)) * mask[1][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +1.0f)) * mask[2][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +2.0f)) * mask[3][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(+1.0f, +3.0f)) * mask[4][3];
    c += read_imagef(input_image, sampler, incoord+(float2)(-4.0f, +0.0f)) * mask[0][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-3.0f, +1.0f)) * mask[1][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-2.0f, +2.0f)) * mask[2][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(-1.0f, +3.0f)) * mask[3][4];
    c += read_imagef(input_image, sampler, incoord+(float2)(+0.0f, +4.0f)) * mask[4][4];

    /* barrier(CLK_LOCAL_MEM_FENCE); */ /* Normaly not necessary */

    write_imagef(output_image, outcoord, c);
}

