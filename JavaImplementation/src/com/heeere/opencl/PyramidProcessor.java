/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.heeere.opencl;

import java.awt.image.BufferedImage;

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
        //System.out.println("Importing L0");
        L[0] = o.importImage(im);
        //System.out.println("Building L1");
        L[1] = o.applyF(L[0]);
        int i = 1;
        while (w >= 5 || h >= 5) {
            //System.out.println("Building L" + (i + 1));
            L[i + 1] = o.applyF(L[i]);
            //System.out.println("Building L" + (i + 2));
            L[i + 2] = o.applyFThenDownscale(L[i + 1]);
            w = L[i + 2].getWidth();
            h = L[i + 2].getHeight();
            //System.err.println(w + " " + h);
            i += 2;
        }
        //System.err.println(Arrays.toString(L));
        return new Pyramid(L);
    }
}

