/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.heeere.opencl;

import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ConvolveOp;
import java.awt.image.Kernel;
import java.awt.image.RescaleOp;
import java.util.Arrays;

/**
 *
 * @author twilight
 */
public class PyramidProcessor {

    public static interface Requirements {

        public BufferedImage importImage(BufferedImage im);

        public BufferedImage applyF(BufferedImage input);

        public BufferedImage applyFThenDownscale(BufferedImage input);
    }

    public static Pyramid processPyramid(BufferedImage im, Requirements o) {
        int w = im.getWidth();
        int h = im.getHeight();
        int levelCount = 4 + 2 * Integer.numberOfTrailingZeros(Integer.highestOneBit(Math.max(w, h) / 5));
        BufferedImage[] L = new BufferedImage[levelCount];
        System.out.println("Importing L0");
        L[0] = o.importImage(im);
        System.out.println("Building L1");
        L[1] = o.applyF(L[0]);
        int i = 1;
        while (w >= 5 || h >= 5) {
            System.out.println("Building L" + (i + 1));
            L[i + 1] = o.applyF(L[i]);
            System.out.println("Building L" + (i + 2));
            L[i + 2] = o.applyFThenDownscale(L[i + 1]);
            w = L[i + 2].getWidth();
            h = L[i + 2].getHeight();
            System.err.println(w + " " + h);
            i += 2;
        }
        System.err.println(Arrays.toString(L));
        return new Pyramid(L);
    }

    // some testing implementations

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


}
