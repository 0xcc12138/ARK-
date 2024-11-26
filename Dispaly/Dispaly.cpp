#include "Dispaly.h"
#include "Kernel.h"
#include <qdebug.h>
int count = 0;
pEprocessInfoArray* pArray = NULL;
Dispaly::Dispaly(QWidget *parent)
    : QMainWindow(parent),
    tableView(new QTableView(this)),       // 创建表格视图
    tableModel(new QStandardItemModel(this)), // 创建数据模型
    button(new QPushButton("点我",this))
{
    ui.setupUi(this);

    // 设置窗口标题
    setWindowTitle("按钮示例");

    // 创建中央窗口
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout();

    // 1. 添加 QTableView 和 QPushButton 到布局
    mainLayout->addWidget(tableView);  // 添加表格视图
    mainLayout->addWidget(button);    // 添加按钮

    // 设置布局
    centralWidget->setLayout(mainLayout);

    // 设置按钮的大小
    button->resize(200, 100);

    // 设置按钮的点击事件连接
    connect(button, &QPushButton::clicked, this, &Dispaly::PushButtonFunc);

    // 设置表格视图模型
    tableView->setModel(tableModel);

    // 初始化表格模型
    setupTableViewModel();


}




void Dispaly::PushButtonFunc()
{
    GetMessageFromKernel();
    updateTable();
    return;
}





Dispaly::~Dispaly()
{}



void Dispaly::setupTableViewModel()
{
    // 设置表头
    tableModel->setHorizontalHeaderLabels({ "进程名", "EPROCESS地址", "Count" });

    // 示例数据
    QList<QList<QVariant>> data = {
        {"notepad.exe", "0xFFFFF80612345678", 12},
        {"explorer.exe", "0xFFFFF80623456789", 25},
        {"chrome.exe", "0xFFFFF80634567890", 43}
    };

    // 将数据加载到模型
    for (const auto& row : data) {
        QList<QStandardItem*> items;
        for (const auto& value : row) {
            items.append(new QStandardItem(value.toString()));
        }
        tableModel->appendRow(items);
    }
}



void Dispaly::updateTable()
{
    if (pArray == NULL) {
        qDebug() << "No process data to display!";
        return;
    }

    // 清空现有的数据
    tableModel->removeRows(0, tableModel->rowCount());

    // 更新表格内容
    for (int i = 0; i < count; i++)
    {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::fromLocal8Bit(pArray[i].ExeName)));  // 进程名
        items.append(new QStandardItem(QString::number((pArray[i].m_PEprocess), 16)));
        items.append(new QStandardItem(QString::number(i + 1)));  // Count（示例是递增的数字）

        tableModel->appendRow(items);
    }
    delete[] pArray;
}


void Dispaly::GetMessageFromKernel()
{
#ifdef _WIN32
    count = 0;
    HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, L"Global\\ProcEvent");
    if (hEvent == NULL)
    {
        qDebug() << "Failed to open event: " << QString::number(GetLastError(), 16);
        //system("pause");
        return ;
    }
    
    HANDLE hDevice = CreateFile(L"\\\\.\\Search_Processes_Link", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        qDebug() << "can't open the device!";
        qDebug() << "Failed to open device: " << QString::number(GetLastError(), 16);
        //system("pause");
        return ;
    }



	// 等待事件被信号
	qDebug() << "等待事件";
	qDebug() << "获得事件";
	DWORD dwBytesReturned;

	pArray = new pEprocessInfoArray[16384];


	BOOL bResult = DeviceIoControl(
		hDevice,
		IOCTL_PROCWATCH,
		nullptr,
		NULL,
		pArray,
		sizeof(pEprocessInfoArray) * 16384 + 1,
		&dwBytesReturned,
		NULL
	);

	count = dwBytesReturned / sizeof(pEprocessInfoArray);
	for (int i = 0; i < count; i++)
	{
		qDebug() << "EPROCESS对象地址为:" << pArray[i].m_PEprocess << " " << "进程名为:" << pArray[i].ExeName;
	}


    

    // 关闭句柄
    CloseHandle(hEvent);
    //system("pause");
    return ;

#endif
}