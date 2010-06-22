/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.heeere.opencl.demo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

/**
 *
 */
public class FileUtils {

    public static File[] listAlphabetically(String dir) {
        List<File> files = Arrays.asList(new File(dir).listFiles());
        Collections.sort(files);
        return files.toArray(new File[0]);
    }

    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 &&  i < s.length() - 1) {
            ext = s.substring(i+1).toLowerCase();
        }
        return ext;
    }

    public static Iterable<String> fileLines(String filename) throws FileNotFoundException {
        return fileLines(new File(filename));
    }
    public static Iterable<String> fileLines(final File f) throws FileNotFoundException {
        if (!f.exists()) throw new FileNotFoundException(f.getAbsolutePath());
        final BufferedReader in = new BufferedReader(new FileReader(f));
        return readerLines(in);
    }
    public static Iterable<String> stringLines(String in) {
        return readerLines(new BufferedReader(new StringReader(in)));
    }
    public static Iterable<String> readerLines(final BufferedReader in) {
        return new Iterable<String>() {
            public Iterator<String> iterator() {
                return new Iterator<String>() {
                    String line;
                    {
                        fetchLine();
                    }

                    public boolean hasNext() {
                        return line != null;
                    }

                    public String next() {
                        String res = line;
                        fetchLine();
                        return res;
                    }

                    public void remove() {
                        throw new UnsupportedOperationException("Not supported.");
                    }

                    private void fetchLine() {
                        try {
                            line = in.readLine();
                        } catch (IOException ex) {
                            throw new RuntimeException("Exception while iterating over lines in file", ex);
                        }
                    }
                };
            }
        };
    }

}
