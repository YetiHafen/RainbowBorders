package net.yetihafen.rainbowborders;

import com.sun.jna.Pointer;
import com.sun.jna.platform.win32.*;
import javafx.concurrent.ScheduledService;
import javafx.concurrent.Task;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.util.Duration;

import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.ResourceBundle;

public class UiController implements Initializable {

    private ScheduledService<Void> refresh;
    private RainbowBorders app;

    @FXML
    private void onStartPress() {
        app.getTimer().start();
    }

    @FXML
    private void onStopPress() {
        app.getTimer().stop();
    }

    @FXML
    public void onReloadPress() {
        app.getWindows().clear();
        User32.INSTANCE.EnumWindows((hWnd, data) -> {
            app.getWindows().add(hWnd);
            return true;
        }, new Pointer(0));
    }

    @Override
    public void initialize(URL url, ResourceBundle resourceBundle) {

        refresh = new ScheduledService<>() {
            @Override
            protected Task<Void> createTask() {
                return new Task<>() {
                    @Override
                    protected Void call() throws Exception {
                        app.setWindows(getWindows());
                        return null;
                    }
                };
            }
        };
        refresh.setPeriod(Duration.seconds(1));
        refresh.start();
    }

    public void setApplication(RainbowBorders app) {
        this.app = app;
    }


    private List<WinDef.HWND> getWindows() {
        List<WinDef.HWND> windows = new ArrayList<>();
        User32.INSTANCE.EnumWindows((hWnd, data) -> {
            windows.add(hWnd);
            return true;
        }, new Pointer(0));
        return windows;
    }

    @FXML
    private void onHidePress() {
        app.getStage().hide();
    }
}
