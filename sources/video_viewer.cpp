#include "video_viewer.h"

VideoViewer::VideoViewer(QWidget* parent, Qt::WindowFlags f)
	: QLabel(parent) {

}

VideoViewer::~VideoViewer() {}

void VideoViewer::mousePressEvent(QMouseEvent* event) {
	emit clicked(event);
}