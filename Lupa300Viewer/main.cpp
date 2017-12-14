#include "lupa300viewerdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Lupa300ViewerWidget w;
    w.show();

    return a.exec();
}
