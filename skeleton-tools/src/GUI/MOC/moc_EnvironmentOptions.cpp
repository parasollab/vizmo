/****************************************************************************
** Meta object code from reading C++ file 'EnvironmentOptions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../EnvironmentOptions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EnvironmentOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EnvironmentOptions[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x08,
      34,   19,   19,   19, 0x08,
      51,   19,   19,   19, 0x08,
      66,   19,   19,   19, 0x08,
      87,   19,   19,   19, 0x08,
     108,   19,   19,   19, 0x08,
     120,   19,   19,   19, 0x08,
     135,   19,   19,   19, 0x08,
     149,   19,   19,   19, 0x08,
     164,   19,   19,   19, 0x08,
     176,   19,   19,   19, 0x08,
     209,  198,   19,   19, 0x08,
     252,   19,   19,   19, 0x08,
     274,   19,   19,   19, 0x08,
     288,   19,   19,   19, 0x08,
     301,   19,   19,   19, 0x08,
     316,   19,   19,   19, 0x08,
     329,   19,   19,   19, 0x08,
     342,   19,   19,   19, 0x08,
     355,   19,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_EnvironmentOptions[] = {
    "EnvironmentOptions\0\0AddObstacle()\0"
    "DeleteObstacle()\0MoveObstacle()\0"
    "DuplicateObstacles()\0ChangeBoundaryForm()\0"
    "EditRobot()\0LoadEllipses()\0AddSkeleton()\0"
    "SaveSkeleton()\0AddVertex()\0"
    "AddStraightLineEdge()\0_e1,_e2,_i\0"
    "AddSpecificEdge(EdgeModel*,EdgeModel*,int)\0"
    "DeleteSelectedItems()\0ChangeColor()\0"
    "MergeEdges()\0CollapseEdge()\0RefreshEnv()\0"
    "ClickRobot()\0ClickPoint()\0"
    "RandomizeEnvColors()\0"
};

void EnvironmentOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        EnvironmentOptions *_t = static_cast<EnvironmentOptions *>(_o);
        switch (_id) {
        case 0: _t->AddObstacle(); break;
        case 1: _t->DeleteObstacle(); break;
        case 2: _t->MoveObstacle(); break;
        case 3: _t->DuplicateObstacles(); break;
        case 4: _t->ChangeBoundaryForm(); break;
        case 5: _t->EditRobot(); break;
        case 6: _t->LoadEllipses(); break;
        case 7: _t->AddSkeleton(); break;
        case 8: _t->SaveSkeleton(); break;
        case 9: _t->AddVertex(); break;
        case 10: _t->AddStraightLineEdge(); break;
        case 11: _t->AddSpecificEdge((*reinterpret_cast< EdgeModel*(*)>(_a[1])),(*reinterpret_cast< EdgeModel*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 12: _t->DeleteSelectedItems(); break;
        case 13: _t->ChangeColor(); break;
        case 14: _t->MergeEdges(); break;
        case 15: _t->CollapseEdge(); break;
        case 16: _t->RefreshEnv(); break;
        case 17: _t->ClickRobot(); break;
        case 18: _t->ClickPoint(); break;
        case 19: _t->RandomizeEnvColors(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData EnvironmentOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject EnvironmentOptions::staticMetaObject = {
    { &OptionsBase::staticMetaObject, qt_meta_stringdata_EnvironmentOptions,
      qt_meta_data_EnvironmentOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EnvironmentOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EnvironmentOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EnvironmentOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EnvironmentOptions))
        return static_cast<void*>(const_cast< EnvironmentOptions*>(this));
    return OptionsBase::qt_metacast(_clname);
}

int EnvironmentOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OptionsBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
