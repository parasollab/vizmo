/****************************************************************************
** vizmoEditor meta object code from reading C++ file 'vizmoEditor.h'
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "vizmoEditor.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.1.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *vizmoEditor::className() const
{
    return "vizmoEditor";
}

QMetaObject *vizmoEditor::metaObj = 0;
static QMetaObjectCleanUp cleanUp_vizmoEditor( "vizmoEditor", &vizmoEditor::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString vizmoEditor::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vizmoEditor", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString vizmoEditor::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vizmoEditor", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* vizmoEditor::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"newDoc", 0, 0 };
    static const QUMethod slot_1 = {"choose", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ "fileName", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"load", 1, param_slot_2 };
    static const QUMethod slot_3 = {"save", 0, 0 };
    static const QUMethod slot_4 = {"saveAs", 0, 0 };
    static const QUMethod slot_5 = {"print", 0, 0 };
    static const QUMethod slot_6 = {"execFile", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "newDoc()", &slot_0, QMetaData::Private },
	{ "choose()", &slot_1, QMetaData::Private },
	{ "load(const QString&)", &slot_2, QMetaData::Private },
	{ "save()", &slot_3, QMetaData::Private },
	{ "saveAs()", &slot_4, QMetaData::Private },
	{ "print()", &slot_5, QMetaData::Private },
	{ "execFile()", &slot_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"vizmoEditor", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_vizmoEditor.setMetaObject( metaObj );
    return metaObj;
}

void* vizmoEditor::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "vizmoEditor" ) )
	return this;
    return QMainWindow::qt_cast( clname );
}

bool vizmoEditor::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: newDoc(); break;
    case 1: choose(); break;
    case 2: load((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: save(); break;
    case 4: saveAs(); break;
    case 5: print(); break;
    case 6: execFile(); break;
    default:
	return QMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool vizmoEditor::qt_emit( int _id, QUObject* _o )
{
    return QMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool vizmoEditor::qt_property( int id, int f, QVariant* v)
{
    return QMainWindow::qt_property( id, f, v);
}

bool vizmoEditor::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
