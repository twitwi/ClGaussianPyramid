/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.heeere.opencl;

import com.heeere.opencl.PyramidProcessor.Requirements;
import java.awt.RenderingHints;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.ColorModel;
import java.awt.image.DataBufferInt;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import org.jocl.cl_kernel;
import org.jocl.cl_mem;
import org.jocl.cl_program;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ConvolveOp;
import java.awt.image.Kernel;
import java.awt.image.RescaleOp;
import org.jocl.Pointer;
import org.jocl.Sizeof;
import org.jocl.cl_command_queue;
import org.jocl.cl_context;
import org.jocl.cl_context_properties;
import org.jocl.cl_device_id;
import org.jocl.cl_platform_id;

import static org.jocl.CL.*;

/**
 *
 * @author twilight
 */
public class PyramidImpl {

    private static float[] norm(float[] kernelData) {
        float sum = 0;
        for (float f : kernelData) {
            sum += f;
        }
        for (int i = 0; i < kernelData.length; i++) {
            kernelData[i] /= sum;
        }
        return kernelData;
    }
    public static float[] kernel = norm(
            new float[]{
                1, 4, 6, 4, 1,
                4, 16, 24, 16, 4,
                6, 24, 36, 24, 6,
                4, 16, 24, 16, 4,
                1, 4, 6, 4, 1
            });

    public static Requirements java2dImplementation() {
        return new Requirements() {

            BufferedImageOp conv = new ConvolveOp(new Kernel(5, 5, kernel), ConvolveOp.EDGE_NO_OP, null);
            BufferedImageOp resize = new RescaleOp(0.5f, 0f, null);

            public BufferedImage applyF(BufferedImage input) {
                return conv.filter(input, null);
            }

            public BufferedImage applyFThenDownscale(BufferedImage input) {
                BufferedImage im = applyF(input);
                BufferedImage res = new BufferedImage(im.getWidth() / 2, im.getHeight() / 2, BufferedImage.TYPE_INT_RGB);
                Graphics2D g = res.createGraphics();
                g.drawImage(im, AffineTransform.getScaleInstance(0.5, 0.5), null);
                g.dispose();
                return res;
            }

            public BufferedImage importImage(BufferedImage im) {
                BufferedImage res = new BufferedImage(im.getWidth(), im.getHeight(), BufferedImage.TYPE_INT_RGB);
                Graphics g = res.createGraphics();
                g.drawImage(im, 0, 0, null);
                g.dispose();
                return res;

            }
        };
    }

    public static Requirements joclImplementation() {
        return new Requirements() {

            BufferedImageOp conv = JOCLConvolveOp.createNaiveConvolution(new Kernel(5, 5, kernel));
            BufferedImageOp resize = new RescaleOp(0.5f, 0f, null);

            public BufferedImage applyF(BufferedImage input) {
                return conv.filter(input, null);
            }

            public BufferedImage applyFThenDownscale(BufferedImage input) {
                BufferedImage im = applyF(input);
                BufferedImage res = new BufferedImage(im.getWidth() / 2, im.getHeight() / 2, BufferedImage.TYPE_INT_RGB);
                Graphics2D g = res.createGraphics();
                g.drawImage(im, AffineTransform.getScaleInstance(0.5, 0.5), null);
                g.dispose();
                return res;
            }

            public BufferedImage importImage(BufferedImage im) {
                BufferedImage res = new BufferedImage(im.getWidth(), im.getHeight(), BufferedImage.TYPE_INT_RGB);
                Graphics g = res.createGraphics();
                g.drawImage(im, 0, 0, null);
                g.dispose();
                return res;
            }
        };
    }
}
