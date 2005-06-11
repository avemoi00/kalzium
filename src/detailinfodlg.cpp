/***************************************************************************
    begin                : Tue Apr 8 2003
    copyright            : (C) 2003, 2004, 2005 by Carsten Niehaus
    email                : cniehaus@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "detailinfodlg.h"

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kstandarddirs.h>

#include <qlabel.h>
#include <qimage.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "element.h"
#include "orbitswidget.h"
#include "detailedgraphicaloverview.h"

DetailedInfoDlg::DetailedInfoDlg( KalziumDataObject *data, Element *el , QWidget *parent, const char *name)
    : KDialogBase( IconList, "", User1|User2|Close, Close, parent, name, false/* non modal */, false, KGuiItem("Next", "1rightarrow"), KGuiItem("Previous", "1leftarrow"))
{
	m_data    = data;
	m_element = el;

	m_baseHtml = KGlobal::dirs()->findResourceDir("data", "kalzium/data/" );
	m_baseHtml.append("kalzium/data/htmlview/");
	m_baseHtml.append("style.css");

	( actionButton( KDialogBase::Close ) )->setFocus();
	
	m_pOverviewTab = addPage(i18n("Overview"), i18n("Overview"), BarIcon( "overview" ));
	m_pPictureTab = addPage(i18n("Picture"), i18n("What does this element look like?"), BarIcon( "elempic" ));
	m_pModelTab = addPage( i18n("Atom Model"), i18n( "Atom Model" ), BarIcon( "orbits" ));
	
	mainLayout = new QVBoxLayout( m_pPictureTab );
	overviewLayout = new QVBoxLayout( m_pOverviewTab );
	QVBoxLayout *modelLayout = new QVBoxLayout( m_pModelTab , 0, KDialog::spacingHint() );

	dTab = new DetailedGraphicalOverview( m_pOverviewTab, "DetailedGraphicalOverview" );
	dTab->setElement( m_element );
 	overviewLayout->addWidget( dTab );
	wOrbits = new OrbitsWidget( m_pModelTab );
	piclabel = new QLabel( m_pPictureTab );
	
	mainLayout->addWidget( piclabel );
	modelLayout->addWidget( wOrbits );
	
	createContent();

	m_currpage = 0;

	setButtonTip( User2, i18n( "Goes to the previous element" ) );
	setButtonTip( User1, i18n( "Goes to the next element" ) );

	if ( m_element->number() == 1 )
		enableButton( User2, false );
	else if ( m_element->number() == m_data->numberOfElements() )
		enableButton( User1, false );

	connect( this, SIGNAL( aboutToShowPage(QWidget *) ), SLOT( slotChangePage(QWidget *) ) );
}

// FIXME: We should never have to delete the windows.  
//        Make it more efficient!
//
void DetailedInfoDlg::setElement(Element *element)
{
	m_element = element;
	
	QValueList<QFrame*>::iterator it = m_pages.begin();
	QValueList<QFrame*>::iterator itEnd = m_pages.end();
	for ( ; it != itEnd ; ++it ){
		delete *it;
		*it = NULL;
	}
	if ( m_currpage > 2 )
		m_currpage += m_pages.size();
	m_pages.clear();

	createContent();

	enableButton( User1, true );
	enableButton( User2, true );
	if ( m_element->number() == 1 )
		enableButton( User2, false );
	else if ( m_element->number() == m_data->numberOfElements() )
		enableButton( User1, false );
}


void DetailedInfoDlg::addTab( const QString& htmlcode, const QString& title, const QString icontext, const QString iconname )
{
	QFrame *frame = addPage(title, icontext, BarIcon(iconname));
	QVBoxLayout *layout = new QVBoxLayout( frame );
	KHTMLPart *w = new KHTMLPart( frame, "html-part", frame );
	layout->addWidget( w->view() );
	
	w->begin();
	w->write( htmlcode );
	w->end();

	//add this page to the list of pages
	m_pages.append( frame );
}

