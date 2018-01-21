#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define OPENCV

#include <QMainWindow>
#include <qfuture.h>
#include <qfuturewatcher.h>
#include <QtConcurrent\qtconcurrentrun.h>
#include <qevent.h>
#include <QFileDialog>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <iostream>
#include <Windows.h>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <cmath>
#include <yolo_v2_class.hpp>	// imported functions from DLL

#include "vehicle.h"

/* NOMINMAX */ // windows.h conflicts
#ifndef NOMINMAX   #ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	// attributes
    Ui::MainWindow *ui;
	int viewerWidth;
	int viewerHeight;
	std::vector<Vehicle*> vehicles; // for vehicles inside screen
	std::vector<Vehicle*> vehiclesOut; // for vehicles already out of screen
	std::vector<box> lastBoxes; // objects in the last frame, used to calculate IoU
	int frameCounter;

	// methods
    void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names,
        unsigned int wait_msec, int current_det_fps, int current_cap_fps);
    void show_console_result(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names);
    std::vector<std::string> objects_names_from_file(std::string const filename);
	void trackVideo(std::string filePath);
	void diffVehicles(cv::Mat curFrame, std::vector<bbox_t> boxes);
	void updateFrame(cv::Mat mat_img);
	void writeOutVehicle(int index); // write all screenshots to file system right after a vehicle run outside screen

signals:
	void frameChanged(cv::Mat mat_img);

private slots:
	void on_actionOpen_triggered();
	// void updateFrame(cv::Mat mat_img);
};

#endif // MAINWINDOW_H
