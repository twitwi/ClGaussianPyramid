/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.heeere.gp.javacl;

import com.nativelibs4java.opencl.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTabbedPane;

/**
 *
 * @author twilight
 */
public class GaussianPyramidProcessor {

    public static void main(String[] args) throws IOException {
        GaussianPyramidProcessor gpp = new GaussianPyramidProcessor();


        showTabbed(gpp.computePyramid(ImageIO.read(new File("../media/lena512color.png"))), true);
        showTabbed(gpp.computeSqrt2Pyramid(ImageIO.read(new File("../media/lena512color.png"))), true);
        showTabbed(gpp.computePyramid(ImageIO.read(new File("../media/blurredbars.png"))), true);
        showTabbed(gpp.computePyramid(ImageIO.read(new File("../media/variablebars.png"))), true);
        showTabbed(gpp.computePyramid(ImageIO.read(new File("../media/dots.png"))), true);
        showTabbed(gpp.computeSqrt2Pyramid(ImageIO.read(new File("../media/dots.png"))), true);
        showTabbed(gpp.computePyramid(ImageIO.read(new File("../media/dotpairs.png"))), true);
        showTabbed(gpp.computeSqrt2Pyramid(ImageIO.read(new File("../media/dotpairs.png"))), true);
    }
    // Utilities

    public static JFrame showTabbed(List<BufferedImage> images, boolean exitOnClose) {
        JFrame f = new JFrame("LENA Pyramid");
        if (exitOnClose) {
            f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        }

        JTabbedPane content = new JTabbedPane();
        int l = 0;
        for (BufferedImage level : images) {
            content.add("" + l, new JLabel(new ImageIcon(level)));
            l++;
        }
        f.getContentPane().add("Center", content);
        f.pack();
        f.setVisible(true);
        return f;
    }
    // cached
    CLContext context;
    CLQueue queue;
    Gauss9x9 gauss9x9;
    Downsampledgauss5x5 downsampledgauss5x5;
    Downscaledgauss5x5 downscaledgauss5x5;

    public GaussianPyramidProcessor() throws IOException {
        context = JavaCL.createBestContext();
        queue = context.createDefaultQueue();
        gauss9x9 = new Gauss9x9(context);
        downsampledgauss5x5 = new Downsampledgauss5x5(context);
        downscaledgauss5x5 = new Downscaledgauss5x5(context);

    }

    // api
    public List<BufferedImage> computePyramid(BufferedImage im) {
        long start = System.currentTimeMillis();
        int width = im.getWidth();
        int height = im.getHeight();
        CLImage2D input = context.createImage2D(CLMem.Usage.InputOutput, im, false);

        int nLevels = 4 + 2 * Integer.numberOfTrailingZeros(Integer.highestOneBit(Math.max(width, height) / 5));
        List<BufferedImage> res = new ArrayList<BufferedImage>();
        List<CLImage2D> L = new ArrayList<CLImage2D>();
        CLImageFormat imageFormat = new CLImageFormat(CLImageFormat.ChannelOrder.BGRA, CLImageFormat.ChannelDataType.UNormInt8);

        L.add(input);
        for (int l = 1; l < nLevels; l++) {
            L.add(context.createImage2D(CLMem.Usage.InputOutput, imageFormat, levelWidth(width, l), levelHeight(height, l)));
        }

        { // doing it
            CLEvent last = null;

            last = queue9x9(L.get(0), L.get(1), last);
            int i = 1;
            while (i < nLevels - 2) {
                last = queue9x9(L.get(i), L.get(i + 1), last);
                last = queue9x9ThenDownscale(L.get(i + 1), L.get(i + 2), last);
                i += 2;
            }
            last.waitFor();
        }

        long afterGpu = System.currentTimeMillis();
        for (CLImage2D cLImage2D : L) {
            res.add(cLImage2D.read(queue));
            cLImage2D.release();
        }
        long now = System.currentTimeMillis();
        System.err.println("Processed in " + (now - start) + "ms (incl. " + (afterGpu - start) + " without the read)");
        return res;
    }

