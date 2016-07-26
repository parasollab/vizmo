/****************************************************************************
** Meta object code from reading C++ file 'CaptureOptions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CaptureOptions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CaptureOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CaptureOptions[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      34,   15,   15,   15, 0x05,
      58,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      75,   15,   15,   15, 0x0a,
      84,   15,   15,   15, 0x08,
      97,   15,   15,   15, 0x08,
     114,   15,   15,   15, 0x08,
     129,   15,   15,   15, 0x08,
     150,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CaptureOptions[] = {
    "CaptureOptions\0\0SimulateMouseUp()\0"
    "ToggleSelectionSignal()\0UpdateFrame(int)\0"
    "Record()\0CropRegion()\0CapturePicture()\0"
    "CaptureMovie()\0StartLiveRecording()\0"
    "StopLiveRecording()\0"
};

void CaptureOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CaptureOptions *_t = static_cast<CaptureOptions *>(_o);
        switch (_id) {
        case 0: _t->SimulateMouseUp(); break;
        case 1: _t->ToggleSelectionSignal(); break;
        case 2: _t->UpdateFrame((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->Record(); break;
        case 4: _t->CropRegion(); break;
        case 5: _t->CapturePicture(); break;
        case 6: _t->CaptureMovie(); break;
        case 7: _t->StartLiveRecording(); break;
        case 8: _t->StopLiveRecording(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CaptureOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CaptureOptions::staticMetaObject = {
    { &OptionsBase::staticMetaObject, qt_meta_stringdata_CaptureOptions,
      qt_meta_data_CaptureOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CaptureOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CaptureOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CaptureOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CaptureOptions))
        return static_cast<void*>(const_cast< CaptureOptions*>(this));
    return OptionsBase::qt_metacast(_clname);
}

int CaptureOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OptionsBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void CaptureOptions::SimulateMouseUp()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void CaptureOptions::ToggleSelectionSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void CaptureOptions::UpdateFrame(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
