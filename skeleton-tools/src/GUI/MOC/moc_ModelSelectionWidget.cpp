/****************************************************************************
** Meta object code from reading C++ file 'ModelSelectionWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ModelSelectionWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModelSelectionWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ModelSelectionWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,
      36,   21,   21,   21, 0x05,
      55,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      77,   21,   21,   21, 0x0a,
      90,   21,   21,   21, 0x0a,
      99,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ModelSelectionWidget[] = {
    "ModelSelectionWidget\0\0ResetSignal()\0"
    "UpdateTextWidget()\0ResetListsFromMPSig()\0"
    "ResetLists()\0Select()\0SelectionChanged()\0"
};

void ModelSelectionWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ModelSelectionWidget *_t = static_cast<ModelSelectionWidget *>(_o);
        switch (_id) {
        case 0: _t->ResetSignal(); break;
        case 1: _t->UpdateTextWidget(); break;
        case 2: _t->ResetListsFromMPSig(); break;
        case 3: _t->ResetLists(); break;
        case 4: _t->Select(); break;
        case 5: _t->SelectionChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ModelSelectionWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ModelSelectionWidget::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_ModelSelectionWidget,
      qt_meta_data_ModelSelectionWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ModelSelectionWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ModelSelectionWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ModelSelectionWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ModelSelectionWidget))
        return static_cast<void*>(const_cast< ModelSelectionWidget*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int ModelSelectionWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ModelSelectionWidget::ResetSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ModelSelectionWidget::UpdateTextWidget()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void ModelSelectionWidget::ResetListsFromMPSig()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