    public List<BufferedImage> computeSqrt2Pyramid(BufferedImage im) {
        long start = System.currentTimeMillis();
        int width = im.getWidth();
        int height = im.getHeight();
        CLImage2D input = context.createImage2D(CLMem.Usage.InputOutput, im, false);

        int nLevels = 4 + 2 * Integer.numberOfTrailingZeros(Integer.highestOneBit(Math.max(width, height) / 5));
        List<BufferedImage> res = new ArrayList<BufferedImage>();
        List<CLImage2D> L = new ArrayList<CLImage2D>();
        CLImageFormat imageFormat = new CLImageFormat(CLImageFormat.ChannelOrder.BGRA, CLImageFormat.ChannelDataType.UNormInt8);

        L.add(input);
        for (int l = 1; l < nLevels; l++) {
            L.add(context.createImage2D(CLMem.Usage.InputOutput, imageFormat, levelWidthSqrt2(width, l), levelHeightSqrt2(height, l)));
        }

        { // doing it
            CLEvent last = null;

            last = queue9x9(L.get(0), L.get(1), last);
            int i = 1;
            while (i < nLevels - 2) {
                last = queue5x5DownsampleCols(L.get(i), L.get(i + 1), last);
                last = queue5x5DownsampleRows(L.get(i + 1), L.get(i + 2), last);
                i += 2;
            }
            last.waitFor();
        }
        long afterGpu = System.currentTimeMillis();
        for (CLImage2D cLImage2D : L) {
            res.add(cLImage2D.read(queue));
            cLImage2D.release();
        }
        long now = System.currentTimeMillis();
        System.err.println("Processed in " + (now - start) + "ms (incl. " + (afterGpu - start) + " without the read)");
        return res;
    }

    private CLEvent queue9x9(CLImage2D input, CLImage2D output, CLEvent... previousToWaitFor) {
        int width = (int) input.getWidth();
        int height = (int) input.getHeight();
        int[] localWorkSize = new int[]{(width >= 16) ? 16 : width, (height >= 16) ? 16 : height};
        int[] globalWorkSize = new int[]{((width - 1) / localWorkSize[0] + 1) * localWorkSize[0], ((height - 1) / localWorkSize[1] + 1) * localWorkSize[1]};
        CLEvent rowsProcess = gauss9x9.gauss9x9_rows(queue, output, input, globalWorkSize, localWorkSize, previousToWaitFor);
        CLEvent colsProcess = gauss9x9.gauss9x9_cols(queue, output, output, globalWorkSize, localWorkSize, rowsProcess);
        return colsProcess;
    }

    private CLEvent queue9x9ThenDownscale(CLImage2D input, CLImage2D output, CLEvent... previousToWaitFor) {
        int width = (int) input.getWidth();
        int height = (int) input.getHeight();
        int[] localWorkSize = new int[]{(width >= 32) ? 16 : width >> 1, (height >= 32) ? 16 : height >> 1};
        int[] globalWorkSize = new int[]{(((width >> 1) - 1) / localWorkSize[0] + 1) * localWorkSize[0], (((height >> 1) - 1) / localWorkSize[1] + 1) * localWorkSize[1]};
        CLEvent blockProcess = downscaledgauss5x5.downscaledgauss5x5(queue, output, input, globalWorkSize, localWorkSize, previousToWaitFor);
        return blockProcess;
    }

    private CLEvent queue5x5DownsampleCols(CLImage2D input, CLImage2D output, CLEvent... previousToWaitFor) {
        int width = (int) input.getWidth();
        int height = (int) input.getHeight();
        int[] localWorkSize = new int[]{(width >= 32) ? 16 : width >> 1, (height >= 32) ? 16 : height >> 1};
        int[] globalWorkSize = new int[]{(((width >> 1) - 1) / localWorkSize[0] + 1) * localWorkSize[0], ((height - 1) / localWorkSize[1] + 1) * localWorkSize[1]};
        CLEvent blockProcess = downsampledgauss5x5.downsampledgauss5x5_cols(queue, output, input, globalWorkSize, localWorkSize, previousToWaitFor);
        return blockProcess;
    }

    private CLEvent queue5x5DownsampleRows(CLImage2D input, CLImage2D output, CLEvent... previousToWaitFor) {
        int width = (int) input.getWidth();
        int height = (int) input.getHeight();
        int[] localWorkSize = new int[]{(width >= 32) ? 16 : width >> 1, (height >= 32) ? 16 : height >> 1};
        int[] globalWorkSize = new int[]{((width - 1) / localWorkSize[0] + 1) * localWorkSize[0], (((height >> 1) - 1) / localWorkSize[1] + 1) * localWorkSize[1]};
        CLEvent blockProcess = downsampledgauss5x5.downsampledgauss5x5_rows(queue, output, input, globalWorkSize, localWorkSize, previousToWaitFor);
        return blockProcess;
    }

    private int levelWidth(int base, int level) {
        return base >> ((level - 1) >> 1);
    }

    private int levelHeight(int base, int level) {
        return base >> ((level - 1) >> 1);
    }

    private int levelWidthSqrt2(int base, int level) {
        int res = base >> ((level - 1) >> 1);
        if (level % 2 == 0) {
            res >>= 1;
        }
        return res;
    }

    private int levelHeightSqrt2(int base, int level) {
        return base >> ((level - 1) >> 1);
    }
}
