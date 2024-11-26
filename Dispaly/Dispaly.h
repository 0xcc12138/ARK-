#pragma once

#pragma execution_character_set("utf-8")


#include <QtWidgets/QMainWindow>
#include <windows.h>
#include "ui_Dispaly.h"
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <qpushbutton.h>
#include <QVBoxLayout>
class Dispaly : public QMainWindow
{
    Q_OBJECT

public:
    Dispaly(QWidget* parent = nullptr);
    ~Dispaly();

private:
    Ui::DispalyClass ui;


private:
    QTableView* tableView;           // �����ͼ
    QStandardItemModel* tableModel; // ����ģ��

    void setupTableViewModel();      // ��ʼ��ģ�ͺ���ͼ

    void GetMessageFromKernel();

    // ��ť��Ա����
    QPushButton* button;


public slots:
    // ��ť�����Ĳۺ���
    void PushButtonFunc();

    void updateTable();




};
