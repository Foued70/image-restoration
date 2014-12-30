#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

#include "fiddlewindow.h"
#include "ui_fiddlewindow.h"

#include "cvimagewidget.h"

#include "../anisotropy.hpp"

FiddleWindow::FiddleWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FiddleWindow)
{
    ui->setupUi(this);

    origWidget = new CVImageWidget();
    blurWidget = new CVImageWidget();
    colorWidget = new CVImageWidget();

    QSlider *blurSlider = new QSlider(Qt::Horizontal);
    blurSlider->setFocusPolicy(Qt::StrongFocus);
    blurSlider->setTickPosition(QSlider::TicksBothSides);
    blurSlider->setTickInterval(10);
    blurSlider->setSingleStep(1);

    QSlider *gammaSlider = new QSlider(Qt::Horizontal);
    gammaSlider->setFocusPolicy(Qt::StrongFocus);
    gammaSlider->setTickPosition(QSlider::TicksBothSides);
    gammaSlider->setTickInterval(10);
    gammaSlider->setSingleStep(1);

    QVBoxLayout *origLayout = new QVBoxLayout;
    origLayout->addWidget(origWidget);

    QVBoxLayout *blurLayout = new QVBoxLayout;
    blurLayout->addWidget(blurWidget);
    blurLayout->addWidget(blurSlider);

    QVBoxLayout *colorLayout = new QVBoxLayout;
    colorLayout->addWidget(colorWidget);
    colorLayout->addWidget(gammaSlider);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(origLayout);
    mainLayout->addLayout(blurLayout);
    mainLayout->addLayout(colorLayout);
    mainLayout->setStretchFactor(origLayout, 4);
    mainLayout->setStretchFactor(blurLayout, 4);
    mainLayout->setStretchFactor(colorLayout, 4);

    QWidget *window = new QWidget;
    window->setLayout(mainLayout);

    createActions();
    createMenus();

    QObject::connect(this, SIGNAL(nameChanged()),
                     this, SLOT(updateOrig()));

    QObject::connect(this, SIGNAL(origChanged()),
                     this, SLOT(updateBlur()));

    QObject::connect(this, SIGNAL(origChanged()),
                     this, SLOT(updateColor()));

    this->setCentralWidget(window);
}

void FiddleWindow::createActions()
{
     openAct = new QAction(tr("&Open"), this);
     openAct->setShortcuts(QKeySequence::Open);
     openAct->setStatusTip(tr("Open a file"));
     connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
}

void FiddleWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);

    menuBar()->setNativeMenuBar(false);
}

void FiddleWindow::openFile()
{
    fileName = QFileDialog::getOpenFileName(this,
         tr("Open Image"), "/home/burk", tr("Image Files (*.png *.pgm)"));

    emit nameChanged();
}

void FiddleWindow::updateOrig()
{
    cv::Mat image = cv::imread(fileName.toUtf8().constData(), true);
    origWidget->showImage(image);
    emit origChanged();
}

void FiddleWindow::updateBlur()
{
    cv::Mat image = cv::imread(fileName.toUtf8().constData(), CV_LOAD_IMAGE_GRAYSCALE);

    cv::Mat_<Tensor> tensors = cv::Mat_<Tensor>::zeros(image.rows, image.cols);
    cv::Mat blur, edge, structure, color;
    createAnisotropyTensor(tensors, image, 5, 10, 10,
		    blur, edge, structure, color);

    blurWidget->showImage(blur);
}

void FiddleWindow::updateColor()
{
    cv::Mat image = cv::imread(fileName.toUtf8().constData(), CV_LOAD_IMAGE_GRAYSCALE);

    cv::Mat_<Tensor> tensors = cv::Mat_<Tensor>::zeros(image.rows, image.cols);
    cv::Mat blur, edge, structure, color;
    createAnisotropyTensor(tensors, image, 5, 10, 100,
		    blur, edge, structure, color);

    cv::normalize(color, color, 0, 255, cv::NORM_MINMAX, CV_8U);
    colorWidget->showImage(color);
}

FiddleWindow::~FiddleWindow()
{
    delete ui;
}

