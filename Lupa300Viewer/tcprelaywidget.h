#ifndef TCPRELAYWIDGET_H
#define TCPRELAYWIDGET_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkSession>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>

// representing pending response types
#define COMMAND_ON  0xAA
#define COMMAND_OFF 0xBB
#define COMMAND_ID  0xCC

// responses from wifi relay upon HTTP request
#define RESPONSE_ON  "ON"
#define RESPONSE_OFF "OFF"
#define RESPONSE_ID  "WIFI RELAY"

class TCPRelayWidget : public QObject
{
    Q_OBJECT
public:
    explicit TCPRelayWidget(QObject *parent = nullptr, bool autoConnect = false);
    ~TCPRelayWidget();

    void verify();
    bool isVerified() { return deviceVerified; }

    int relayState;

private:
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    int pendingCommand;
    bool pending, commandVerified, deviceVerified;

signals:
    void emitRelayError(QString);
    void emitRelayState(bool);
    void emitDeviceVerified(bool);

public slots:
    void onSetRelayState(bool state);
    void onSetRelayOn();
    void onSetRelayOff();

private slots:
    void managerFinished(QNetworkReply *reply);
};

#endif // TCPRELAYWIDGET_H
