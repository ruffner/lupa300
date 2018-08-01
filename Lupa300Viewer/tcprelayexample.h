#ifndef LUPA300VIEWERWIDGET_H
#define LUPA300VIEWERWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

#include "tcprelaywidget.h"

class Lupa300ViewerWidget : public QWidget
{
    Q_OBJECT

public:
    Lupa300ViewerWidget(QWidget *parent = 0);
    ~Lupa300ViewerWidget();

private:
    QLabel *statusLabel;
    QPushButton *relayToggleButton;
    TCPRelayWidget *relay;

private slots:
    void onRelayError(QString err);
    void onRelayVerified(bool);
    void onRelayStateChanged(bool state);
};

#endif // LUPA300VIEWERWIDGET_H
