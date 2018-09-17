/****************************************************************************
** Meta object code from reading C++ file 'qzeroconf.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qzeroconf.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qzeroconf.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QZeroConf[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,   29,   29,   29, 0x05,
      30,   29,   29,   29, 0x05,
      56,   29,   29,   29, 0x05,
      88,   29,   29,   29, 0x05,
     122,   29,   29,   29, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QZeroConf[] = {
    "QZeroConf\0servicePublished()\0\0"
    "error(QZeroConf::error_t)\0"
    "serviceAdded(QZeroConfService*)\0"
    "serviceUpdated(QZeroConfService*)\0"
    "serviceRemoved(QZeroConfService*)\0"
};

void QZeroConf::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QZeroConf *_t = static_cast<QZeroConf *>(_o);
        switch (_id) {
        case 0: _t->servicePublished(); break;
        case 1: _t->error((*reinterpret_cast< QZeroConf::error_t(*)>(_a[1]))); break;
        case 2: _t->serviceAdded((*reinterpret_cast< QZeroConfService*(*)>(_a[1]))); break;
        case 3: _t->serviceUpdated((*reinterpret_cast< QZeroConfService*(*)>(_a[1]))); break;
        case 4: _t->serviceRemoved((*reinterpret_cast< QZeroConfService*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QZeroConf::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QZeroConf::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QZeroConf,
      qt_meta_data_QZeroConf, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QZeroConf::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QZeroConf::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QZeroConf::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QZeroConf))
        return static_cast<void*>(const_cast< QZeroConf*>(this));
    return QObject::qt_metacast(_clname);
}

int QZeroConf::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void QZeroConf::servicePublished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QZeroConf::error(QZeroConf::error_t _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QZeroConf::serviceAdded(QZeroConfService * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QZeroConf::serviceUpdated(QZeroConfService * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QZeroConf::serviceRemoved(QZeroConfService * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
