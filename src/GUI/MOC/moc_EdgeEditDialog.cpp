/****************************************************************************
** Meta object code from reading C++ file 'EdgeEditDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../EdgeEditDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EdgeEditDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CfgListWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CfgListWidget[] = {
    "CfgListWidget\0\0SelectInMap()\0"
};

void CfgListWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CfgListWidget *_t = static_cast<CfgListWidget *>(_o);
        switch (_id) {
        case 0: _t->SelectInMap(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CfgListWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CfgListWidget::staticMetaObject = {
    { &QListWidget::staticMetaObject, qt_meta_stringdata_CfgListWidget,
      qt_meta_data_CfgListWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CfgListWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CfgListWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CfgListWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CfgListWidget))
        return static_cast<void*>(const_cast< CfgListWidget*>(this));
    return QListWidget::qt_metacast(_clname);
}

int CfgListWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_EdgeEditDialog[] = {

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
      16,   15,   15,   15, 0x08,
      35,   15,   15,   15, 0x08,
      53,   15,   15,   15, 0x08,
      84,   74,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_EdgeEditDialog[] = {
    "EdgeEditDialog\0\0EditIntermediate()\0"
    "AddIntermediate()\0RemoveIntermediate()\0"
    "_accepted\0FinalizeEdgeEdit(int)\0"
};

void EdgeEditDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        EdgeEditDialog *_t = static_cast<EdgeEditDialog *>(_o);
        switch (_id) {
        case 0: _t->EditIntermediate(); break;
        case 1: _t->AddIntermediate(); break;
        case 2: _t->RemoveIntermediate(); break;
        case 3: _t->FinalizeEdgeEdit((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData EdgeEditDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject EdgeEditDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_EdgeEditDialog,
      qt_meta_data_EdgeEditDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EdgeEditDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EdgeEditDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EdgeEditDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EdgeEditDialog))
        return static_cast<void*>(const_cast< EdgeEditDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int EdgeEditDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
