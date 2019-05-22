/****************************************************************************
** Meta object code from reading C++ file 'EditRobotDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../EditRobotDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EditRobotDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EditRobotDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      28,   16,   16,   16, 0x0a,
      40,   16,   16,   16, 0x0a,
      59,   16,   16,   16, 0x0a,
      79,   16,   16,   16, 0x0a,
     103,   16,   16,   16, 0x0a,
     130,   16,   16,   16, 0x0a,
     154,   16,   16,   16, 0x0a,
     167,   16,   16,   16, 0x0a,
     177,   16,   16,   16, 0x0a,
     188,   16,   16,   16, 0x0a,
     201,   16,   16,   16, 0x0a,
     218,   16,   16,   16, 0x0a,
     244,   16,   16,   16, 0x0a,
     268,   16,   16,   16, 0x0a,
     291,   16,   16,   16, 0x0a,
     305,   16,   16,   16, 0x0a,
     316,   16,   16,   16, 0x0a,
     337,  328,   16,   16, 0x0a,
     355,   16,   16,   16, 0x2a,
     369,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_EditRobotDialog[] = {
    "EditRobotDialog\0\0ShowBase()\0ShowJoint()\0"
    "BaseFixedChecked()\0BasePlanarChecked()\0"
    "BaseVolumetricChecked()\0"
    "BaseTranslationalChecked()\0"
    "BaseRotationalChecked()\0DeleteBase()\0"
    "AddBase()\0SaveBase()\0UpdateBase()\0"
    "CreateNewRobot()\0JointNonActuatedChecked()\0"
    "JointSphericalChecked()\0JointRevoluteChecked()\0"
    "DeleteJoint()\0AddJoint()\0SaveJoint()\0"
    "_clicked\0UpdateJoint(bool)\0UpdateJoint()\0"
    "Accept()\0"
};

void EditRobotDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        EditRobotDialog *_t = static_cast<EditRobotDialog *>(_o);
        switch (_id) {
        case 0: _t->ShowBase(); break;
        case 1: _t->ShowJoint(); break;
        case 2: _t->BaseFixedChecked(); break;
        case 3: _t->BasePlanarChecked(); break;
        case 4: _t->BaseVolumetricChecked(); break;
        case 5: _t->BaseTranslationalChecked(); break;
        case 6: _t->BaseRotationalChecked(); break;
        case 7: _t->DeleteBase(); break;
        case 8: _t->AddBase(); break;
        case 9: _t->SaveBase(); break;
        case 10: _t->UpdateBase(); break;
        case 11: _t->CreateNewRobot(); break;
        case 12: _t->JointNonActuatedChecked(); break;
        case 13: _t->JointSphericalChecked(); break;
        case 14: _t->JointRevoluteChecked(); break;
        case 15: _t->DeleteJoint(); break;
        case 16: _t->AddJoint(); break;
        case 17: _t->SaveJoint(); break;
        case 18: _t->UpdateJoint((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 19: _t->UpdateJoint(); break;
        case 20: _t->Accept(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData EditRobotDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject EditRobotDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_EditRobotDialog,
      qt_meta_data_EditRobotDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EditRobotDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EditRobotDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EditRobotDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EditRobotDialog))
        return static_cast<void*>(const_cast< EditRobotDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int EditRobotDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}
static const uint qt_meta_data_VerticalScrollArea[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_VerticalScrollArea[] = {
    "VerticalScrollArea\0"
};

void VerticalScrollArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData VerticalScrollArea::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VerticalScrollArea::staticMetaObject = {
    { &QScrollArea::staticMetaObject, qt_meta_stringdata_VerticalScrollArea,
      qt_meta_data_VerticalScrollArea, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VerticalScrollArea::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VerticalScrollArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VerticalScrollArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VerticalScrollArea))
        return static_cast<void*>(const_cast< VerticalScrollArea*>(this));
    return QScrollArea::qt_metacast(_clname);
}

int VerticalScrollArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
