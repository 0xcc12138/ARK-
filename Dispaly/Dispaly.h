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
    QTableView* tableView;           // 表格视图
    QStandardItemModel* tableModel; // 数据模型

    void setupTableViewModel();      // 初始化模型和视图

    void GetMessageFromKernel();

    // 按钮成员变量
    QPushButton* button;


public slots:
    // 按钮点击后的槽函数
    void PushButtonFunc();

    void updateTable();




};
