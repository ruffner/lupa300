#include "slimojoloaderwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SLIMojoLoaderWidget d;
    d.show();

    return a.exec();
}
