#ifndef TCPRELAYWIDGET_H
#define TCPRELAYWIDGET_H

#include <QObject>

class TCPRelayWidget : public QObject
{
    Q_OBJECT
public:
    explicit TCPRelayWidget(QObject *parent = nullptr);

signals:

public slots:
};

#endif // TCPRELAYWIDGET_H