/****************************************************************************
** Meta object code from reading C++ file 'AnimationWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../AnimationWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AnimationWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AnimationWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      32,   16,   16,   16, 0x08,
      46,   16,   16,   16, 0x08,
      56,   16,   16,   16, 0x08,
      71,   16,   16,   16, 0x08,
      87,   16,   16,   16, 0x08,
      99,   16,   16,   16, 0x08,
     111,   16,   16,   16, 0x08,
     122,   16,   16,   16, 0x08,
     139,   16,   16,   16, 0x08,
     156,   16,   16,   16, 0x08,
     170,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AnimationWidget[] = {
    "AnimationWidget\0\0GoToFrame(int)\0"
    "BackAnimate()\0Animate()\0PauseAnimate()\0"
    "PreviousFrame()\0NextFrame()\0GoToFirst()\0"
    "GoToLast()\0SliderMoved(int)\0"
    "UpdateStepSize()\0UpdateFrame()\0Timeout()\0"
};

void AnimationWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AnimationWidget *_t = static_cast<AnimationWidget *>(_o);
        switch (_id) {
        case 0: _t->GoToFrame((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->BackAnimate(); break;
        case 2: _t->Animate(); break;
        case 3: _t->PauseAnimate(); break;
        case 4: _t->PreviousFrame(); break;
        case 5: _t->NextFrame(); break;
        case 6: _t->GoToFirst(); break;
        case 7: _t->GoToLast(); break;
        case 8: _t->SliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->UpdateStepSize(); break;
        case 10: _t->UpdateFrame(); break;
        case 11: _t->Timeout(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AnimationWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AnimationWidget::staticMetaObject = {
    { &QToolBar::staticMetaObject, qt_meta_stringdata_AnimationWidget,
      qt_meta_data_AnimationWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AnimationWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AnimationWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AnimationWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AnimationWidget))
        return static_cast<void*>(const_cast< AnimationWidget*>(this));
    return QToolBar::qt_metacast(_clname);
}

int AnimationWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
