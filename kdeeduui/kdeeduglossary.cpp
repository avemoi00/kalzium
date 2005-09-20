/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdeeduglossary.h"

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kglobal.h>
#include <klistview.h>
#include <klistviewsearchline.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>

#include <qevent.h>
#include <qfile.h>
#include <qlabel.h>
#include <q3header.h>
#include <qlayout.h>
#include <qlist.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qstringlist.h>
#include <qtoolbutton.h>

Glossary::Glossary( const KURL& url, const QString& path )
{
	init( url, path );
}

Glossary::Glossary()
{
	init( KURL(), QString() );
}

Glossary::~Glossary()
{
}

void Glossary::init( const KURL& url, const QString& path )
{
	// setting a generic name for a new glossary
	m_name = i18n( "Glossary" );

	setPicturePath( path );

	if ( !url.isEmpty() )
	{

		QDomDocument doc( "document" );

		if ( loadLayout( doc, url ) )
		{
			setItemlist( readItems( doc ) );
			if ( !m_picturepath.isEmpty() )
				fixImagePath();
		}
	}
}

bool Glossary::loadLayout( QDomDocument &Document, const KURL& url )
{
	QFile layoutFile( url.path() );

	if ( !layoutFile.exists() )
	{
		kdDebug() << "no such file: " << layoutFile.name() << endl;
		return false;
	}

	if ( !layoutFile.open( QIODevice::ReadOnly ) )
		return false;

	// check if document is well-formed
	if ( !Document.setContent( &layoutFile ) )
	{
		kdDebug() << "wrong xml of " << layoutFile.name() << endl;
		layoutFile.close();
		return false;
	}
	layoutFile.close();

	return true;
}

bool Glossary::isEmpty() const
{
	return m_itemlist.count() == 0;
}

void Glossary::setName( const QString& name )
{
	if ( name.isEmpty())
		return;
	m_name = name;
}

void Glossary::setPicturePath( const QString& path )
{
	if ( path.isEmpty())
		return;
	m_picturepath = path;
}

void Glossary::setBackgroundPicture( const QString& filename )
{
	if ( filename.isEmpty())
		return;
	m_backgroundpicture = filename;
}

void Glossary::fixImagePath()
{
	kdDebug() << "Glossary::fixImagePath()" << endl;
	QList<GlossaryItem*>::iterator it = m_itemlist.begin();
	const QList<GlossaryItem*>::iterator itEnd = m_itemlist.end();
	QString imgtag = "<img src=\"" + m_picturepath + "/" + "\\1\" />";
	QRegExp exp( "\\[img\\]([^[]+)\\[/img\\]" );

	for ( ; it != itEnd ; ++it )
	{
		QString tmp = ( *it )->desc();
		while ( exp.search( tmp ) > -1 )
		{
			tmp = tmp.replace( exp, imgtag );
		}
		( *it )->setDesc( tmp );
	}
}

QList<GlossaryItem*> Glossary::readItems( QDomDocument &itemDocument )
{
	QList<GlossaryItem*> list;

	QDomNodeList itemList;
	QDomNodeList refNodeList;
	QDomElement itemElement;
	QStringList reflist;

	itemList = itemDocument.elementsByTagName( "item" );

	const uint num = itemList.count();
	for ( uint i = 0; i < num; ++i )
	{
		reflist.clear();
		GlossaryItem *item = new GlossaryItem();
		
		itemElement = ( const QDomElement& ) itemList.item( i ).toElement();
		
		QDomNode nameNode = itemElement.namedItem( "name" );
		QDomNode descNode = itemElement.namedItem( "desc" );
		
		QString picName = itemElement.namedItem( "picture" ).toElement().text();
		QDomElement refNode = ( const QDomElement& ) itemElement.namedItem( "references" ).toElement();

		QString desc = descNode.toElement().text().utf8();
		if ( !picName.isEmpty() )
			desc.prepend("[img]"+picName +"[/img][brclear][br]" );

		item->setName( i18n( nameNode.toElement( ).text().utf8() ) );
		
		desc = desc.replace("[b]", "<b>" );
		desc = desc.replace("[/b]", "</b>" );
		desc = desc.replace("[i]", "<i>" );
		desc = desc.replace("[/i]", "</i>" );
		desc = desc.replace("[sub]", "<sub>" );
		desc = desc.replace("[/sub]", "</sub>" );
		desc = desc.replace("[sup]", "<sup>" );
		desc = desc.replace("[/sup]", "</sup>" );
		desc = desc.replace("[br]", "<br />" );
		desc = desc.replace("[brclear]", "<br clear=\"left\"/>" );
		item->setDesc( desc );

		refNodeList = refNode.elementsByTagName( "refitem" );
		for ( int it = 0; it < refNodeList.count(); it++ )
		{
			reflist << i18n( refNodeList.item( it ).toElement().text().utf8() );
		}
		item->setRef( reflist );
		
		list.append( item );
	}
	
	return list;
}

