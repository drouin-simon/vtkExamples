#include "mfdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MFDialog w;
    w.show();

    return a.exec();
}
