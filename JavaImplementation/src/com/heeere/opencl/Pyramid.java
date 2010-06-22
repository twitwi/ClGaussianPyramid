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
public class Pyramid {

    int w;
    int h;
    BufferedImage[] l;

    public Pyramid(BufferedImage[] l) {
        this.l = l;
        w = l[0].getWidth();
        h = l[0].getHeight();
    }

}
