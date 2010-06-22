/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.heeere.opencl;

import com.heeere.opencl.demo.ImageStackViewer;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ConvolveOp;
import java.awt.image.Kernel;
import java.awt.image.RescaleOp;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

/**
 *
 * @author twilight
 */
public class TestPyramidOnLena {

    public static void main(String[] args) throws IOException {
        String input = "../media/lena512color.png";
        BufferedImage im = ImageIO.read(new File(input));

        Pyramid py = PyramidProcessor.processPyramid(im, PyramidProcessor.java2dImplementation());

        ImageStackViewer viewer = new ImageStackViewer("Pyramid", true, true);
        for (BufferedImage i : py.l) {
            viewer.addImage(i);
        }
    }
    
}
