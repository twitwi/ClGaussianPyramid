/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.heeere.opencl;

import com.heeere.opencl.demo.ImageStackViewer;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.Map;
import javax.imageio.ImageIO;

/**
 *
 * @author twilight
 */
public class TestPyramidOnLena {

    public static void main(String[] args) throws IOException {
        String input = "../media/lena512color.png";
        BufferedImage im = ImageIO.read(new File(input));

        Map<String, PyramidProcessor.Requirements> implementations = new LinkedHashMap<String, PyramidProcessor.Requirements>();
        implementations.put("warmup(Java2D)", PyramidImpl.java2dImplementation());
        implementations.put("Java2D", PyramidImpl.java2dImplementation());
        //implementations.put("warmup(JOCL)", PyramidImpl.joclImplementation());
        implementations.put("JOCL", PyramidImpl.joclImplementation());

        for (Map.Entry<String, PyramidProcessor.Requirements> e : implementations.entrySet()) {
            long start = System.nanoTime();
            Pyramid py = PyramidProcessor.processPyramid(im, e.getValue());
            long end = System.nanoTime();
            System.err.format("%s: %.3f ms\n", e.getKey(), ((end - start) / 1000000.f));

            ImageStackViewer viewer = new ImageStackViewer("Pyramid " + e.getKey(), true, true);
            for (BufferedImage i : py.l) {
                viewer.addImage(i);
            }
        }
    }
    
}
