/****************************************************************************
** Meta object code from reading C++ file 'RoadmapOptions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../RoadmapOptions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RoadmapOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RoadmapOptions[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x08,
      30,   15,   15,   15, 0x08,
      51,   15,   15,   15, 0x08,
      64,   15,   15,   15, 0x08,
      90,   15,   15,   15, 0x08,
     112,   15,   15,   15, 0x08,
     141,   15,   15,   15, 0x08,
     166,   15,   15,   15, 0x08,
     187,   15,   15,   15, 0x08,
     208,   15,   15,   15, 0x08,
     218,   15,   15,   15, 0x08,
     240,   15,   15,   15, 0x08,
     262,   15,   15,   15, 0x08,
     283,   15,   15,   15, 0x08,
     303,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RoadmapOptions[] = {
    "RoadmapOptions\0\0ShowRoadmap()\0"
    "ShowNodeSizeDialog()\0ScaleNodes()\0"
    "ShowEdgeThicknessDialog()\0"
    "ChangeEdgeThickness()\0"
    "ShowNumIntermediatesDialog()\0"
    "ChangeNumIntermediates()\0ShowNodeEditDialog()\0"
    "ShowEdgeEditDialog()\0AddNode()\0"
    "AddStraightLineEdge()\0DeleteSelectedItems()\0"
    "MergeSelectedNodes()\0RandomizeCCColors()\0"
    "MakeCCsOneColor()\0"
};

void RoadmapOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RoadmapOptions *_t = static_cast<RoadmapOptions *>(_o);
        switch (_id) {
        case 0: _t->ShowRoadmap(); break;
        case 1: _t->ShowNodeSizeDialog(); break;
        case 2: _t->ScaleNodes(); break;
        case 3: _t->ShowEdgeThicknessDialog(); break;
        case 4: _t->ChangeEdgeThickness(); break;
        case 5: _t->ShowNumIntermediatesDialog(); break;
        case 6: _t->ChangeNumIntermediates(); break;
        case 7: _t->ShowNodeEditDialog(); break;
        case 8: _t->ShowEdgeEditDialog(); break;
        case 9: _t->AddNode(); break;
        case 10: _t->AddStraightLineEdge(); break;
        case 11: _t->DeleteSelectedItems(); break;
        case 12: _t->MergeSelectedNodes(); break;
        case 13: _t->RandomizeCCColors(); break;
        case 14: _t->MakeCCsOneColor(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData RoadmapOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RoadmapOptions::staticMetaObject = {
    { &OptionsBase::staticMetaObject, qt_meta_stringdata_RoadmapOptions,
      qt_meta_data_RoadmapOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RoadmapOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RoadmapOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RoadmapOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RoadmapOptions))
        return static_cast<void*>(const_cast< RoadmapOptions*>(this));
    return OptionsBase::qt_metacast(_clname);
}

int RoadmapOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OptionsBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
