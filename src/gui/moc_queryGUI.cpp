/****************************************************************************
** Meta object code from reading C++ file 'queryGUI.h'
**
** Created: Fri Aug 27 15:06:30 2010
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "queryGUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'queryGUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_queryGUI[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      23,    9,    9,    9, 0x08,
      32,    9,    9,    9, 0x08,
      48,    9,    9,    9, 0x08,
      63,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_queryGUI[] = {
    "queryGUI\0\0callUpdate()\0SaveSG()\0newCfg(QString)\0updateQryCfg()\0"
    "resetCfg()\0"
};

const QMetaObject queryGUI::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_queryGUI,
      qt_meta_data_queryGUI, 0 }
};

const QMetaObject *queryGUI::metaObject() const
{
    return &staticMetaObject;
}

void *queryGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_queryGUI))
	return static_cast<void*>(const_cast<queryGUI*>(this));
    return QDialog::qt_metacast(_clname);
}

int queryGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: callUpdate(); break;
        case 1: SaveSG(); break;
        case 2: newCfg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: updateQryCfg(); break;
        case 4: resetCfg(); break;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void queryGUI::callUpdate()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
