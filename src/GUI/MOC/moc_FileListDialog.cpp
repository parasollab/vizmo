/****************************************************************************
** Meta object code from reading C++ file 'FileListDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../FileListDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FileListDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FileListDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x08,
      28,   15,   15,   15, 0x08,
      40,   15,   15,   15, 0x08,
      54,   15,   15,   15, 0x08,
      67,   15,   15,   15, 0x08,
      81,   15,   15,   15, 0x08,
      93,   15,   15,   15, 0x08,
     102,   15,   15,   15, 0x08,
     116,   15,   15,   15, 0x08,
     153,  138,  131,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FileListDialog[] = {
    "FileListDialog\0\0ChangeEnv()\0ChangeMap()\0"
    "ChangeQuery()\0ChangePath()\0ChangeDebug()\0"
    "ChangeXML()\0Accept()\0PathChecked()\0"
    "DebugChecked()\0string\0_filename,_key\0"
    "SearchXML(string,string)\0"
};

void FileListDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FileListDialog *_t = static_cast<FileListDialog *>(_o);
        switch (_id) {
        case 0: _t->ChangeEnv(); break;
        case 1: _t->ChangeMap(); break;
        case 2: _t->ChangeQuery(); break;
        case 3: _t->ChangePath(); break;
        case 4: _t->ChangeDebug(); break;
        case 5: _t->ChangeXML(); break;
        case 6: _t->Accept(); break;
        case 7: _t->PathChecked(); break;
        case 8: _t->DebugChecked(); break;
        case 9: { string _r = _t->SearchXML((*reinterpret_cast< string(*)>(_a[1])),(*reinterpret_cast< string(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< string*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FileListDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FileListDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_FileListDialog,
      qt_meta_data_FileListDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FileListDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FileListDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FileListDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FileListDialog))
        return static_cast<void*>(const_cast< FileListDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int FileListDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
