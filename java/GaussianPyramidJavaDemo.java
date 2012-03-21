import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

import java.io.IOException;
import java.io.File;
import javax.imageio.ImageIO;

import org.jocl.NativePointerObject;
import org.jocl.Pointer;
import org.jocl.cl_command_queue;
import org.jocl.cl_context;
import org.jocl.cl_device_id;
import org.jocl.cl_image_format;
import org.jocl.cl_mem;
import org.jocl.cl_platform_id;
import static org.jocl.CL.*;

import static clgp.CLGP.*;

import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

class GaussianPyramidJavaDemo {
    private static long pyramidLayoutOriginX(long level, long width, long height) {
        if (level == 0) {
            return 0;
        }
        else {
            return Double.valueOf(((1. - java.lang.Math.pow(0.5, Long.valueOf(level).doubleValue())) / (1.-0.5)) * Long.valueOf(width).doubleValue()).longValue();
        }
    }
    private static long pyramidLayoutOriginY(long level, long width, long height) {
         return 0;
    }
    
    public static void main(String[] args) {
        int[] err = { CL_SUCCESS };

        cl_platform_id[] platforms = new cl_platform_id[8];
        int[] nplatforms = new int[1]; 
	cl_device_id[] devices = new cl_device_id[16];
        int[] ndevices = new int[1];
        cl_context context = null;
        cl_command_queue queue = null;
        NativePointerObject clgpkernels = null;

        BufferedImage input_image = null, pyramid_image = null;
	int[] input_data = null;
        int[] pyramid_data = null;
        long input_width = 0, pyramid_width = 0;
        long input_height = 0, pyramid_height = 0;

        cl_image_format[] imageformat = { new cl_image_format() };
        imageformat[0].image_channel_order = CL_RGBA;
        imageformat[0].image_channel_data_type = CL_UNORM_INT8;
        cl_mem input_climage;
        cl_mem[] pyramid_climages = new cl_mem[32];
        long maxlevel = 0;


        // OpenCL init
        clGetPlatformIDs(8, platforms, nplatforms);
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 16, devices, ndevices);
        if (devices[0] == null) {
            throw new RuntimeException("No device available");
        }

	// Create a context on this device
        context = clCreateContext(null, 1, devices, null, null, err);
        if (err[0] != CL_SUCCESS) {
            throw new RuntimeException("Could not create a context for the device");
        }

        // Create a command queue 
        queue = clCreateCommandQueue(context, devices[0], 0, err);
        if (err[0] != CL_SUCCESS) {
            throw new RuntimeException("Could not create a command queue for the device");
        }

        // Initialize the clgp library
        clgpkernels = clgpInit(context, err);
        if (err[0] != CL_SUCCESS) {
            throw new RuntimeException("Could not init clgp library");
        }

        // Load image
        try {
	    File input_file = new File(args[0]);
            BufferedImage tmpimage = ImageIO.read(input_file);

            input_image = 
                new BufferedImage(
                        tmpimage.getWidth(), 
                        tmpimage.getHeight(), 
                        BufferedImage.TYPE_INT_RGB);
            Graphics input_image_graphics = input_image.createGraphics();
            input_image_graphics.drawImage(tmpimage, 0, 0, null);
            input_image_graphics.dispose();
        }
        catch (IOException e) {
            throw new RuntimeException("Could not load input image");
        }
        input_data = 
            ((DataBufferInt)input_image.getRaster().getDataBuffer()).getData();
        input_width = (long)input_image.getWidth();
        input_height = (long)input_image.getHeight();

        // Create pyramid image
        pyramid_image = 
            new BufferedImage(
                    input_image.getWidth()*2, 
                    input_image.getHeight(), 
                    BufferedImage.TYPE_INT_RGB);
        pyramid_data = 
            ((DataBufferInt)pyramid_image.getRaster().getDataBuffer()).getData();
        pyramid_width = 2*input_width;
        pyramid_height = input_height;

        // Create buffers on device
        input_climage = 
            clCreateImage2D(
                    context,
                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    imageformat,
                    input_width,
                    input_height,
                    0,
                    Pointer.to(input_data),
                    err);
        if (err[0] != CL_SUCCESS) {
            throw new RuntimeException("Could not allocate input_climage");
        }
	maxlevel = clgpMaxlevel(input_width, input_height) - 4;
        for (long level = 0; level < maxlevel; level++) {
            pyramid_climages[(int)level] =
                clCreateImage2D(
                        context,
                        CL_MEM_READ_WRITE,
                        imageformat,
                        input_width >> level,
                        input_height >> level,
                        0,
                        null,
                        err);
            if (err[0] != CL_SUCCESS) {
                throw new RuntimeException("Could not  allocate pyramid_climages["+level+"]");
            }
        }

        // At last, call our pyramid function
        err[0] =
            clgpEnqueuePyramid(
                    queue,
                    clgpkernels,
                    pyramid_climages,
                    input_climage,
                    maxlevel);
        if (err[0] != CL_SUCCESS) {
            throw new RuntimeException("Pyramid failed");
        }

        // Retrieve images
        for (long level = 0; level < maxlevel; level++) {
            err[0] = 
                clEnqueueReadImage(
                        queue,
                        pyramid_climages[(int)level],
                        CL_TRUE,
                        new long[]{ 0, 0, 0 },
                        new long[]{ input_width>>level, input_height>>level, 1 },
                        pyramid_width*4,
                        0,
                        Pointer.to(pyramid_data).withByteOffset(pyramidLayoutOriginY(level, input_width, input_height)*pyramid_width*4 + pyramidLayoutOriginX(level, input_width, input_height)*4),
                        0,
                        null,
                        null);
            if (err[0] != CL_SUCCESS) {
                throw new RuntimeException("Could not copy pyramid data on host");
            }
        }
        clFinish(queue);

        // Release device ressources
        clReleaseMemObject(input_climage);
        for (long level = 0; level < maxlevel; level++) {
            clReleaseMemObject(pyramid_climages[(int)level]);
        }

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

        // Display
        JFrame frame = new JFrame("Gaussian Pyramid Java Demo");
        JLabel label = new JLabel(new ImageIcon(pyramid_image));
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.add(label);
        frame.pack();
        frame.setVisible(true);
    }
}

