package net.yetihafen.rainbowborders;

import com.sun.jna.platform.win32.WinDef;
import com.sun.jna.platform.win32.WinNT;
import com.sun.jna.ptr.IntByReference;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.scene.paint.Color;
import javafx.stage.Stage;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class RainbowBorders extends Application {

    private AnimationTimer timer;
    private List<WinDef.HWND> windows = new ArrayList<>();
    private Stage stage;

    @Override
    public void start(Stage stage) throws IOException, AWTException {
        Platform.setImplicitExit(false);
        FXMLLoader fxmlLoader = new FXMLLoader(RainbowBorders.class.getResource("ui.fxml"));
        Scene scene = new Scene(fxmlLoader.load(), 320, 240);
        UiController c = fxmlLoader.getController();
        c.setApplication(this);
        timerInit();
        c.onReloadPress();
        stage.setTitle("RainbowBorders");
        stage.setScene(scene);
        this.stage = stage;
        stage.getIcons().add(new Image(getClass().getResourceAsStream("icon.png")));
        stage.show();

        stage.setOnCloseRequest(e -> {
            Platform.exit();
            System.exit(0);
        });

        TrayIcon trayIcon = new TrayIcon(ImageIO.read(getClass().getResourceAsStream("icon.png")));
        trayIcon.setImageAutoSize(true);
        trayIcon.setToolTip("RainbowBorders");
        trayIcon.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                Platform.runLater(stage::show);
                super.mouseClicked(e);
            }
        });

        SystemTray.getSystemTray().add(trayIcon);
    }

    private void timerInit() {
        timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                javafx.scene.paint.Color c = Color.hsb((now / 10000000.0) % 360, 1, 1);
                for(WinDef.HWND hWnd : windows) {
                    setBorderColor(hWnd, c);
                }
            }
        };
    }

    private void setBorderColor(WinDef.HWND hWnd, Color color) {
        int red = (int) (color.getRed() * 255);
        int green = (int) (color.getGreen() * 255);
        int blue = (int) (color.getBlue() * 255);
        // win api accepts the colors in reverse order
        int rgb = red + (green << 8) + (blue << 16);
        WinNT.HRESULT res = DwmApi.INSTANCE.DwmSetWindowAttribute(hWnd, DWMWINDOWATTRIBUTE.DWMWA_BORDER_COLOR, new IntByReference(rgb), 4);
    }

    public AnimationTimer getTimer() {
        return timer;
    }

    public List<WinDef.HWND> getWindows() {
        return windows;
    }

    public void setWindows(List<WinDef.HWND> windows) {
        this.windows = windows;
    }

    public Stage getStage() {
        return stage;
    }

    public static void main(String[] args) {
        launch();
    }
}