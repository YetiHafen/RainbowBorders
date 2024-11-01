module net.yetihafen.rainbowborders {
    requires javafx.controls;
    requires javafx.fxml;
    requires com.sun.jna.platform;
    requires com.sun.jna;
    requires java.desktop;


    opens net.yetihafen.rainbowborders to javafx.fxml, com.sun.jna;
    exports net.yetihafen.rainbowborders;
}