GlossaryDialog::GlossaryDialog( bool folded, QWidget *parent )
    : KDialogBase( Plain, i18n( "Glossary" ), Close, NoDefault, parent, "glossary-dialog", false )
{
	//this string will be used for all items. If a backgroundpicture should
	//be used call Glossary::setBackgroundPicture().
	m_htmlbasestring = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"><html><body%1>" ;

	m_folded = folded;
	
	QVBoxLayout *vbox = new QVBoxLayout( plainPage(), 0, KDialog::spacingHint() );
	vbox->activate();

	QHBoxLayout *hbox = new QHBoxLayout( 0L, 0, KDialog::spacingHint() );
	hbox->activate();

	QToolButton *clear = new QToolButton( plainPage() );
	clear->setIconSet( SmallIconSet( "locationbar_erase" ) );
	clear->setToolTip( i18n( "Clear filter" ) );
	hbox->addWidget( clear );

	QLabel *lbl = new QLabel( plainPage() );
	lbl->setText( i18n( "Search:" ) );
	hbox->addWidget( lbl );

	m_search = new KListViewSearchLine( plainPage(), 0, "search-line" );
	hbox->addWidget( m_search );
	vbox->addLayout( hbox );
	setFocusProxy(m_search);
 
	QSplitter *vs = new QSplitter( plainPage() );
	vbox->addWidget( vs );

	m_glosstree = new KListView( vs, "treeview" );
	m_glosstree->addColumn( "entries" );
	m_glosstree->header()->hide();
	m_glosstree->setFullWidth( true );
	m_glosstree->setRootIsDecorated( true );
 
	m_search->setListView( m_glosstree );
 
	m_htmlpart = new KHTMLPart( vs, "html-part" );

	connect( m_htmlpart->browserExtension(), SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, SLOT( displayItem( const KURL &, const KParts::URLArgs & ) ) );
	connect( m_glosstree, SIGNAL(clicked( Q3ListViewItem * )), this, SLOT(slotClicked( Q3ListViewItem * )));
	connect( clear, SIGNAL(clicked()), m_search, SLOT(clear()));

	resize( 600, 400 );
}

GlossaryDialog::~GlossaryDialog()
{
}

void GlossaryDialog::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Return) {
		e->ignore();
	}
	KDialogBase::keyPressEvent(e);
}

void GlossaryDialog::displayItem( const KURL& url, const KParts::URLArgs& )
{
	// using the "host" part of a kurl as reference
	QString myurl = url.host().lower();
	m_search->setText( "" );
	m_search->updateSearch( "" );
	Q3ListViewItem *found = 0;
	Q3ListViewItemIterator it( m_glosstree );
	Q3ListViewItem *item;
	while ( it.current() )
	{
		item = it.current();
		if ( item->text(0).lower() == myurl )
		{
			found = item;
			break;
		}
		++it;
	}
	slotClicked( found );
}

