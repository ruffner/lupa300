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

class SLIMojoLoaderWidget : public QWidget
{
    Q_OBJECT

public:
    SLIMojoLoaderWidget(QWidget *parent = 0);
    ~SLIMojoLoaderWidget();

private:
    QProgressBar *progressUpload;
    QComboBox *cmbSerial;
    QLineEdit *leBitFilePath, *leEEPROMFilePath;
    QCheckBox *ckbVerifyFlash, *ckbStoreFlash;
    QPushButton *buttonChooseBitFile, *buttonLoadBitFile, *buttonClose, *buttonChooseEEPROMFile, *buttonLoadEEPROMFile;

};

#endif // SLIMOJOLOADERWIDGET_H
