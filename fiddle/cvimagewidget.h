#pragma once
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QEvent>
#include <QPaintEvent>
#include <opencv2/opencv.hpp>
#include <iostream>

class CVImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CVImageWidget(QWidget *parent = 0) : QWidget(parent) {}

    QSize sizeHint() const { return _qimage.size(); }
    QSize minimumSizeHint() const { return _qimage.size(); }

public slots:
    
    void showImage(const cv::Mat& image) {
        // Convert the image to the RGB888 format
        switch (image.type()) {
        case CV_8UC1:
            cvtColor(image, _tmp, CV_GRAY2RGB);
            break;
        case CV_8UC3:
            cvtColor(image, _tmp, CV_BGR2RGB);
            break;
        default:
            std::cerr << "COLOR SPACE ERROR";
            break;
        }

        // QImage needs the data to be stored continuously in memory
        assert(_tmp.isContinuous());
        // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
        // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
        // has three bytes.
        _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);

        //this->setFixedSize(image.cols, image.rows);

        repaint();
    }

protected:
    void paintEvent(QPaintEvent *event) {

        QPainter painter(this);
        //painter.drawImage(QPoint(0,0), _qimage);
        //painter.end();

        QWidget::paintEvent(event);

        if (_qimage.isNull())
            return;

        painter.setRenderHint(QPainter::Antialiasing);

        QSize pixSize = _qimage.size();
        pixSize.scale(event->rect().size(), Qt::KeepAspectRatio);

        QImage scaledPix = _qimage.scaled(pixSize,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation
                                       );

        painter.drawImage(QPoint(), scaledPix);

    }
    
    QImage _qimage;
    cv::Mat _tmp;
};

