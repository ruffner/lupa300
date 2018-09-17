#include "lupa300viewerdialog.h"

Lupa300ViewerWidget::Lupa300ViewerWidget(QWidget *parent)
    : QWidget(parent), relayToggleButton(NULL), relay(NULL), statusLabel(NULL)
{
    relay = new TCPRelayWidget();

    statusLabel = new QLabel("relay disconnected");

    this->setLayout(new QVBoxLayout());
    relayToggleButton = new QPushButton("TOGGLE RELAY");
    relayToggleButton->setCheckable(true);

    this->layout()->addWidget(relayToggleButton);
    this->layout()->addWidget(statusLabel);


    connect(relayToggleButton, SIGNAL(clicked(bool)), relay, SLOT(onSetRelayState(bool)));
    connect(relay, SIGNAL(emitDeviceVerified(bool)), this, SLOT(onRelayVerified(bool)));
    connect(relay, SIGNAL(emitRelayState(bool)), this, SLOT(onRelayStateChanged(bool)));
    connect(relay, SIGNAL(emitRelayError(QString)), this, SLOT(onRelayError(QString)));
}

Lupa300ViewerWidget::~Lupa300ViewerWidget()
{
    delete relay;
}

void Lupa300ViewerWidget::onRelayError(QString err)
{
    statusLabel->setText(err);
}

void Lupa300ViewerWidget::onRelayVerified(bool con)
{
    statusLabel->setText(QString("RELAY ") + QString(con ? "IS " : "IS NOT ") + QString("CONNECTED"));
}

void Lupa300ViewerWidget::onRelayStateChanged(bool newState)
{
    statusLabel->setText(QString("RELAY IS ") + QString(newState ? "ON" : "OFF"));
}
