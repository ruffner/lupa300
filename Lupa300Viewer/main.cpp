#include "lau3dvideotcpserver.h"
#include "lau3dvideotcpwidget.h"
#include <QSurfaceFormat>
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));
    a.setApplicationName(QString("LAURealSense"));

    QSurfaceFormat format;
    format.setDepthBufferSize(10);
    format.setMajorVersion(4);
    format.setMinorVersion(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);

    qRegisterMetaType<LAUMemoryObject>("LAUMemoryObject");
    qRegisterMetaType<LAUScan>("LAUScan");

    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    a.setQuitOnLastWindowClosed(true);

    //LAU3DVideoTCPServer s(ColorGray, DeviceIDS);
    LAU3DVideoTCPWidget c(ColorGray, DeviceIDS);
    c.show();
    return (a.exec());
}
