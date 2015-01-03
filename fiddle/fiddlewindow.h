#ifndef FIDDLEWINDOW_H
#define FIDDLEWINDOW_H

#include <QMainWindow>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include "cvimagewidget.h"

namespace Ui {
class FiddleWindow;
}

class FiddleWindow : public QMainWindow
{
    Q_OBJECT
    QMenu *fileMenu;
    QAction *openAct;
    QString fileName;
    CVImageWidget *origWidget;
    CVImageWidget *blurWidget;
    CVImageWidget *edgeWidget;
    CVImageWidget *structureWidget;
    CVImageWidget *colorWidget;
    CVImageWidget *restoredWidget;

    QDoubleSpinBox *gammaSpinBox;
    QDoubleSpinBox *rhoSpinBox;
    QDoubleSpinBox *betaSpinBox;
    QDoubleSpinBox *sigmaSpinBox;

    QRadioButton *n4, *n8, *n16, *n32, *n48, *n72;

public:
    explicit FiddleWindow(QWidget *parent = 0);
    ~FiddleWindow();

    void createActions();
    void createMenus();

private slots:
    void openFile();
    void updateOrig();
    void updateTensor();
    void restore();

signals:
    void nameChanged();
    void origChanged();

private:
    Ui::FiddleWindow *ui;
};

#endif // FIDDLEWINDOW_H
