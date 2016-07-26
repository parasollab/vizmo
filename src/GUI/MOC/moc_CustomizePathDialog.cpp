/****************************************************************************
** Meta object code from reading C++ file 'CustomizePathDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CustomizePathDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CustomizePathDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CustomizePathDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x0a,
      38,   20,   20,   20, 0x08,
      49,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CustomizePathDialog[] = {
    "CustomizePathDialog\0\0RestoreDefault()\0"
    "AddColor()\0AcceptData()\0"
};

void CustomizePathDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CustomizePathDialog *_t = static_cast<CustomizePathDialog *>(_o);
        switch (_id) {
        case 0: _t->RestoreDefault(); break;
        case 1: _t->AddColor(); break;
        case 2: _t->AcceptData(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CustomizePathDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CustomizePathDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CustomizePathDialog,
      qt_meta_data_CustomizePathDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CustomizePathDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CustomizePathDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CustomizePathDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CustomizePathDialog))
        return static_cast<void*>(const_cast< CustomizePathDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int CustomizePathDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
