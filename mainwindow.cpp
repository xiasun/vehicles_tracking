#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	// initialize video viewer size
	viewerWidth = ui->videoViewer->width();
	viewerHeight = ui->videoViewer->height();
	frameCounter = 0;

	emptyImg = Mat(viewerHeight, viewerWidth, CV_8UC3, Scalar(100, 100, 100)); // empty screen
	/*ROI.push_back(make_pair(0, 0));
	ROI.push_back(make_pair(viewerWidth, 0));
	ROI.push_back(make_pair(viewerWidth, viewerHeight));
	ROI.push_back(make_pair(0, viewerHeight));
	for (auto vertex : ROI) {
		circle(emptyImg, Point(vertex.first, vertex.second), 20, Scalar(0, 0, 255));
	}*/
	
	updateFrame(emptyImg);

	bool result = connect(ui->videoViewer, SIGNAL(clicked(QMouseEvent*)),
		this, SLOT(selectROI(QMouseEvent*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectROI(QMouseEvent* event) {
	int xPos = event->pos().x(), yPos = event->pos().y();
	if (ROI.size() < 4) {
		ROI.push_back(make_pair(xPos, yPos));
	}
}

void MainWindow::draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names,
    unsigned int wait_msec = 0, int current_det_fps = -1, int current_cap_fps = -1)
{
	//imwrite("D:\\Data\\test\\nobox.jpg", mat_img);
    int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };

    for (auto &i : result_vec) {
        int const offset = i.obj_id * 123457 % 6;
        int const color_scale = 150 + (i.obj_id * 123457) % 100;
        cv::Scalar color(colors[offset][0], colors[offset][1], colors[offset][2]);
        color *= color_scale;
        cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 5);
        if (obj_names.size() > i.obj_id) {
            std::string obj_name = obj_names[i.obj_id];
            if (i.track_id > 0) obj_name += " - " + std::to_string(i.track_id);
            cv::Size const text_size = getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, 0);
            int const max_width = (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
            cv::rectangle(mat_img, cv::Point2f(max((int)i.x - 3, 0), max((int)i.y - 30, 0)),
                cv::Point2f(min((int)i.x + max_width, mat_img.cols-1), min((int)i.y, mat_img.rows-1)),
                color, CV_FILLED, 8, 0);
            putText(mat_img, obj_name, cv::Point2f(i.x, i.y - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(0, 0, 0), 2);
        }
    }
    if (current_det_fps >= 0 && current_cap_fps >= 0) {
        std::string fps_str = "FPS detection: " + std::to_string(current_det_fps) + "   FPS capture: " + std::to_string(current_cap_fps);
        putText(mat_img, fps_str, cv::Point2f(10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(50, 255, 0), 2);
    }
	updateFrame(mat_img);
	// imwrite("D:\\Data\\test\\withbox2.jpg", mat_img);
	/*cv::imshow("hehe", mat_img);
    cv::waitKey(wait_msec);*/
}

void MainWindow::show_console_result(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names) {
    for (auto &i : result_vec) {
		
		if (obj_names.size() > i.obj_id) {
			ui->logOutput->append((obj_names[i.obj_id] + ", prob = " + to_string(i.prob)).c_str());
		}
		ui->logOutput->append(("obj_id = " + to_string(i.obj_id) + 
			",  x = " + to_string(i.x) + ", y = " + to_string(i.y) + 
			", w = " + to_string(i.w) + ", h = " + to_string(i.h)).c_str());

        /*if (obj_names.size() > i.obj_id) std::cout << obj_names[i.obj_id] << " - ";
        std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x << ", y = " << i.y
            << ", w = " << i.w << ", h = " << i.h
            << std::setprecision(3) << ", prob = " << i.prob << std::endl;*/
    }
}

std::vector<std::string> MainWindow::objects_names_from_file(std::string const filename) {
    std::ifstream file(filename);
    std::vector<std::string> file_lines;
    if (!file.is_open()) return file_lines;
    for(std::string line; getline(file, line);) file_lines.push_back(line);
    std::cout << "object names loaded \n";
    return file_lines;
}

void MainWindow::on_actionOpen_triggered() {
	auto filePath = QFileDialog::getOpenFileName(this, tr("Open video file"), "", tr("All Files (*)")).toUtf8().constData();
	QFuture<void> future = QtConcurrent::run(this, &MainWindow::trackVideo, filePath);
}

void MainWindow::trackVideo(string filePath) {
	//std::string filename = "D:\\Data\\session1_center.avi";
	string filename = filePath;

	Detector detector("D:\\Data\\yolo-all.cfg",
		"D:\\Data\\yolo-all_45000.weights");

	auto obj_names = objects_names_from_file("D:\\Data\\all_obj.names");
	std::string out_videofile = "result.avi";
	bool const save_output_videofile = false;

	while (true)
	{
		std::cout << "input image or video filename: ";
		if (filename.size() == 0) std::cin >> filename;
		if (filename.size() == 0) break;

		try {
#ifdef OPENCV
			std::string const file_ext = filename.substr(filename.find_last_of(".") + 1);
			std::string const protocol = filename.substr(0, 7);
			if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mjpg" || file_ext == "mov" || 	// video file
				protocol == "rtmp://" || protocol == "rtsp://" || protocol == "http://" || protocol == "https:/")	// video network stream
			{
				// std::vector<Vehicle*> vehicles; // for all vehicles appeared in the video
				// std::vector<box> lastBoxes; // objects in the last frame, used to calculate IoU

				cv::Mat cap_frame, cur_frame, det_frame, write_frame;
				std::shared_ptr<image_t> det_image;
				std::vector<bbox_t> result_vec, thread_result_vec;
				detector.nms = 0.02;	// comment it - if track_id is not required
				std::atomic<bool> consumed, videowrite_ready;
				consumed = true;
				videowrite_ready = true;
				std::atomic<int> fps_det_counter, fps_cap_counter;
				fps_det_counter = 0;
				fps_cap_counter = 0;
				int current_det_fps = 0, current_cap_fps = 0;
				std::thread t_detect, t_cap, t_videowrite;
				std::mutex mtx;
				std::condition_variable cv;
				std::chrono::steady_clock::time_point steady_start, steady_end;
				cv::VideoCapture cap(filename); cap >> cur_frame;
				int const video_fps = cap.get(CV_CAP_PROP_FPS);
				cv::Size const frame_size = cur_frame.size();
				cv::VideoWriter output_video;
				if (save_output_videofile)
					output_video.open(out_videofile, CV_FOURCC('D', 'I', 'V', 'X'), max(35, video_fps), frame_size, true);

				int frameCounter = 0;
				while (!cur_frame.empty()) {
					if (t_cap.joinable()) {
						t_cap.join();
						++fps_cap_counter;
						cur_frame = cap_frame.clone();
					}
					t_cap = std::thread([&]() { cap >> cap_frame; });

					while (ROI.size() < 4) {
						updateFrame(cur_frame); // pause until ROI is selected
					}
					std::vector<bbox_t> result_vec = detector.detect(cur_frame);
					std::vector<bbox_t> filtered_vec;
					// filter based on selected ROI
					for (auto vec : result_vec) {
						Mat vecMat;
					}

					if (!cur_frame.empty()) {
						steady_end = std::chrono::steady_clock::now();
						if (std::chrono::duration<double>(steady_end - steady_start).count() >= 1) {
							current_det_fps = fps_det_counter;
							current_cap_fps = fps_cap_counter;
							steady_start = steady_end;
							fps_det_counter = 0;
							fps_cap_counter = 0;
						}

						// --- tracking part below
						diffVehicles(cur_frame, result_vec);
						// --- tracking part above

						// imwrite("D:\\Data\\test\\3.jpg", cur_frame);
						// draw_boxes(cur_frame, result_vec, obj_names, 3, -1, -1);
						draw_boxes(cur_frame, result_vec, obj_names, 3, current_det_fps, current_cap_fps);
						show_console_result(result_vec, obj_names);

						if (output_video.isOpened() && videowrite_ready) {
							if (t_videowrite.joinable()) t_videowrite.join();
							write_frame = cur_frame.clone();
							videowrite_ready = false;
							t_videowrite = std::thread([&]() {
								output_video << write_frame; videowrite_ready = true;
							});
						}
					}

					// wait detection result for video-file only (not for net-cam)
					/*if (protocol != "rtsp://" && protocol != "http://" && protocol != "https:/") {
						std::unique_lock<std::mutex> lock(mtx);
						while (!consumed) cv.wait(lock);
					}*/
				}
				if (t_cap.joinable()) t_cap.join();
				if (t_detect.joinable()) t_detect.join();
				if (t_videowrite.joinable()) t_videowrite.join();
				std::cout << "Video ended \n";
			}
			else if (file_ext == "txt") {	// list of image files
				std::ifstream file(filename);
				if (!file.is_open()) std::cout << "File not found! \n";
				else
					for (std::string line; file >> line;) {
						std::cout << line << std::endl;
						cv::Mat mat_img = cv::imread(line);
						std::vector<bbox_t> result_vec = detector.detect(mat_img);
						show_console_result(result_vec, obj_names);
						//draw_boxes(mat_img, result_vec, obj_names);
						//cv::imwrite("res_" + line, mat_img);
					}

			}
			else {	// image file
				cv::Mat mat_img = cv::imread(filename);
				std::vector<bbox_t> result_vec = detector.detect(mat_img);
				result_vec = detector.tracking(result_vec);	// comment it - if track_id is not required
				draw_boxes(mat_img, result_vec, obj_names, 0, -1, -1);
				show_console_result(result_vec, obj_names);
			}
#else
			//std::vector<bbox_t> result_vec = detector.detect(filename);

			auto img = detector.load_image(filename);
			std::vector<bbox_t> result_vec = detector.detect(img);
			detector.free_image(img);
			show_console_result(result_vec, obj_names);
#endif
			}
		catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
		catch (...) { std::cerr << "unknown exception \n"; getchar(); }
		filename.clear();
	}
}

void MainWindow::diffVehicles(cv::Mat curFrame, std::vector<bbox_t> boxes) {
	std::vector<box> currentBoxes;
	for (int i = 0; boxes.size() > 0 && i < boxes.size(); i++) {
		box currentVehicle;
		currentVehicle.x = boxes[i].x,
			currentVehicle.y = boxes[i].y,
			currentVehicle.w = boxes[i].w,
			currentVehicle.h = boxes[i].h;
		float iouMax = 0;
		int maxIdx = -1;
		for (int j = 0; j < lastBoxes.size(); j++) {
			float iou = box_iou(currentVehicle, lastBoxes[j]);
			if (iou > iouMax) {
				iouMax = iou;
				maxIdx = j;
			}
		}
		if (iouMax == 0) { // no previous found, new vehicle entered
			vehicles.push_back(new Vehicle(currentVehicle));
		}
		else {
			for (int j = 0; j < vehicles.size(); j++) {
				float fuck = box_iou(currentVehicle, vehicles[j]->boxes[vehicles[j]->boxes.size() - 1]);
				if (fuck > 0) {
					vehicles[j]->out = false;
					vehicles[j]->boxes.push_back(currentVehicle);
					string oImgPath = "D:\\Data\\output\\" + std::to_string(j) + "\\" + std::to_string(frameCounter) + ".jpg";
					Rect boxRect = cv::Rect(currentVehicle.x, currentVehicle.y,
						min(currentVehicle.w, curFrame.cols - currentVehicle.x),
						min(currentVehicle.h, curFrame.rows - currentVehicle.y)); // avoid out of boundary
					Mat oImg = cv::Mat(curFrame, boxRect);
					vehicles[j]->screenShots.push_back(oImg);
					// cv::imwrite(oImgPath, oImg);
				}
			}
		}
		currentBoxes.push_back(currentVehicle);
	}
	for (int i = 0; i < vehicles.size(); i++) {
		if (vehicles[i]->out) {
			vehiclesOut.push_back(vehicles[i]);
			QFuture<void> future = QtConcurrent::run(this, &MainWindow::writeOutVehicle, vehiclesOut.size() - 1);
			vehicles.erase(vehicles.begin() + i);
		}
		else {
			vehicles[i]->out = true;
		}
	}
	lastBoxes = currentBoxes;
	frameCounter++;
}

void MainWindow::updateFrame(Mat mat_img) {
	int imgWidth = mat_img.cols, imgHeight = mat_img.rows;
	cv::resize(mat_img, mat_img, Size(viewerWidth, viewerHeight), 1.0, 1.0, INTER_LANCZOS4);
	cvtColor(mat_img, mat_img, CV_BGR2RGB);
	auto ROIdraw = ROI;
	if (ROI.size() == 4) {
		ROIdraw.push_back(ROIdraw[0]); // draw rectangle when 4 points selected
		vector<Point> ROIPoints = { Point(ROI[0].first, ROI[0].second),
			Point(ROI[1].first, ROI[1].second),
			Point(ROI[2].first, ROI[2].second),
			Point(ROI[3].first, ROI[3].second)
		};
		
		imshow("fuck", ROIMask); waitKey();
	}
	for (int i = 0; i < (int)ROIdraw.size() - 1; i++) {
		line(mat_img, Point(ROIdraw[i].first, ROIdraw[i].second), Point(ROIdraw[i + 1].first, ROIdraw[i + 1].second), Scalar(255, 0, 0), 3);
	}
	ui->videoViewer->setPixmap(QPixmap::fromImage(QImage(mat_img.data, mat_img.cols, mat_img.rows, mat_img.step, QImage::Format_RGB888)));
}

void MainWindow::writeOutVehicle(int index) {
	string outputPath = "D:\\Data\\output\\" + to_string(index);
	wstring outputPathL(outputPath.begin(), outputPath.end());
	CreateDirectory(outputPathL.c_str(), NULL);
	for (int i = 0; i < vehiclesOut[index]->screenShots.size(); i++) { 
		imwrite("D:\\Data\\output\\" + std::to_string(index) + "\\" + std::to_string(i) + ".jpg", vehiclesOut[index]->screenShots[i]);
	}
	vehiclesOut[index]->screenShots.clear();
	// vehiclesOut.erase(vehiclesOut.begin() + index);
	// delete &vehiclesOut[index];
	// vehiclesOut[index] = new Vehicle();
}
