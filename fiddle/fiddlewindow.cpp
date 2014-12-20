#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

#include "fiddlewindow.h"
#include "ui_fiddlewindow.h"

#include "cvimagewidget.h"

FiddleWindow::FiddleWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FiddleWindow)
{
    ui->setupUi(this);

    origWidget = new CVImageWidget();
    blurWidget = new CVImageWidget();

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(1);

    QVBoxLayout *origLayout = new QVBoxLayout;
    origLayout->addWidget(origWidget);

    QVBoxLayout *blurLayout = new QVBoxLayout;
    blurLayout->addWidget(blurWidget);
    blurLayout->addWidget(slider);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(origLayout);
    mainLayout->addLayout(blurLayout);
    mainLayout->setStretchFactor(origLayout, 4);
    mainLayout->setStretchFactor(blurLayout, 4);

    QWidget *window = new QWidget;
    window->setLayout(mainLayout);

    createActions();
    createMenus();

    QObject::connect(this, SIGNAL(nameChanged()),
                     this, SLOT(updateOrig()));

    QObject::connect(this, SIGNAL(origChanged()),
                     this, SLOT(updateBlur()));

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
    cv::Mat image = cv::imread(fileName.toUtf8().constData(), true);
    cv::GaussianBlur(image, image, cv::Size(0,0), 2, 0, cv::BORDER_REFLECT);
    blurWidget->showImage(image);
}

FiddleWindow::~FiddleWindow()
{
    delete ui;
}

