/****************************************************************************
** VizmoItemSelectionGUI meta object code from reading C++ file 'itemselection_gui.h'
**
** Created: Mon Dec 2 15:17:21 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "itemselection_gui.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *VizmoItemSelectionGUI::className() const
{
    return "VizmoItemSelectionGUI";
}

QMetaObject *VizmoItemSelectionGUI::metaObj = 0;
static QMetaObjectCleanUp cleanUp_VizmoItemSelectionGUI;

#ifndef QT_NO_TRANSLATION
QString VizmoItemSelectionGUI::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoItemSelectionGUI", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString VizmoItemSelectionGUI::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoItemSelectionGUI", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* VizmoItemSelectionGUI::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QToolBar::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"VizmoItemSelectionGUI", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_VizmoItemSelectionGUI.setMetaObject( metaObj );
    return metaObj;
}

void* VizmoItemSelectionGUI::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "VizmoItemSelectionGUI" ) ) return (VizmoItemSelectionGUI*)this;
    return QToolBar::qt_cast( clname );
}

bool VizmoItemSelectionGUI::qt_invoke( int _id, QUObject* _o )
{
    return QToolBar::qt_invoke(_id,_o);
}

bool VizmoItemSelectionGUI::qt_emit( int _id, QUObject* _o )
{
    return QToolBar::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool VizmoItemSelectionGUI::qt_property( int _id, int _f, QVariant* _v)
{
    return QToolBar::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
