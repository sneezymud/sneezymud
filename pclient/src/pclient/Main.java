/*
 * Main.java
 *
 * Created on September 5, 2005, 7:50 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package pclient;


/**
 *
 * @author Travis
 */
public class Main {
    
    
    /** Creates a new instance of Main */
    public Main() {
    }
 
    public static void main(String[] args) {
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            OutputHandler output = new OutputHandler();
    
            public void run() {
                output.createAndShowGUI();
            }
        });
    }

}
