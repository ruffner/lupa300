#ifndef SLIMOJOLOADERWIDGET_H
#define SLIMOJOLOADERWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QFileDialog>

#include "mojoloaderobject.h"

class SLIMojoLoaderWidget : public QWidget
{
    Q_OBJECT

public:
    SLIMojoLoaderWidget(QWidget *parent = 0);
    ~SLIMojoLoaderWidget();

public slots:
    void onLoadBitFile();
    void onChooseSerial();
    void onChooseBitFile();
    void onLoadEEPROMFile();
    void onChooseEEPROMFile();
    void onLoaderStatusChanged(QString status);

private:
    MojoLoaderObject *mojoLoader;

    QProgressBar *progressUpload;
    QComboBox *cmbSerial;
    QLabel *labelStatus;
    QLineEdit *leBitFilePath, *leEEPROMFilePath;
    QCheckBox *ckbVerifyFlash, *ckbStoreFlash, *ckbVerifyEEPROM;
    QPushButton *buttonChooseBitFile, *buttonLoadBitFile, *buttonChooseEEPROMFile, *buttonLoadEEPROMFile, *buttonSerialChoose;

};

#endif // SLIMOJOLOADERWIDGET_H
