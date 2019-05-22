/****************************************************************************
** Meta object code from reading C++ file 'PlanningOptions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../PlanningOptions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlanningOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PlanningOptions[] = {

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
      17,   16,   16,   16, 0x08,
      35,   16,   16,   16, 0x08,
      50,   16,   16,   16, 0x08,
      61,   16,   16,   16, 0x08,
      71,   16,   16,   16, 0x08,
      81,   16,   16,   16, 0x08,
      99,   16,   16,   16, 0x08,
     114,   16,   16,   16, 0x08,
     134,   16,   16,   16, 0x08,
     161,  152,   16,   16, 0x08,
     184,   16,   16,   16, 0x08,
     197,   16,   16,   16, 0x08,
     210,   16,   16,   16, 0x08,
     224,   16,   16,   16, 0x08,
     241,   16,   16,   16, 0x08,
     257,   16,   16,   16, 0x08,
     277,   16,   16,   16, 0x08,
     297,   16,   16,   16, 0x08,
     308,   16,   16,   16, 0x08,
     319,   16,   16,   16, 0x08,
     336,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PlanningOptions[] = {
    "PlanningOptions\0\0AddRegionSphere()\0"
    "AddRegionBox()\0PlaceCfg()\0SaveCfg()\0"
    "LoadCfg()\0DuplicateRegion()\0DeleteRegion()\0"
    "MakeRegionAttract()\0MakeRegionAvoid()\0"
    "_attract\0ChangeRegionType(bool)\0"
    "SaveRegion()\0LoadRegion()\0AddUserPath()\0"
    "DeleteUserPath()\0PrintUserPath()\0"
    "HapticPathCapture()\0CameraPathCapture()\0"
    "SavePath()\0LoadPath()\0MapEnvironment()\0"
    "ThreadDone()\0"
};

void PlanningOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PlanningOptions *_t = static_cast<PlanningOptions *>(_o);
        switch (_id) {
        case 0: _t->AddRegionSphere(); break;
        case 1: _t->AddRegionBox(); break;
        case 2: _t->PlaceCfg(); break;
        case 3: _t->SaveCfg(); break;
        case 4: _t->LoadCfg(); break;
        case 5: _t->DuplicateRegion(); break;
        case 6: _t->DeleteRegion(); break;
        case 7: _t->MakeRegionAttract(); break;
        case 8: _t->MakeRegionAvoid(); break;
        case 9: _t->ChangeRegionType((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->SaveRegion(); break;
        case 11: _t->LoadRegion(); break;
        case 12: _t->AddUserPath(); break;
        case 13: _t->DeleteUserPath(); break;
        case 14: _t->PrintUserPath(); break;
        case 15: _t->HapticPathCapture(); break;
        case 16: _t->CameraPathCapture(); break;
        case 17: _t->SavePath(); break;
        case 18: _t->LoadPath(); break;
        case 19: _t->MapEnvironment(); break;
        case 20: _t->ThreadDone(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PlanningOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PlanningOptions::staticMetaObject = {
    { &OptionsBase::staticMetaObject, qt_meta_stringdata_PlanningOptions,
      qt_meta_data_PlanningOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PlanningOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PlanningOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PlanningOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PlanningOptions))
        return static_cast<void*>(const_cast< PlanningOptions*>(this));
    return OptionsBase::qt_metacast(_clname);
}

int PlanningOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OptionsBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}
static const uint qt_meta_data_MapEnvironmentWorker[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MapEnvironmentWorker[] = {
    "MapEnvironmentWorker\0\0Finished()\0"
    "Solve()\0"
};

void MapEnvironmentWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MapEnvironmentWorker *_t = static_cast<MapEnvironmentWorker *>(_o);
        switch (_id) {
        case 0: _t->Finished(); break;
        case 1: _t->Solve(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MapEnvironmentWorker::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MapEnvironmentWorker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MapEnvironmentWorker,
      qt_meta_data_MapEnvironmentWorker, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MapEnvironmentWorker::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MapEnvironmentWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MapEnvironmentWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MapEnvironmentWorker))
        return static_cast<void*>(const_cast< MapEnvironmentWorker*>(this));
    return QObject::qt_metacast(_clname);
}

int MapEnvironmentWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void MapEnvironmentWorker::Finished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