QString DetailedInfoDlg::getHtml(DATATYPE type)
{
	QString html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"><html><head><title>Chemical data</title><link rel=\"stylesheet\" type=\"text/css\" href=\"";
	html += m_baseHtml + "\" /><base href=\"" + m_baseHtml + "\"/></head><body><div class=\"chemdata\">";

	//get the list of ionisation-energies
	QValueList<double> ionlist = m_element->ionisationList();
	
	html.append( "<div><table summary=\"header\"><tr><td>" );
	html.append( m_element->symbol() );
	html.append( "<td><td>" );
	html.append( i18n( "Block: %1" ).arg( m_element->block() ) );
	html.append( "</td></tr></table></div>" );
	html.append( "<table summary=\"characteristics\" class=\"characterstics\">");
	switch ( type )
	{
		case CHEMICAL:
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( "<b>" + i18n( "Orbital structure: %1" ).arg( m_element->parsedOrbits() ) + "</b>" );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"density.png\" alt=\"icon\"/></td><td>" );
			html.append( "<b>" + i18n( "Density: %1" ).arg( m_element->adjustUnits( Element::DENSITY ) ) + "</b>" );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td><b>" );
			html.append( "<b>" + i18n( "Covalent Radius: %1" ).arg( m_element->adjustRadius( Element::COVALENT ) ) + "</b>" );
			html.append( "</td></tr>" );
			if ( m_element->radius(Element::IONIC) > 0.0 )
			{
				html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td>" );
				html.append( i18n( "<b>Ionic Radius (Charge): %1</b> (%2)" ).arg( m_element->adjustRadius(Element::IONIC) ).arg( m_element->ioncharge() ) );
				html.append( "</td></tr>" );
			}
			if ( m_element->radius(Element::VDW) > 0.0 )
			{
				html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td>" );
				html.append( i18n( "<b>van der Waals Radius: %1</b>" ).arg( m_element->adjustRadius(Element::VDW) ) );
				html.append( "</td></tr>" );
			}
		
			if ( m_element->radius(Element::ATOMIC) > 0.0 )
			{
				html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td><b>" );
				html.append( i18n( "<b>Atomic Radius: %1</b>" ).arg( m_element->adjustRadius(Element::ATOMIC) ) );
				html.append( "</td></tr>" );
			}
			
			html.append( "<tr><td stype=\"text-align:center\"><img src=\"mass.png\" alt=\"icon\"/></td><td>" );
			html.append( "<b>" + i18n( "Mass: %1" ).arg( m_element->adjustUnits( Element::MASS ) ) + "</b>" );
			html.append( "</td></tr>" );
			if ( !m_element->Isotopes().isEmpty() )
			{
				html.append( "<tr><td stype=\"text-align:center\"><img src=\"mass.png\" alt=\"icon\"/></td><td>" );
				html.append( isotopeTable() );
				html.append( "</td></tr>" );
			}
			break;
		case MISC:
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( m_element->adjustUnits( Element::DATE ) );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( i18n( "Mean mass: %1 u" ).arg( QString::number( m_element->meanmass() ) ) );
			html.append( "</td></tr>" );
			if ( !m_element->nameOrigin().isEmpty() )
			{
				html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
				html.append( i18n( "Origin of the name: %1" ).arg( m_element->nameOrigin() ) );
				html.append( "</td></tr>" );
			}
			if ( m_element->artificial() )
			{
				html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
				html.append( i18n( "This element is artificial" ));
				html.append( "</td></tr>" );
			}
			if ( m_element->radioactive() )
			{
				html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
				html.append( i18n( "This element is radioactive" ));
				html.append( "</td></tr>" );
			}
			break;
		case ENERGY:
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( i18n( "Orbital structure: %1" ).arg( m_element->parsedOrbits() ) );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( i18n( "Melting Point: %1" ).arg( m_element->adjustUnits( Element::MELTINGPOINT ) ) );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( i18n( "Boiling Point: %1" ).arg( m_element->adjustUnits( Element::BOILINGPOINT ) ) );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( i18n( "Electronegativity: %1" ).arg( m_element->adjustUnits( Element::EN ) ) );
			html.append( "</td></tr>" );
			html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
			html.append( i18n( "Electron affinity: %1 " ).arg( m_element->adjustUnits(Element::EA) ) );
			html.append( "</td></tr>" );
						

			uint i = 0;
			for ( ; i < ionlist.count() ; ++i )
			{
				html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
				html.append( i18n("the first variable is a number. The result is for example '1.' or '5.', the second is the value of the ionisation energy",
				             "%1. Ionization energy: %2" ).arg( QString::number( i+1 ), m_element->adjustUnits( Element::IE, ionlist[i] ) ) );
			html.append( "</td></tr>" );
			}
			break;
	}
			
	html += ( "</table>" );
	html += "</div></body></html>";
	
	return html;
}

QString DetailedInfoDlg::isotopeTable()
{
	const QString isotopes_string = m_element->Isotopes();
	QString isotopes = isotopes_string;
	QString html;

	int pos;
	int l;

	html = "<table class=\"isotopes\" cellspacing=\"0\"><tr><td colspan=\"3\">";
	html += i18n( "Isotope-Table" );
	html += "</tr></td><tr><td><b>";
	html += i18n( "Weight" );
	html += "</b></td><td><b>";
	html += i18n( "Neutrons" );
	html += "</b></td><td><b>";
	html += i18n( "Percentage" );
	html += "</b></td></tr>";
	
	for (  int num = 0; num < isotopes_string.contains(  ";" ) ; ++num )
	{
		pos = isotopes.find(  ";" );
		l = isotopes.length();

		QString str = isotopes.left(  pos );
		QString new_str = isotopes.right(  l-pos-1 );

		//now append the html-code... 
		html.append( isotopeRow( str ) );
		isotopes = new_str;
	}
	
	html += ( "</table>" );

	return html;
}

