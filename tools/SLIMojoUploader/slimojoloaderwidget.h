#ifndef SLIMOJOLOADERWIDGET_H
#define SLIMOJOLOADERWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>

class SLIMojoLoaderWidget : public QWidget
{
    Q_OBJECT

public:
    SLIMojoLoaderWidget(QWidget *parent = 0);
    ~SLIMojoLoaderWidget();
};

class SLIMojoLoaderDialog : public QDialog
{
    Q_OBJECT
public:
    SLIMojoLoaderDialog(QDialog *parent) : QDialog(parent) {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(10,10,10,10);

        loaderWidget = new SLIMojoLoaderWidget();
        this->layout()->addWidget(loaderWidget);
        ((QVBoxLayout*)(this->layout()))->addSpacing(20);
    }
    ~SLIMojoLoaderDialog();

protected:
    void accept() { QDialog::accept(); }
    void reject() { QDialog::reject(); }

private:
    SLIMojoLoaderWidget *loaderWidget;
};

#endif // SLIMOJOLOADERWIDGET_H
