#include "Dispaly.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dispaly w;
    w.show();
    return a.exec();
}