void GlossaryDialog::updateTree()
{
	m_glosstree->clear();

	QList<Glossary*>::const_iterator itGl = m_glossaries.begin();
	const QList<Glossary*>::const_iterator itGlEnd = m_glossaries.end();
	
	for ( ; itGl != itGlEnd ; ++itGl )
	{
		QList<GlossaryItem*> items = ( *itGl )->itemlist();
		QList<GlossaryItem*>::iterator it = items.begin();
		const QList<GlossaryItem*>::iterator itEnd = items.end();

		Q3ListViewItem *main = new Q3ListViewItem( m_glosstree, ( *itGl )->name() );
		main->setExpandable( true );
		main->setSelectable( false );

		for ( ; it != itEnd ; ++it )
		{
			if ( m_folded )
			{
				QChar thisletter = ( *it )->name().upper()[0];
				Q3ListViewItem *thisletteritem = findTreeWithLetter( thisletter, main );
				if ( !thisletteritem )
				{
					thisletteritem = new Q3ListViewItem( main, QString(thisletter) );
					thisletteritem->setExpandable( true );
					thisletteritem->setSelectable( false );
				}
				new Q3ListViewItem( thisletteritem, ( *it )->name() );
			}
			else
				new Q3ListViewItem( main, ( *it )->name() );
		}
		main->sort();
	}
}

void GlossaryDialog::addGlossary( Glossary* newgloss )
{
	if ( !newgloss ) return;
	if ( newgloss->isEmpty() ) return;
	m_glossaries.append( newgloss );

	kdDebug() << "Count of the new glossary: " << newgloss->itemlist().count() << endl;
	kdDebug() << "Number of glossaries: " << m_glossaries.count() << endl;

	updateTree();
}

Q3ListViewItem* GlossaryDialog::findTreeWithLetter( const QChar& l, Q3ListViewItem* i )
{
	Q3ListViewItem *it = i->firstChild();
	while ( it )
	{
		if ( it->text(0)[0] == l )
			return it;
		it = it->nextSibling();
	}
	return 0;
}

void GlossaryDialog::slotClicked( Q3ListViewItem *item )
{
	if ( !item )
		return;
	
	// The next lines are searching for the correct KnowledgeItem
	// in the m_itemList. When it is found the HTML will be
	// generated
	QList<Glossary*>::iterator itGl = m_glossaries.begin();
	const QList<Glossary*>::iterator itGlEnd = m_glossaries.end();
	bool found = false;
	GlossaryItem *i = 0;

	QString bg_picture;
	
	while ( !found && itGl != itGlEnd )
	{
		QList<GlossaryItem*> items = ( *itGl )->itemlist();
		QList<GlossaryItem*>::const_iterator it = items.begin();
		const QList<GlossaryItem*>::const_iterator itEnd = items.end();
		while ( !found && it != itEnd )
		{
			if ( ( *it )->name() == item->text( 0 ) )
			{
				i = *it;
				bg_picture = ( *itGl )->backgroundPicture();
				found = true;
			}
			++it;
		}
		++itGl;
	}
	if ( found && i )
	{
		QString html;
		if ( !bg_picture.isEmpty() )
		{
			html = " background=\"" + bg_picture + "\"";
		}

		html = m_htmlbasestring.arg( html );
		html += i->toHtml() + "</body></html>";

		m_htmlpart->begin();
		m_htmlpart->write( html );
		m_htmlpart->end();
		return;
	}
}

void GlossaryItem::setRef( const QStringList& s )
{
	m_ref = s;
	m_ref.sort();
}

QString GlossaryItem::toHtml() const
{
	QString code = "<h1>" + m_name + "</h1>" + m_desc + parseReferences();

	return code;
}

QString GlossaryItem::parseReferences() const
{
	if ( m_ref.isEmpty() )
		return QString();

	QString htmlcode = "<h3>" + i18n( "References" ) + "</h3><ul type=\"disc\">";
	
	for ( int i = 0; i < m_ref.size(); i++ )
	{
		htmlcode += QString( "<li><a href=\"item://%1\">%2</a></li>" ).arg( m_ref[i], m_ref[i] );
	}
	htmlcode += "</ul>";

	return htmlcode;
}


#include "kdeeduglossary.moc"
