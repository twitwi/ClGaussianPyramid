/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.heeere.gp.javacl;

import fr.prima.gsp.framework.ModuleParameter;
import fr.prima.gsp.framework.spi.AbstractModuleEnablable;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author twilight
 */
public class GaussianPyramid extends AbstractModuleEnablable {

    @ModuleParameter
    public boolean lazy = true;
    @ModuleParameter
    public boolean useSqrt2 = false;
    //
    private GaussianPyramidProcessor processor = null;
    private boolean forgetAboutIt = false;

    @Override
    protected void initModule() {
        if (!lazy) {
            initProcessor();
        }
    }

    private void initProcessor() {
        if (forgetAboutIt || processor != null) {
            return;
        }
        try {
            processor = new GaussianPyramidProcessor();
        } catch (IOException ex) {
            Logger.getLogger(GaussianPyramid.class.getName()).log(Level.SEVERE, null, ex);
            forgetAboutIt = true;
        }
    }

    public void input(BufferedImage im) {
        initProcessor();
        if (!enabled || processor == null) {
            return;
        }
        // TODO: should use some Future<BufferedImage> and also output the raw CL pointers
        if (useSqrt2) {
            output(processor.computeSqrt2Pyramid(im));
        } else {
            output(processor.computePyramid(im));
        }
    }

    private void output(List<BufferedImage> pyramidLevels) {
        emitEvent(pyramidLevels);
    }
}
