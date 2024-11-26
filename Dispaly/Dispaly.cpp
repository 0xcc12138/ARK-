#include "Dispaly.h"
#include "Kernel.h"
#include <qdebug.h>
int count = 0;
pEprocessInfoArray* pArray = NULL;
Dispaly::Dispaly(QWidget *parent)
    : QMainWindow(parent),
    tableView(new QTableView(this)),       // ���������ͼ
    tableModel(new QStandardItemModel(this)), // ��������ģ��
    button(new QPushButton("����",this))
{
    ui.setupUi(this);

    // ���ô��ڱ���
    setWindowTitle("��ťʾ��");

    // �������봰��
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ������ֱ����
    QVBoxLayout* mainLayout = new QVBoxLayout();

    // 1. ��� QTableView �� QPushButton ������
    mainLayout->addWidget(tableView);  // ��ӱ����ͼ
    mainLayout->addWidget(button);    // ��Ӱ�ť

    // ���ò���
    centralWidget->setLayout(mainLayout);

    // ���ð�ť�Ĵ�С
    button->resize(200, 100);

    // ���ð�ť�ĵ���¼�����
    connect(button, &QPushButton::clicked, this, &Dispaly::PushButtonFunc);

    // ���ñ����ͼģ��
    tableView->setModel(tableModel);

    // ��ʼ�����ģ��
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
    // ���ñ�ͷ
    tableModel->setHorizontalHeaderLabels({ "������", "EPROCESS��ַ", "Count" });

    // ʾ������
    QList<QList<QVariant>> data = {
        {"notepad.exe", "0xFFFFF80612345678", 12},
        {"explorer.exe", "0xFFFFF80623456789", 25},
        {"chrome.exe", "0xFFFFF80634567890", 43}
    };

    // �����ݼ��ص�ģ��
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

    // ������е�����
    tableModel->removeRows(0, tableModel->rowCount());

    // ���±������
    for (int i = 0; i < count; i++)
    {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::fromLocal8Bit(pArray[i].ExeName)));  // ������
        items.append(new QStandardItem(QString::number((pArray[i].m_PEprocess), 16)));
        items.append(new QStandardItem(QString::number(i + 1)));  // Count��ʾ���ǵ��������֣�

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



	// �ȴ��¼����ź�
	qDebug() << "�ȴ��¼�";
	qDebug() << "����¼�";
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
		qDebug() << "EPROCESS�����ַΪ:" << pArray[i].m_PEprocess << " " << "������Ϊ:" << pArray[i].ExeName;
	}


    

    // �رվ��
    CloseHandle(hEvent);
    //system("pause");
    return ;

#endif
}