QString DetailedInfoDlg::isotopeRow( const QString& str )
{
	QString text;

	//number of neutrons of the isotope
	int pos = str.find( ":" );
	QString neutrons_str = str.left( pos );
	QString tmp = str.right( str.length()-pos-1 );

	//Weight of the isotope
	pos = tmp.find( ":" );
	QString weight_str = tmp.left( pos );

	//Percentage of the isotope
	tmp = tmp.right( tmp.length()-pos-1 );

 	QString neutrons;
 	QString weight;
 	QString percentage;

	weight.append( weight_str );
	neutrons.append( neutrons_str );

	percentage.append( tmp );

	text = "<tr><td align=\"right\">";
	text.append( i18n( "%1 u" ).arg( weight ) );
	text.append( "</td><td>" );
	text.append( neutrons );
	text.append( "</td><td>" );
	text.append( i18n( "this can for example be '24%'", "%1%" ).arg( percentage ) );
	text.append( "</td></tr>" );

	return text;
}


void DetailedInfoDlg::createContent( )
{
	addTab( getHtml(CHEMICAL), i18n( "Chemical Data" ), i18n( "Chemical Data" ), "chemical" );
	addTab( getHtml(ENERGY), i18n( "Energies" ), i18n( "Energy Information" ), "energies" );
	addTab( getHtml(MISC), i18n( "Miscellaneous" ), i18n( "Miscellaneous" ), "misc" );
	
	QString num = QString::number( m_element->number() );
	QString cap = i18n("For example Carbon (6)" , "%1 (%2)" ).arg( m_element->elname() ).arg( num );
	setCaption( cap );

	dTab->setElement( m_element );

	////////////////////////////////////7
	if ( !locate(  "data" , "kalzium/elempics/" + m_element->symbol() + ".jpg" ).isEmpty() )
	{
		QPixmap pic ( locate( "data" , "kalzium/elempics/" + m_element->symbol() + ".jpg" ) );
		QImage img = pic.convertToImage();
		img = img.smoothScale ( 400, 400, QImage::ScaleMin );
		pic.convertFromImage( img );
		piclabel->setPixmap( pic );
	}
	else 
	{
		piclabel->setText( i18n( "No picture of %1 found." ).arg( m_element->elname() ) );
	}

    /////////////////////////////////
	
	wOrbits->setElementNumber( m_element->number() );
	wOrbits->repaint();
	QWhatsThis::add( wOrbits,  i18n( "Here you can see the atomic hull of %1. %2 has the configuration %3." )
							.arg( m_element->elname() )
							.arg( m_element->elname() )
							.arg( m_element->parsedOrbits() ) );
}

void DetailedInfoDlg::wheelEvent( QWheelEvent *ev )
{
	if ( ev->delta() < 0 )
		// setting the previous element
		slotUser2();
	else
		// setting the next element
		slotUser1();
}

void DetailedInfoDlg::slotUser1()
{
// setting the next element
	int number = m_element->number();

	if ( number < m_data->numberOfElements() )
	{
		disconnect( this, SIGNAL( aboutToShowPage(QWidget *) ), this, SLOT( slotChangePage(QWidget *) ) );
		setElement( m_data->element( number + 1 ) );
		emit elementChanged( number + 1 );
		if ( number == ( m_data->numberOfElements() - 1 ) )
			enableButton( User1, false );
		if ( !actionButton( User2 )->isEnabled() )
			enableButton( User2, true );
		showPage( m_currpage );
		connect( this, SIGNAL( aboutToShowPage(QWidget *) ), SLOT( slotChangePage(QWidget *) ) );
	}
}

void DetailedInfoDlg::slotUser2()
{
// setting the previous element
	int number = m_element->number();

	if ( number > 1 )
	{
		disconnect( this, SIGNAL( aboutToShowPage(QWidget *) ), this, SLOT( slotChangePage(QWidget *) ) );
		setElement( m_data->element( number - 1 ) );
		emit elementChanged( number - 1 );
		if ( number == 2 )
			enableButton( User2, false );
		if ( !actionButton( User1 )->isEnabled() )
			enableButton( User1, true );
		showPage( m_currpage );
		connect( this, SIGNAL( aboutToShowPage(QWidget *) ), SLOT( slotChangePage(QWidget *) ) );
	}
}

void DetailedInfoDlg::slotChangePage( QWidget *newpage )
{
	m_currpage = pageIndex( newpage );
}

#include "detailinfodlg.moc"
