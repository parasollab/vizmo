/****************************************************************************
** VizmoScreenShotGUI meta object code from reading C++ file 'snapshot_gui.h'
**
** Created: Mon Feb 3 15:25:48 2003
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "snapshot_gui.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *VizmoScreenShotGUI::className() const
{
    return "VizmoScreenShotGUI";
}

QMetaObject *VizmoScreenShotGUI::metaObj = 0;
static QMetaObjectCleanUp cleanUp_VizmoScreenShotGUI;

#ifndef QT_NO_TRANSLATION
QString VizmoScreenShotGUI::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoScreenShotGUI", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString VizmoScreenShotGUI::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoScreenShotGUI", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* VizmoScreenShotGUI::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QToolBar::staticMetaObject();
    static const QUMethod slot_0 = {"takeSnapshot", 0, 0 };
    static const QUMethod slot_1 = {"takeBoxSnapshot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "takeSnapshot()", &slot_0, QMetaData::Private },
	{ "takeBoxSnapshot()", &slot_1, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ "w", &static_QUType_ptr, "int", QUParameter::In },
	{ "h", &static_QUType_ptr, "int", QUParameter::In }
    };
    static const QUMethod signal_0 = {"getScreenSize", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ "xOffet", &static_QUType_ptr, "int", QUParameter::In },
	{ "yOffset", &static_QUType_ptr, "int", QUParameter::In },
	{ "w", &static_QUType_ptr, "int", QUParameter::In },
	{ "h", &static_QUType_ptr, "int", QUParameter::In }
    };
    static const QUMethod signal_1 = {"getBoxDimensions", 4, param_signal_1 };
    static const QUMethod signal_2 = {"togleSelectionSignal", 0, 0 };
    static const QUMethod signal_3 = {"simulateMouseUp", 0, 0 };
    static const QUMethod signal_4 = {"callUpdate", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "getScreenSize(int*,int*)", &signal_0, QMetaData::Public },
	{ "getBoxDimensions(int*,int*,int*,int*)", &signal_1, QMetaData::Public },
	{ "togleSelectionSignal()", &signal_2, QMetaData::Public },
	{ "simulateMouseUp()", &signal_3, QMetaData::Public },
	{ "callUpdate()", &signal_4, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"VizmoScreenShotGUI", parentObject,
	slot_tbl, 2,
	signal_tbl, 5,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_VizmoScreenShotGUI.setMetaObject( metaObj );
    return metaObj;
}

void* VizmoScreenShotGUI::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "VizmoScreenShotGUI" ) ) return (VizmoScreenShotGUI*)this;
    return QToolBar::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL getScreenSize
void VizmoScreenShotGUI::getScreenSize( int* t0, int* t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,t0);
    static_QUType_ptr.set(o+2,t1);
    activate_signal( clist, o );
}

// SIGNAL getBoxDimensions
void VizmoScreenShotGUI::getBoxDimensions( int* t0, int* t1, int* t2, int* t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_ptr.set(o+1,t0);
    static_QUType_ptr.set(o+2,t1);
    static_QUType_ptr.set(o+3,t2);
    static_QUType_ptr.set(o+4,t3);
    activate_signal( clist, o );
}

// SIGNAL togleSelectionSignal
void VizmoScreenShotGUI::togleSelectionSignal()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

// SIGNAL simulateMouseUp
void VizmoScreenShotGUI::simulateMouseUp()
{
    activate_signal( staticMetaObject()->signalOffset() + 3 );
}

// SIGNAL callUpdate
void VizmoScreenShotGUI::callUpdate()
{
    activate_signal( staticMetaObject()->signalOffset() + 4 );
}

bool VizmoScreenShotGUI::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: takeSnapshot(); break;
    case 1: takeBoxSnapshot(); break;
    default:
	return QToolBar::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool VizmoScreenShotGUI::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: getScreenSize((int*)static_QUType_ptr.get(_o+1),(int*)static_QUType_ptr.get(_o+2)); break;
    case 1: getBoxDimensions((int*)static_QUType_ptr.get(_o+1),(int*)static_QUType_ptr.get(_o+2),(int*)static_QUType_ptr.get(_o+3),(int*)static_QUType_ptr.get(_o+4)); break;
    case 2: togleSelectionSignal(); break;
    case 3: simulateMouseUp(); break;
    case 4: callUpdate(); break;
    default:
	return QToolBar::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool VizmoScreenShotGUI::qt_property( int _id, int _f, QVariant* _v)
{
    return QToolBar::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
