/****************************************************************************
** VizmoRoadmapNodesShapeGUI meta object code from reading C++ file 'roadmapShape.h'
**
** Created: Mon Feb 3 15:26:58 2003
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "roadmapShape.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *VizmoRoadmapNodesShapeGUI::className() const
{
    return "VizmoRoadmapNodesShapeGUI";
}

QMetaObject *VizmoRoadmapNodesShapeGUI::metaObj = 0;
static QMetaObjectCleanUp cleanUp_VizmoRoadmapNodesShapeGUI;

#ifndef QT_NO_TRANSLATION
QString VizmoRoadmapNodesShapeGUI::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoRoadmapNodesShapeGUI", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString VizmoRoadmapNodesShapeGUI::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "VizmoRoadmapNodesShapeGUI", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* VizmoRoadmapNodesShapeGUI::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QToolBar::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"VizmoRoadmapNodesShapeGUI", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_VizmoRoadmapNodesShapeGUI.setMetaObject( metaObj );
    return metaObj;
}

void* VizmoRoadmapNodesShapeGUI::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "VizmoRoadmapNodesShapeGUI" ) ) return (VizmoRoadmapNodesShapeGUI*)this;
    return QToolBar::qt_cast( clname );
}

bool VizmoRoadmapNodesShapeGUI::qt_invoke( int _id, QUObject* _o )
{
    return QToolBar::qt_invoke(_id,_o);
}

bool VizmoRoadmapNodesShapeGUI::qt_emit( int _id, QUObject* _o )
{
    return QToolBar::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool VizmoRoadmapNodesShapeGUI::qt_property( int _id, int _f, QVariant* _v)
{
    return QToolBar::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
