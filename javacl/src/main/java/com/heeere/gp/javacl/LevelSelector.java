/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.heeere.gp.javacl;

import fr.prima.gsp.framework.ModuleParameter;
import fr.prima.gsp.framework.spi.AbstractModule;
import java.awt.image.BufferedImage;
import java.util.List;

/**
 *
 * @author twilight
 */
public class LevelSelector extends AbstractModule {

    @ModuleParameter
    public int level = 0;

    public void input(List<BufferedImage> levels) {
        if (level < levels.size()) {
            output(levels.get(level));
        } else {
            debug("Not enough levels: request=" + level + " but there is only size=" + levels.size() + " levels");
        }
    }

    private void output(BufferedImage im) {
        emitEvent(im);
    }

    private void debug(String message) {
        emitEvent(message);
    }
}
