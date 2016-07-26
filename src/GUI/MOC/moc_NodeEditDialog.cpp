/****************************************************************************
** Meta object code from reading C++ file 'NodeEditDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../NodeEditDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NodeEditDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NodeEditSlider[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   16,   15,   15, 0x08,
      54,   44,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NodeEditSlider[] = {
    "NodeEditSlider\0\0_newVal\0UpdateDOFLabel(int)\0"
    "_inputVal\0MoveSlider(QString)\0"
};

void NodeEditSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NodeEditSlider *_t = static_cast<NodeEditSlider *>(_o);
        switch (_id) {
        case 0: _t->UpdateDOFLabel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->MoveSlider((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NodeEditSlider::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NodeEditSlider::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_NodeEditSlider,
      qt_meta_data_NodeEditSlider, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NodeEditSlider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NodeEditSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NodeEditSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NodeEditSlider))
        return static_cast<void*>(const_cast< NodeEditSlider*>(this));
    return QWidget::qt_metacast(_clname);
}

int NodeEditSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_NodeEditDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   16,   15,   15, 0x08,
      45,   35,   15,   15, 0x08,
      67,   35,   15,   15, 0x08,
      88,   35,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NodeEditDialog[] = {
    "NodeEditDialog\0\0_id\0UpdateDOF(int)\0"
    "_accepted\0FinalizeNodeEdit(int)\0"
    "FinalizeNodeAdd(int)\0FinalizeNodeMerge(int)\0"
};

void NodeEditDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NodeEditDialog *_t = static_cast<NodeEditDialog *>(_o);
        switch (_id) {
        case 0: _t->UpdateDOF((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->FinalizeNodeEdit((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->FinalizeNodeAdd((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->FinalizeNodeMerge((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NodeEditDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NodeEditDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_NodeEditDialog,
      qt_meta_data_NodeEditDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NodeEditDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NodeEditDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NodeEditDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NodeEditDialog))
        return static_cast<void*>(const_cast< NodeEditDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int NodeEditDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
