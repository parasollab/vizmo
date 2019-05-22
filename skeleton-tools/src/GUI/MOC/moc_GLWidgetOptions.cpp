/****************************************************************************
** Meta object code from reading C++ file 'GLWidgetOptions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../GLWidgetOptions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GLWidgetOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GLWidgetOptions[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      31,   16,   16,   16, 0x08,
      51,   16,   16,   16, 0x08,
      72,   16,   16,   16, 0x08,
      93,   16,   16,   16, 0x08,
     112,   16,   16,   16, 0x08,
     128,   16,   16,   16, 0x08,
     148,   16,   16,   16, 0x08,
     160,   16,   16,   16, 0x08,
     172,   16,   16,   16, 0x08,
     188,   16,   16,   16, 0x08,
     208,   16,   16,   16, 0x08,
     224,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_GLWidgetOptions[] = {
    "GLWidgetOptions\0\0ResetCamera()\0"
    "SetCameraPosition()\0SaveCameraPosition()\0"
    "LoadCameraPosition()\0ToggleCameraFree()\0"
    "ResetCameraUp()\0ChangeObjectColor()\0"
    "MakeSolid()\0MakeWired()\0MakeInvisible()\0"
    "ShowObjectNormals()\0ChangeBGColor()\0"
    "ShowGeneralContextMenu()\0"
};

void GLWidgetOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GLWidgetOptions *_t = static_cast<GLWidgetOptions *>(_o);
        switch (_id) {
        case 0: _t->ResetCamera(); break;
        case 1: _t->SetCameraPosition(); break;
        case 2: _t->SaveCameraPosition(); break;
        case 3: _t->LoadCameraPosition(); break;
        case 4: _t->ToggleCameraFree(); break;
        case 5: _t->ResetCameraUp(); break;
        case 6: _t->ChangeObjectColor(); break;
        case 7: _t->MakeSolid(); break;
        case 8: _t->MakeWired(); break;
        case 9: _t->MakeInvisible(); break;
        case 10: _t->ShowObjectNormals(); break;
        case 11: _t->ChangeBGColor(); break;
        case 12: _t->ShowGeneralContextMenu(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GLWidgetOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GLWidgetOptions::staticMetaObject = {
    { &OptionsBase::staticMetaObject, qt_meta_stringdata_GLWidgetOptions,
      qt_meta_data_GLWidgetOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GLWidgetOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GLWidgetOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GLWidgetOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GLWidgetOptions))
        return static_cast<void*>(const_cast< GLWidgetOptions*>(this));
    return OptionsBase::qt_metacast(_clname);
}

int GLWidgetOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OptionsBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
