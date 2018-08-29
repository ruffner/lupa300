#ifndef MOJOLOADEROBJECT_H
#define MOJOLOADEROBJECT_H

#include <QObject>

class MojoLoaderObject : public QObject
{
    Q_OBJECT
public:
    explicit MojoLoaderObject(QObject *parent = nullptr);

signals:

public slots:
};

#endif // MOJOLOADEROBJECT_H