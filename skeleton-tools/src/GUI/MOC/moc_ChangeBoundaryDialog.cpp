/****************************************************************************
** Meta object code from reading C++ file 'ChangeBoundaryDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ChangeBoundaryDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChangeBoundaryDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ChangeBoundaryDialog[] = {

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
      22,   21,   21,   21, 0x0a,
      36,   21,   21,   21, 0x0a,
      59,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ChangeBoundaryDialog[] = {
    "ChangeBoundaryDialog\0\0SetBoundary()\0"
    "ChangeToSphereDialog()\0ChangeToBoxDialog()\0"
};

void ChangeBoundaryDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ChangeBoundaryDialog *_t = static_cast<ChangeBoundaryDialog *>(_o);
        switch (_id) {
        case 0: _t->SetBoundary(); break;
        case 1: _t->ChangeToSphereDialog(); break;
        case 2: _t->ChangeToBoxDialog(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ChangeBoundaryDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ChangeBoundaryDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ChangeBoundaryDialog,
      qt_meta_data_ChangeBoundaryDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ChangeBoundaryDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ChangeBoundaryDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ChangeBoundaryDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ChangeBoundaryDialog))
        return static_cast<void*>(const_cast< ChangeBoundaryDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ChangeBoundaryDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
