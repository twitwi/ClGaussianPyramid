/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.heeere.opencl.demo;

import java.awt.BorderLayout;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.IndexColorModel;
import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.filechooser.FileFilter;

/**
 *
 */
public class ImageStackViewer {

    public static interface Click {
        void imageClicked(int imageIndex, Point2D at);
    }
    public Click imageClickListener;


    public static void quicklyShowDebugImage(BufferedImage image) {
        final BlockingQueue<String> event = new ArrayBlockingQueue<String>(1);
        ImageStackViewer isv = new ImageStackViewer("Debug", true, false);
        isv.setThresholdingEnabled(false);
        isv.f.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                event.add("Here");
            }
        });
        isv.addImage(image);
        try {
            event.take();
        } catch (InterruptedException ex) {
            Logger.getLogger(ImageStackViewer.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    JFrame f;
    boolean scaleDisplay;
    JSlider slider;
    JPanel controlPanel;
    JCheckBox scaleDisplayCheckbox;
    JCheckBox thresholdingEnabledCheckbox;
    JSlider thresholdingSlider;
    JLabel thresholdLabel;
    ImageDisplay imageContainer;
    ImageDisplay imageContainer2;
    boolean inBiMode;
    
    List<Image> images = new Vector<Image>();
    boolean thresholdingEnabled = false;
    int threshold = -1;
    String baseTitle;

    public ImageStackViewer(String title, boolean initialScaleDisplay, boolean exitOnClose) {
        this(title, initialScaleDisplay, exitOnClose, null);
    }
    public ImageStackViewer(String title, boolean initialScaleDisplay, boolean exitOnClose, Integer boxLayoutOrientationOrNull) {
        this.inBiMode = boxLayoutOrientationOrNull != null;
        baseTitle = title;
        // init gui components
        f = new JFrame(title);
        f.setLayout(new BorderLayout());
        
        imageContainer = new ImageDisplay();
        imageContainer2 = new ImageDisplay();
        for (final ImageDisplay i : new ImageDisplay[]{imageContainer, imageContainer2}) {
            i.addMouseListener(new MouseAdapter() {

                @Override
                public void mousePressed(MouseEvent e) {
                    if (e.getButton() == MouseEvent.BUTTON1) {
                        slider.requestFocus();
                        if (imageClickListener != null) {
                            int imageIndex = slider.getValue() + (i == imageContainer2 ? 1 : 0);
                            imageClickListener.imageClicked(imageIndex, i.getImageCoordinates(e));
                        }
                    } else {
                        exportCurrentImage();
                    }
                }
            });
            i.addMouseWheelListener(new MouseAdapter() {

                @Override
                public void mouseWheelMoved(MouseWheelEvent e) {
                    if (e.getWheelRotation() < 0) {
                        switchToImage(Math.min(slider.getValue() + 1, slider.getMaximum() - 1));
                    } else {
                        switchToImage(Math.max(slider.getValue() - 1, slider.getMinimum()));
                    }
                }
            });
        }

        slider = new JSlider(JSlider.VERTICAL);
        updateSliderRange();
        {
            controlPanel = new JPanel();
            scaleDisplayCheckbox = new JCheckBox("autoscale");
            scaleDisplayCheckbox.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    setScaleDisplay(scaleDisplayCheckbox.isSelected());
                }
            });
            thresholdingEnabledCheckbox = new JCheckBox("thresholding");
            thresholdingEnabledCheckbox.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    setThresholdingEnabled(thresholdingEnabledCheckbox.isSelected());
                }
            });
            thresholdingSlider = new JSlider(0, 255);
            thresholdingSlider.addChangeListener(new ChangeListener() {
                public void stateChanged(ChangeEvent e) {
                    setThreshold(thresholdingSlider.getValue());
                }
            });
            thresholdLabel = new JLabel("     ");
            controlPanel.add(scaleDisplayCheckbox);
            controlPanel.add(thresholdingEnabledCheckbox);
            controlPanel.add(thresholdingSlider);
            controlPanel.add(thresholdLabel);
        }

        f.add(slider, BorderLayout.BEFORE_LINE_BEGINS);
        f.add(controlPanel, BorderLayout.AFTER_LAST_LINE);
        if (inBiMode) {
            JPanel bi = new JPanel();
            bi.setLayout(new BoxLayout(bi, boxLayoutOrientationOrNull));
            bi.add(imageContainer);
            bi.add(imageContainer2);
            f.add(bi);
        } else {
            f.add(new JScrollPane(imageContainer));
        }
        slider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent arg0) {
                switchToImage(slider.getValue());
            }
        });
        f.setPreferredSize(new Dimension(800, 600));
        if (exitOnClose) {
            f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        }
        f.pack();

        // set infos
        {
            setScaleDisplay(!initialScaleDisplay);
            setScaleDisplay(initialScaleDisplay);
            setThreshold(5);
            boolean thrEnabled = this.thresholdingEnabled;
            setThresholdingEnabled(!thrEnabled);
            setThresholdingEnabled(thrEnabled);
        }
        f.setVisible(true);
    }

    public void addImage(Image image) {
        images.add(image);
        updateSliderRange();
    }

    public void clearImages() {
        images.clear();
    }

    public void replaceImage(int i, Image image) {
        images.set(i, image);
        switchToImage(slider.getValue());
    }

    public void switchToImage(int value) {
        imageContainer.setImage(images.get(value));
        if (inBiMode) {
            if (value < slider.getMaximum() - 1) {
                imageContainer2.setImage(images.get(value + 1));
            } else {
                imageContainer2.setImage(new BufferedImage(1, 1, BufferedImage.TYPE_3BYTE_BGR));
            }
        }
        slider.setValue(value);
        f.setTitle(baseTitle + "(" + images.get(value).getWidth(null) + "x" + images.get(value).getHeight(null) + ")");
    }

    private void updateSliderRange() {
        if (images.isEmpty()) {
            return;
        }
        int value = slider.getValue();
        int max = images.size() - 1;
        value = Math.min(value, max);
        slider.getModel().setRangeProperties(value, 1, 0, max+1, false);
        slider.setValue(value);
    }

    public boolean isScaleDisplay() {
        return scaleDisplay;
    }

    public void setScaleDisplay(boolean scaleDisplay) {
        if (this.scaleDisplay != scaleDisplay) {
            this.scaleDisplay = scaleDisplay;
            this.scaleDisplayCheckbox.setSelected(scaleDisplay);
            this.imageContainer.repaint();
            this.imageContainer2.repaint();
        }
    }
    
    public boolean isThresholdingEnabled() {
        return thresholdingEnabled;
    }

    public void setThresholdingEnabled(boolean thresholdingEnabled) {
        if (this.thresholdingEnabled != thresholdingEnabled) {
            this.thresholdingEnabled = thresholdingEnabled;
            this.thresholdingEnabledCheckbox.setSelected(thresholdingEnabled);
            this.thresholdingSlider.setEnabled(thresholdingEnabled);
            this.imageContainer.repaint();
            this.imageContainer2.repaint();
        }
    }

    public int getThreshold() {
        return threshold;
    }

    public void setThreshold(int threshold) {
        if (this.threshold != threshold) {
            this.threshold = threshold;
            this.thresholdingSlider.setValue(threshold);
            this.thresholdLabel.setText("" + threshold);
            if (thresholdingEnabled) {
                this.imageContainer.repaint();
                this.imageContainer2.repaint();
            }
        }
    }

    JFileChooser chooser = null;
    private File selectImage(String forWhat) {
        if (chooser == null) {
            chooser = new JFileChooser();
            //Main.setupChooserDirectory(chooser); // this dependency to Main is bad but anyway ... (quickly fixable)
            chooser.addChoosableFileFilter(new FileFilter() {
                @Override
                public boolean accept(File f) {
                    return f.isDirectory() || Arrays.asList(ImageIO.getWriterFileSuffixes()).contains(FileUtils.getExtension(f));
                }

                @Override
                public String getDescription() {
                    return "Supported Images " + Arrays.toString(ImageIO.getWriterFileSuffixes());
                }
            });
            chooser.setAcceptAllFileFilterUsed(false);
            chooser.setMultiSelectionEnabled(false);
        }
        if (JFileChooser.APPROVE_OPTION == chooser.showDialog(null, forWhat)) {
            return chooser.getSelectedFile();
        } else {
            return null;
        }
    }

    private void exportCurrentImage() {
        try {
            File out = selectImage("Save");
            BufferedImage im = new BufferedImage(imageContainer.image.getWidth(null), imageContainer.image.getHeight(null), BufferedImage.TYPE_INT_BGR);
            im.createGraphics().drawImage(imageContainer.image, 0, 0, null);
            ImageIO.write(im, "png", out);
        } catch (IOException ex) {
            Logger.getLogger(ImageStackViewer.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    private /*non-static*/ class ImageDisplay extends JPanel {

        Image image;

        public ImageDisplay() {
            setCursor(Cursor.getPredefinedCursor(Cursor.CROSSHAIR_CURSOR));
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Image toRender = image;
            if (toRender == null) return;
            if (thresholdingEnabled && threshold != 0) {
                BufferedImage in = (BufferedImage) toRender;
                int w = in.getWidth();
                int h = in.getHeight();

                int thresheldColor = 0x408F40;
                IndexColorModel currentModel = (IndexColorModel) in.getColorModel();
                int nBits = currentModel.getPixelSize();
                int[] cmap;
                int transfertType;
                if (nBits == 8) {
                    transfertType = DataBuffer.TYPE_BYTE;
                    cmap = new int[256];
                    for (int i = 0; i < cmap.length; i++) {
                        cmap[i] = i < threshold ? thresheldColor : rgbgray(i);
                    }
                } else {
                    transfertType = DataBuffer.TYPE_USHORT;
                    cmap = new int[256 * 256];
                    for (int i = 0; i < cmap.length; i++) {
                        cmap[i] = i < threshold*255 ? thresheldColor : rgbgray(i>>8);
                    }
                }
                IndexColorModel colorModel = new IndexColorModel(nBits, cmap.length, cmap, 0, false, 0, transfertType);
                toRender = new BufferedImage(colorModel, in.getRaster(), in.isAlphaPremultiplied(), new Hashtable());
            }
            if (scaleDisplay) {
                int w = this.getWidth();
                int h = this.getHeight();
                int imW = toRender.getWidth(null);
                int imH = toRender.getHeight(null);
                if (w / (float) imW > h / (float) imH) {
                    w = imW * h / imH;
                } else {
                    h = imH * w / imW;
                }
                g.drawImage(toRender, 0, 0, w, h, null);
            } else {
                g.drawImage(toRender, 0, 0, null);
            }
        }
        Point2D getImageCoordinates(MouseEvent e) {
            if (scaleDisplay && image != null) {
                int w = this.getWidth();
                int h = this.getHeight();
                int imW = image.getWidth(null);
                int imH = image.getHeight(null);
                if (w / (float) imW > h / (float) imH) {
                    w = imW * h / imH;
                } else {
                    h = imH * w / imW;
                }
                return new Point2D.Double(((double) e.getX()) / w * imW, ((double) e.getY()) / h * imH);
            } else {
                return new Point2D.Double(e.getX(), e.getY());
            }
        }

        private void setImage(Image image) {
            this.image = image;
            this.repaint();
        }

        private int rgbgray(int i) {
            int res = i;
            res <<= 8;
            res += i;
            res <<= 8;
            res += i;
            return res;
        }

    }

}
