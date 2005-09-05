package pclient;

import javax.swing.JInternalFrame;

import java.awt.event.*;
import java.awt.*;


public class OutputWindow extends JInternalFrame {
    public OutputWindow(int width, int height) {
        super("window",
              true, //resizable
              true, //closable
              true, //maximizable
              true);//iconifiable

        setSize(width, height);
        setVisible(true);
    }
}
