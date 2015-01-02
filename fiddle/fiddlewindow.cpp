#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDoubleSpinBox>

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
    edgeWidget = new CVImageWidget();
    structureWidget = new CVImageWidget();
    colorWidget = new CVImageWidget();
    restoredWidget = new CVImageWidget();

    sigmaSpinBox = new QDoubleSpinBox();
    sigmaSpinBox->setDecimals(2);
    sigmaSpinBox->setRange(0.01, 1000);
    rhoSpinBox = new QDoubleSpinBox();
    rhoSpinBox->setDecimals(2);
    rhoSpinBox->setRange(0.01, 1000);
    gammaSpinBox = new QDoubleSpinBox();
    gammaSpinBox->setDecimals(2);
    gammaSpinBox->setRange(0.01, 2000000000);
    betaSpinBox = new QDoubleSpinBox();
    betaSpinBox->setDecimals(0);
    betaSpinBox->setRange(1, 2000000000);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(origWidget, 0, 2);
    gridLayout->addWidget(blurWidget, 2, 2);
    gridLayout->addWidget(edgeWidget, 0, 4);
    gridLayout->addWidget(structureWidget, 2, 4);
    gridLayout->addWidget(colorWidget, 0, 6);
    gridLayout->addWidget(restoredWidget, 2, 6);

    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 0);
    gridLayout->setColumnStretch(2, 4);
    gridLayout->setColumnStretch(3, 0);
    gridLayout->setColumnStretch(4, 4);
    gridLayout->setColumnStretch(5, 0);
    gridLayout->setColumnStretch(6, 4);

    gridLayout->setColumnMinimumWidth(1, 10);
    gridLayout->setColumnMinimumWidth(3, 10);
    gridLayout->setColumnMinimumWidth(5, 10);

    gridLayout->setRowStretch(0, 4);
    gridLayout->setRowStretch(1, 0);
    gridLayout->setRowStretch(2, 4);

    gridLayout->setRowMinimumHeight(1, 10);

    QGridLayout *paramLayout = new QGridLayout();

    QLabel *rhoLabel = new QLabel();
    rhoLabel->setText("rho");
    QLabel *sigmaLabel = new QLabel();
    sigmaLabel->setText("sigma");
    QLabel *gammaLabel = new QLabel();
    gammaLabel->setText("gamma");
    QLabel *betaLabel = new QLabel();
    betaLabel->setText("beta");

    paramLayout->addWidget(sigmaLabel, 0, 0);
    paramLayout->addWidget(sigmaSpinBox, 0, 1);
    paramLayout->addWidget(rhoLabel, 1, 0);
    paramLayout->addWidget(rhoSpinBox, 1, 1);
    paramLayout->addWidget(gammaLabel, 2, 0);
    paramLayout->addWidget(gammaSpinBox, 2, 1);
    paramLayout->addWidget(betaLabel, 3, 0);
    paramLayout->addWidget(betaSpinBox, 3, 1);

    gridLayout->addLayout(paramLayout, 0, 0, 3, 1);

    QWidget *window = new QWidget;
    window->setLayout(gridLayout);

    createActions();
    createMenus();

    QObject::connect(this, SIGNAL(nameChanged()),
                     this, SLOT(updateOrig()));

    QObject::connect(this, SIGNAL(origChanged()),
                     this, SLOT(updateTensor()));

    QObject::connect(sigmaSpinBox, SIGNAL(valueChanged(double)),
		    this, SLOT(updateTensor()));
    QObject::connect(rhoSpinBox, SIGNAL(valueChanged(double)),
		    this, SLOT(updateTensor()));
    QObject::connect(gammaSpinBox, SIGNAL(valueChanged(double)),
		    this, SLOT(updateTensor()));
    QObject::connect(betaSpinBox, SIGNAL(valueChanged(double)),
		    this, SLOT(updateTensor()));

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

void FiddleWindow::updateTensor()
{
    cv::Mat image = cv::imread(fileName.toUtf8().constData(), CV_LOAD_IMAGE_GRAYSCALE);

    cv::Mat_<Tensor> tensors = cv::Mat_<Tensor>::zeros(image.rows, image.cols);
    cv::Mat blur, edge, structure, color;
    createAnisotropyTensor(tensors, image, sigmaSpinBox->value(),
		    rhoSpinBox->value(), gammaSpinBox->value(), blur, edge,
		    structure, color);

    cv::normalize(edge, edge, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::normalize(structure, structure, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::normalize(color, color, 0, 255, cv::NORM_MINMAX, CV_8U);

    blurWidget->showImage(blur);
    edgeWidget->showImage(edge);
    structureWidget->showImage(structure);
    colorWidget->showImage(color);
}

FiddleWindow::~FiddleWindow()
{
    delete ui;
}

