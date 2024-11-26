/********************************************************************************
** Form generated from reading UI file 'Dispaly.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISPALY_H
#define UI_DISPALY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DispalyClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DispalyClass)
    {
        if (DispalyClass->objectName().isEmpty())
            DispalyClass->setObjectName(QString::fromUtf8("DispalyClass"));
        DispalyClass->resize(600, 400);
        menuBar = new QMenuBar(DispalyClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        DispalyClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DispalyClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DispalyClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(DispalyClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        DispalyClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(DispalyClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DispalyClass->setStatusBar(statusBar);

        retranslateUi(DispalyClass);

        QMetaObject::connectSlotsByName(DispalyClass);
    } // setupUi

    void retranslateUi(QMainWindow *DispalyClass)
    {
        DispalyClass->setWindowTitle(QApplication::translate("DispalyClass", "Dispaly", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DispalyClass: public Ui_DispalyClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISPALY_H
