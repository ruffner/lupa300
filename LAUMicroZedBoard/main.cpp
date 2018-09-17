#include <QCoreApplication>
#include "lau3dvideotcpserver.h"

#define PI 3.14159265359
#define LUPA300_WIDTH  640
#define LUPA300_HEIGHT 480

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<LAUMemoryObject>("LAUMemoryObject");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    LAU3DVideoTCPServer s(ColorGray, DeviceIDS);

    return a.exec();
}
