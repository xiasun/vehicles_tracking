#ifndef VIDEOVIEWER_H
#define VIDEOVIEWER_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class VideoViewer : public QLabel {
	Q_OBJECT

public:
	explicit VideoViewer(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~VideoViewer();

signals:
	void clicked(QMouseEvent*);

protected:
	void mousePressEvent(QMouseEvent* event);

};

#endif // VideoViewer_H