/****************************************************************************
** Meta object code from reading C++ file 'FileOptions.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../FileOptions.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FileOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FileOptions[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x08,
      24,   12,   12,   12, 0x08,
      38,   12,   12,   12, 0x08,
      48,   12,   12,   12, 0x08,
      62,   12,   12,   12, 0x08,
      76,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FileOptions[] = {
    "FileOptions\0\0LoadFile()\0UpdateFiles()\0"
    "SaveEnv()\0SaveQryFile()\0SaveRoadmap()\0"
    "SavePath()\0"
};

void FileOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FileOptions *_t = static_cast<FileOptions *>(_o);
        switch (_id) {
        case 0: _t->LoadFile(); break;
        case 1: _t->UpdateFiles(); break;
        case 2: _t->SaveEnv(); break;
        case 3: _t->SaveQryFile(); break;
        case 4: _t->SaveRoadmap(); break;
        case 5: _t->SavePath(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData FileOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FileOptions::staticMetaObject = {
    { &OptionsBase::staticMetaObject, qt_meta_stringdata_FileOptions,
      qt_meta_data_FileOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FileOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FileOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FileOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FileOptions))
        return static_cast<void*>(const_cast< FileOptions*>(this));
    return OptionsBase::qt_metacast(_clname);
}

int FileOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = OptionsBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
