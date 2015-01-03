/********************************************************************************
** Form generated from reading UI file 'fiddlewindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FIDDLEWINDOW_H
#define UI_FIDDLEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FiddleWindow
{
public:
    QWidget *centralWidget;
    QStatusBar *statusBar;
    QMenuBar *menuBar;

    void setupUi(QMainWindow *FiddleWindow)
    {
        if (FiddleWindow->objectName().isEmpty())
            FiddleWindow->setObjectName(QStringLiteral("FiddleWindow"));
        FiddleWindow->resize(400, 300);
        centralWidget = new QWidget(FiddleWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        FiddleWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(FiddleWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        FiddleWindow->setStatusBar(statusBar);
        menuBar = new QMenuBar(FiddleWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 25));
        FiddleWindow->setMenuBar(menuBar);

        retranslateUi(FiddleWindow);

        QMetaObject::connectSlotsByName(FiddleWindow);
    } // setupUi

    void retranslateUi(QMainWindow *FiddleWindow)
    {
        FiddleWindow->setWindowTitle(QApplication::translate("FiddleWindow", "FiddleWindow", 0));
    } // retranslateUi

};

namespace Ui {
    class FiddleWindow: public Ui_FiddleWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FIDDLEWINDOW_H
