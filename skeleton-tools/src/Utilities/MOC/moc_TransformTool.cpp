/****************************************************************************
** Meta object code from reading C++ file 'TransformTool.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../TransformTool.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TransformTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TransformTool[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   15,   14,   14, 0x05,
      50,   47,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      78,   15,   14,   14, 0x0a,
     106,   47,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_TransformTool[] = {
    "TransformTool\0\0_t\0TranslationChanged(Vector3d)\0"
    "_r\0RotationChanged(Quaternion)\0"
    "ChangeTranslation(Vector3d)\0"
    "ChangeRotation(Quaternion)\0"
};

void TransformTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TransformTool *_t = static_cast<TransformTool *>(_o);
        switch (_id) {
        case 0: _t->TranslationChanged((*reinterpret_cast< const Vector3d(*)>(_a[1]))); break;
        case 1: _t->RotationChanged((*reinterpret_cast< const Quaternion(*)>(_a[1]))); break;
        case 2: _t->ChangeTranslation((*reinterpret_cast< const Vector3d(*)>(_a[1]))); break;
        case 3: _t->ChangeRotation((*reinterpret_cast< const Quaternion(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TransformTool::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TransformTool::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_TransformTool,
      qt_meta_data_TransformTool, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TransformTool::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TransformTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TransformTool::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TransformTool))
        return static_cast<void*>(const_cast< TransformTool*>(this));
    return QObject::qt_metacast(_clname);
}

int TransformTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void TransformTool::TranslationChanged(const Vector3d & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TransformTool::RotationChanged(const Quaternion & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
