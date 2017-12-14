#include "lupa300viewerwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Lupa300ViewerWidget w;
    w.show();

    return a.exec();
}
