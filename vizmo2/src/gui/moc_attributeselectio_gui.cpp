/****************************************************************************
** VizmoAttributeSelectionGUI meta object code from reading C++ file 'attributeselectio_gui.h'
**
** Created: Sat Nov 30 22:08:33 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "attributeselectio_gui.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *VizmoAttributeSelectionGUI::className() const
{
    return "VizmoAttributeSelectionGUI";
}

QMetaObject *VizmoAttributeSelectionGUI::metaObj = 0;
static QMetaObjectCleanUp cleanUp_VizmoAttributeSelectionGUI;

#ifndef QT_NO_TRANSLATION
QString VizmoAttributeSelectionGUI::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoAttributeSelectionGUI", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString VizmoAttributeSelectionGUI::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoAttributeSelectionGUI", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* VizmoAttributeSelectionGUI::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QToolBar::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"VizmoAttributeSelectionGUI", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_VizmoAttributeSelectionGUI.setMetaObject( metaObj );
    return metaObj;
}

void* VizmoAttributeSelectionGUI::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "VizmoAttributeSelectionGUI" ) ) return (VizmoAttributeSelectionGUI*)this;
    return QToolBar::qt_cast( clname );
}

bool VizmoAttributeSelectionGUI::qt_invoke( int _id, QUObject* _o )
{
    return QToolBar::qt_invoke(_id,_o);
}

bool VizmoAttributeSelectionGUI::qt_emit( int _id, QUObject* _o )
{
    return QToolBar::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool VizmoAttributeSelectionGUI::qt_property( int _id, int _f, QVariant* _v)
{
    return QToolBar::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
