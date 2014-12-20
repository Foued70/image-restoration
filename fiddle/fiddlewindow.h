#ifndef FIDDLEWINDOW_H
#define FIDDLEWINDOW_H

#include <QMainWindow>
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

public:
    explicit FiddleWindow(QWidget *parent = 0);
    ~FiddleWindow();

    void createActions();
    void createMenus();

private slots:
    void openFile();
    void updateOrig();
    void updateBlur();

signals:
    void nameChanged();
    void origChanged();

private:
    Ui::FiddleWindow *ui;
};

#endif // FIDDLEWINDOW_H
