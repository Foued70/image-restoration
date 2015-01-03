#include <QPushButton>
#include <QToolTip>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QFileDialog>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>

#include "fiddlewindow.h"
#include "ui_fiddlewindow.h"

#include "cvimagewidget.h"

#include "../anisotropy.hpp"
#include "../image.hpp"

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

    origWidget->setToolTip(tr("Original image."));
    blurWidget->setToolTip(tr("Blurred with parameter sigma."));
    edgeWidget->setToolTip(tr("Edge detector before smoothing with rho.\n\n"
			    "Note that this image has been\n"
			    "normalized to the range 0-255."));
    structureWidget->setToolTip(tr("Blurred image with some tensors overlayed"));
    colorWidget->setToolTip(tr("Colorized visualization of the tensors.\n\n"
			    "Note that in this image the colors have been\n"
			    "normalized to the range 0-255."));
    restoredWidget->setToolTip(tr("Restored image."));

    sigmaSpinBox = new QDoubleSpinBox();
    sigmaSpinBox->setDecimals(2);
    sigmaSpinBox->setRange(0.01, 1000);
    sigmaSpinBox->setValue(2);
    rhoSpinBox = new QDoubleSpinBox();
    rhoSpinBox->setDecimals(2);
    rhoSpinBox->setRange(0.01, 1000);
    rhoSpinBox->setValue(4);
    gammaSpinBox = new QDoubleSpinBox();
    gammaSpinBox->setDecimals(2);
    gammaSpinBox->setRange(0.01, 2000000000);
    gammaSpinBox->setValue(100);
    betaSpinBox = new QDoubleSpinBox();
    betaSpinBox->setDecimals(0);
    betaSpinBox->setRange(1, 2000000000);
    betaSpinBox->setValue(1000);

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

    QLabel *sigmaLabel = new QLabel();
    sigmaLabel->setText("Sigma");
    QLabel *rhoLabel = new QLabel();
    rhoLabel->setText("Rho");
    QLabel *gammaLabel = new QLabel();
    gammaLabel->setText("Gamma");
    QLabel *betaLabel = new QLabel();
    betaLabel->setText("Beta");

    QPushButton *restoreButton = new QPushButton(tr("Restore!"));

    QGroupBox *neighBox = new QGroupBox(tr("Neighborhood size"));

    n4 = new QRadioButton(tr("4"));
    n8 = new QRadioButton(tr("8"));
    n16 = new QRadioButton(tr("16"));
    n32 = new QRadioButton(tr("32"));
    n48 = new QRadioButton(tr("48"));
    n72 = new QRadioButton(tr("72"));

    n16->setChecked(true);

    QVBoxLayout *radioBox = new QVBoxLayout();

    radioBox->addWidget(n4);
    radioBox->addWidget(n8);
    radioBox->addWidget(n16);
    radioBox->addWidget(n32);
    radioBox->addWidget(n48);
    radioBox->addWidget(n72);

    neighBox->setLayout(radioBox);

    paramLayout->addWidget(sigmaLabel, 0, 0);
    paramLayout->addWidget(sigmaSpinBox, 0, 1);
    paramLayout->addWidget(rhoLabel, 1, 0);
    paramLayout->addWidget(rhoSpinBox, 1, 1);
    paramLayout->addWidget(gammaLabel, 2, 0);
    paramLayout->addWidget(gammaSpinBox, 2, 1);
    paramLayout->addWidget(betaLabel, 3, 0);
    paramLayout->addWidget(betaSpinBox, 3, 1);
    paramLayout->addWidget(neighBox, 4, 0, 1, 2);
    paramLayout->addWidget(restoreButton, 5, 0);
    paramLayout->setRowStretch(6, 4);

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

    QObject::connect(restoreButton, SIGNAL(clicked()),
		    this, SLOT(restore()));

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

void FiddleWindow::restore()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    cv::Mat image = cv::imread(fileName.toUtf8().constData(), CV_LOAD_IMAGE_GRAYSCALE);

    cv::Mat_<Tensor> tensors = cv::Mat_<Tensor>::zeros(image.rows, image.cols);
    cv::Mat blur, edge, structure, color;
    createAnisotropyTensor(tensors, image, sigmaSpinBox->value(),
		    rhoSpinBox->value(), gammaSpinBox->value(), blur, edge,
		    structure, color);

    Neighborhood neigh;
    int neighbors = 16;

    if (n4->isChecked())
	    neighbors = 4;
    else if (n8->isChecked())
	    neighbors = 8;
    else if (n16->isChecked())
	    neighbors = 16;
    else if (n32->isChecked())
	    neighbors = 32;
    else if (n48->isChecked())
	    neighbors = 48;
    else if (n72->isChecked())
	    neighbors = 72;

    if (neighbors >= 4) {
	    neigh.add( 1, 0, 1.0);
	    neigh.add( 0, 1, 1.0);
	    neigh.add(-1, 0, 1.0);
	    neigh.add( 0,-1, 1.0);
    }

    if (neighbors >= 8) {
	    neigh.add( 1, 1, 1.0);
	    neigh.add(-1, 1, 1.0);
	    neigh.add( 1,-1, 1.0);
	    neigh.add(-1,-1, 1.0);
    }

    if (neighbors >= 16) {
	    neigh.add8(1, 2, 1.0);
    }

    if (neighbors >= 32) {
	    neigh.add8(3, 1, 1.0);
	    neigh.add8(3, 2, 1.0);
    }

    if (neighbors >= 48) {
	    neigh.add8(1, 4, 1.0);
	    neigh.add8(3, 4, 1.0);
    }

    if (neighbors >= 72) {
	    neigh.add8(1, 5, 1.0);
	    neigh.add8(2, 5, 1.0);
	    neigh.add8(3, 5, 1.0);
    }

    neigh.setupAngles();

    cv::Mat out = image.clone();
    restoreAnisotropicTV(image, out, tensors, neigh, 100, betaSpinBox->value(), 2);
    restoredWidget->showImage(out);

    QApplication::restoreOverrideCursor();
}

FiddleWindow::~FiddleWindow()
{
    delete ui;
}

