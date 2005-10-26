/***************************************************************************
 *   Copyright (C) 2005 by Carsten Niehaus                                 *
 *   cniehaus@kde.org                                                      *
 *   
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include "nuclideboard.h"

#include <element.h>
#include <isotope.h>

#include "kalziumdataobject.h"
#include "kalziumutils.h"

#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qsizepolicy.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstdaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>

#include <math.h>
#include <stdlib.h>

static const int MaxIsotopeSize = 60;
static const int MinIsotopeSize = 12;

static const int MaxElementDisplayed = 111;
static const int MinElementDisplayed = 1;

IsotopeTableView::IsotopeTableView( QWidget* parent, IsotopeScrollArea *scroll )
	: QWidget( parent ), m_parent( parent ), m_scroll( scroll )
{
	setBackgroundMode( Qt::NoBackground );
//	setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
	setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
//	setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
	setMinimumSize( 200, 200 );
	// resizing to a fake size
	resize( sizeHint() );
	m_pix.resize( size() );

	m_duringSelection = false;
	m_isMoving = false;

	m_rectSize = -1;

	m_firstElem = MinElementDisplayed;
	m_lastElem = MaxElementDisplayed;
	m_firstElemNucleon = minNucleonOf( KalziumDataObject::instance()->element( m_firstElem ) );
	m_lastElemNucleon = maxNucleonOf( KalziumDataObject::instance()->element( m_lastElem ) );

kdDebug() << k_funcinfo << m_firstElemNucleon << " ... " << m_lastElemNucleon << endl;

	// updating the list of isotope rects...
	updateIsoptopeRectList();
	// ... and repainting
	drawInternally();
	repaint();
}

void IsotopeTableView::paintEvent( QPaintEvent* /* e */ )
{
	QPixmap pm( size() );
	bitBlt( &pm, 0, 0, &m_pix );

	if ( m_duringSelection )
	{
		//draw the selection-rectangle
		QPainter p;
		p.begin( &pm, this );
	
		p.setPen( QPen( Qt::black, 1, Qt::DotLine ) );
		p.drawRect( m_selectedRegion );

		p.end();
	}

	bitBlt( this, 0, 0, &pm );
}

QRect IsotopeTableView::getNewCoords( const QRect& rect ) const
{
	QRect ret;

//X 	int i = 0;
//X 	int a = 0;
//X 
//X kdDebug() << "ORIG RECT: " << rect << endl
//X           << "OUR SIZE:  " << size() << endl;
//X 
//X 	i = 0;
//X 	a = height() - rect.top() - 30;
//X 	if ( a > 0 )
//X 	{
//X 		while ( i * ( m_rectSize - 1 ) < a ) i++;
//X kdDebug() << "TOP: " << i << endl;
//X 		ret.setTop( m_firstElem + i - 1 );
//X 	}
//X 	else
//X 		ret.setTop( m_firstElem + 2 );
//X 
//X 	i = 0;
//X 	while ( i * ( m_rectSize - 1 ) < rect.right() - 30 ) i++;
//X kdDebug() << "RIGHT: " << i << endl;
//X 	ret.setRight( m_firstElemNucleon + i - 1 );
//X 
//X 	i = 0;
//X 	a = height() - rect.bottom() - 30;
//X 	if ( a > 0 )
//X 	{
//X 		while ( i * ( m_rectSize - 1 ) < a ) i++;
//X kdDebug() << "BOTTOM: " << i << endl;
//X 		ret.setBottom( m_firstElem + i - 1 );
//X 	}
//X 	else
//X 		ret.setBottom( m_firstElem );
//X 
//X 	i = 0;
//X 	while ( i * ( m_rectSize - 1 ) < rect.left() - 30 ) i++;
//X kdDebug() << "LEFT: " << i << endl;
//X 	ret.setLeft( m_firstElemNucleon + i - 1 );
//X 
//X 	// normalize the rect - needed to get valid coordinates
//X 	ret = ret.normalize();
//X 
//X kdDebug() << "RECT: " << ret << endl;

	return ret;
}

void IsotopeTableView::selectionDone( const QRect& selectedRect )
{
	QRect r = getNewCoords( selectedRect );
	// I know it's strange, but somehow sometimes the bottom and the
	// top are swapped...
	m_firstElem = qMin( r.bottom(), r.top() );
	m_lastElem = qMax( r.bottom(), r.top() );
	m_firstElemNucleon = qMin( r.left(), r.right() );
	m_lastElemNucleon = qMax( r.left(), r.right() );

	// tell to the updateIsoptopeRectList() to recalc the size of every
	// element rect
	m_rectSize = -1;

	//now update the list of isotopes to be displayed
	updateIsoptopeRectList();
}

void IsotopeTableView::updateIsoptopeRectList( bool redoSize )
{
//X 	m_IsotopeAdapterRectMap.clear();
//X 	
//X 	const int numOfElements = m_lastElem - m_firstElem + 1;
//X 	const int numOfNucleons = m_lastElemNucleon - m_firstElemNucleon + 1;
//X 
//X 	kdDebug () << m_firstElem << " ... " << m_lastElem << endl;
//X 	kdDebug () << "Nucleons: " << m_firstElemNucleon << " ... " << m_lastElemNucleon << endl;
//X 
//X 	if ( m_rectSize < 1 ) // need to recalc the size
//X 	{
//X 		if ( m_scroll )
//X 		{
//X 			m_rectSize = (int)kMin( ( m_scroll->viewport()->height() - 30 ) / (double)numOfElements,
//X 			                        ( m_scroll->viewport()->width() - 30 ) / (double)numOfNucleons );
//X 		}
//X 		else
//X 		{
//X 			m_rectSize = (int)kMin( ( height() - 30 ) / (double)numOfElements,
//X 			                        ( width() - 30 ) / (double)numOfNucleons );
//X 		}
//X 		redoSize = true;
//X 	}
//X /*
//X kdDebug() << "width(): " << width() << " - height(): " << height()
//X           << " - noe: " << numOfElements << " - non: " << numOfNucleons
//X //          << " - a: " << a << " - b: " << b
//X           << " - size(): " << size()
//X           << " - m_rectSize: " << m_rectSize << endl;
//X //*/
//X 	if ( redoSize )
//X 	{
//X 		if ( m_rectSize < MinIsotopeSize )
//X 		{
//X 			m_rectSize = MinIsotopeSize;
//X 		}
//X 		else if ( m_rectSize > MaxIsotopeSize )
//X 		{
//X 			m_rectSize = MaxIsotopeSize;
//X 		}
//X 		const int newsizex = numOfNucleons * ( m_rectSize - 1 ) + 2 + 30;
//X 		const int newsizey = numOfElements * ( m_rectSize - 1 ) + 2 + 30;
//X 		resize( newsizex, newsizey );
//X 		m_pix.resize( size() );
//X //kdDebug() << "size(): " << size() << endl;
//X 	}
//X 	
//X 	QList<Isotope*> isotopeList;
//X 	QList<Isotope*>::ConstIterator isotope;
//X 	QList<Isotope*>::ConstIterator isotopeEnd;
//X 
//X 	int id = 0;
//X 
//X 	for ( int i = m_firstElem; i <= m_lastElem; i++ )
//X 	{
//X //kdDebug() << k_funcinfo << "i: " << i << endl;
//X 		Element* el = KalziumDataObject::instance()->element( i );
//X 		if ( !el ) continue;
//X 
//X //		kdDebug() << "el: " << el->elname() << " (" << el->number() <<")" << endl;
//X 		isotopeList = isotopesWithNucleonsInRange( el, m_firstElemNucleon, m_lastElemNucleon );
//X 		isotopeEnd = isotopeList.constEnd();
//X 		isotope = isotopeList.constBegin();
//X //kdDebug() << k_funcinfo << "isolist: " << isotopeList.count() << endl;
//X 		
//X 		for ( ; isotope != isotopeEnd; ++isotope )
//X 		{
//X 			Isotope* iso = *isotope;
//X 			if ( !iso ) continue;
//X 
//X 			int Xpositon = iso->nucleons() - m_firstElemNucleon;
//X 
//X 			QRect boundingRect = QRect( 
//X 					Xpositon*(m_rectSize-1)+1, 
//X 					( numOfElements - id - 1 ) * ( m_rectSize - 1 ) + 1,
//X 					m_rectSize - 1, m_rectSize - 1 );
//X //kdDebug() << k_funcinfo << boundingRect << endl;
//X 
//X  			m_IsotopeAdapterRectMap.insert(iso, boundingRect);
//X 		}
//X 		++id;
//X 	}
//X 
//X 	drawInternally();
//X 	update();
}

int IsotopeTableView::minNucleonOf( Element* el, int lowerbound ) const
{
	if ( !el ) return lowerbound;

	int minNumber = 1000;

//X 	QList<Isotope*> isotopeList = el->isotopes();
//X 	QList<Isotope*>::const_iterator isoIt = isotopeList.constBegin();
//X 	QList<Isotope*>::const_iterator isoItEnd = isotopeList.constEnd();
//X 
//X 	for ( ; isoIt != isoItEnd; ++isoIt )
//X 	{
//X 		if ( ( ( *isoIt )->nucleons() < minNumber ) &&
//X 		     ( ( *isoIt )->nucleons() >= lowerbound ) )
//X 			minNumber = ( *isoIt )->nucleons();
//X 	}
	return minNumber;
}

int IsotopeTableView::maxNucleonOf( Element* el, int upperbound ) const
{
	if ( !el ) return upperbound;

	int maxNumber = 0;

//X 	QList<Isotope*> isotopeList = el->isotopes();
//X 	QList<Isotope*>::const_iterator isoIt = isotopeList.constBegin();
//X 	QList<Isotope*>::const_iterator isoItEnd = isotopeList.constEnd();
//X 
//X 	for ( ; isoIt != isoItEnd; ++isoIt )
//X 	{
//X 		if ( ( ( *isoIt )->nucleons() > maxNumber ) &&
//X 		     ( ( *isoIt )->nucleons() <= upperbound ) )
//X 			maxNumber = ( *isoIt )->nucleons();
//X 	}
	return maxNumber;
}

QList<Isotope*> IsotopeTableView::isotopesWithNucleonsInRange( Element* el, int lowerbound, int upperbound ) const
{
	QList<Isotope*> isolist;

//X 	if ( !el ) return isolist;
//X 
//X //kdDebug() << "isotopesWithNucleonsInRange(): " << el << " - low: " << lowerbound
//X //          << " - up: " << upperbound << endl;
//X 
//X 	QList<Isotope*> isotopeList = el->isotopes();
//X 	QList<Isotope*>::ConstIterator isoIt = isotopeList.constBegin();
//X 	QList<Isotope*>::ConstIterator isoItEnd = isotopeList.constEnd();
//X 
//X 	for ( ; isoIt != isoItEnd; ++isoIt )
//X 	{
//X 		if ( ( ( *isoIt )->nucleons() >= lowerbound ) &&
//X 		     ( ( *isoIt )->nucleons() <= upperbound ) )
//X 		{
//X 			isolist.append( ( *isoIt ) );
//X 		}
//X 	}

	return isolist;
}


QPair<QColor, QColor> IsotopeTableView::isotopeColor( Isotope* isotope )
{
//X 	QPair<QColor, QColor> def = qMakePair( QColor( Qt::magenta ), QColor( Qt::magenta ) );
	QPair<QColor, QColor> c = qMakePair( QColor(), QColor() );
//X 	if ( !isotope ) return def;
//X 
//X 	if ( !isotope->betaminusdecay() && !isotope->betaplusdecay() &&
//X 	     !isotope->alphadecay() && !isotope->ecdecay() )
//X 		c = def;
//X 	else
//X 	{
//X 		if ( isotope->betaminusdecay() )
//X 			if ( c.first.isValid() )
//X 				c.second = Qt::cyan;
//X 			else
//X 				c.first = Qt::cyan;
//X 		if ( isotope->betaplusdecay() )
//X 			if ( c.first.isValid() )
//X 				c.second = Qt::red;
//X 			else
//X 				c.first = Qt::red;
//X 		if ( isotope->alphadecay() )
//X 			if ( c.first.isValid() )
//X 				c.second = Qt::yellow;
//X 			else
//X 				c.first = Qt::yellow;
//X 		if ( isotope->ecdecay() )
//X 			if ( c.first.isValid() )
//X 				c.second = Qt::green;
//X 			else
//X 				c.first = Qt::green;
//X 
//X 		if ( !c.second.isValid() )
//X 			c.second = c.first;
//X 	}
//X 
//X 	return c;
}

void IsotopeTableView::drawInternally()
{
	m_pix.fill( Qt::lightGray );

	QPainter p( &m_pix );

	drawIsotopeWidgets( &p );

	drawLegends( &p );

	p.end();
}

void IsotopeTableView::drawIsotopeWidgets( QPainter *p )
{
	p->translate( 30, 0 );

	QMap<Isotope*, QRect>::ConstIterator it = m_IsotopeAdapterRectMap.constBegin();
	const QMap<Isotope*, QRect>::ConstIterator itEnd = m_IsotopeAdapterRectMap.constEnd();

	QFont f = p->font();
	f.setPointSize( KalziumUtils::maxSize( "Mn", QRect( 0, 0, m_rectSize, m_rectSize ), f, p ) );
	p->setFont( f );

	for ( ; it != itEnd ; ++it )
	{
		Isotope* i = it.key();
	
		if ( i )
		{
			QPair<QColor, QColor> colors( isotopeColor( i ) ) ;
			if ( colors.first.name() == colors.second.name() )
			{
				p->setBrush( colors.first );
				p->drawRect( it.data() );
			}
			else
			{
				QPen oldpen = p->pen();
				p->setPen( Qt::NoPen );
				QPolygon poly( 3 );
				poly.setPoint( 1, it.data().topRight() + QPoint( 1, 0 ) );
				poly.setPoint( 2, it.data().bottomLeft() + QPoint( 0, 1 ) );

				poly.setPoint( 0, it.data().topLeft() );
				p->setBrush( colors.first );
				p->drawPolygon( poly );

				poly.setPoint( 0, it.data().bottomRight() + QPoint( 1, 1 ) );
				p->setBrush( colors.second );
				p->drawPolygon( poly );

				p->setPen( oldpen );
				QBrush oldbrush = p->brush();
				p->setBrush( Qt::NoBrush );
				p->drawRect( it.data() );
				p->setBrush( oldbrush );
			}
			
			//For debugging, lets add the information
//			p->drawText( it.data() ,Qt::AlignCenter, QString::number( it.key()->neutrons() ) );
//X 			p->drawText( it.data(), Qt::AlignCenter, KalziumDataObject::instance()->element( it.key()->protones() )->symbol() );
		}
	}
	p->setBrush( Qt::black );

	p->translate( -30, 0 );
}

void IsotopeTableView::drawLegends( QPainter *p )
{
	QFont oldfont = p->font();

	const int numElems = m_lastElem - m_firstElem + 1;
	const int numNucleons = m_lastElemNucleon - m_firstElemNucleon + 1;
	int i;
	QRect rect;

	QFont f = p->font();
	f.setPointSize( KalziumUtils::maxSize( "Mn", QRect( 0, 0, 30, m_rectSize ), f, p ) );
	p->setFont( f );

	// elements
	for ( i = 0; i < numElems; i++ )
	{
		rect = QRect( 0, ( numElems - i - 1 ) * ( m_rectSize - 1 ),
		              30, m_rectSize - 1 );
//X 		p->drawText( rect, Qt::AlignCenter,
//X 		             KalziumDataObject::instance()->element( m_firstElem + i )->symbol() );
	}

	f = p->font();
	f.setPointSize( KalziumUtils::maxSize( "222", QRect( 0, 0, m_rectSize, 30 ), f, p ) );
	p->setFont( f );

	// number of protons
	for ( i = 0; i < numNucleons; i++ )
	{
		rect = QRect( i * ( m_rectSize - 1 ) + 30, height() - 30,
		              m_rectSize - 1, 30 );
		p->drawText( rect, Qt::AlignCenter, QString::number( m_firstElemNucleon + i ) );
	}
/*
	int Xpositon = iso->nucleons() - m_firstElemNucleon;

	QRect boundingRect = QRect( 
		Xpositon*(m_rectSize-1)+1, 
		( numOfElements - id - 1 ) * ( m_rectSize - 1 ) + 1,
		m_rectSize - 1, m_rectSize - 1 );
*/
	p->setFont( oldfont );
}

void IsotopeTableView::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
	{
		if ( m_duringSelection )
		{
			m_firstPoint = e->pos();
		}
		else
		{
			m_isMoving = true;
			m_firstPoint = m_scroll->mapToParent( e->pos() );
			setCursor( KCursor::handCursor() );
		}
	}
}

void IsotopeTableView::mouseReleaseEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
	{
		if ( m_duringSelection )
		{
			QRect startPoint( m_firstPoint, m_firstPoint );
			QRect endPoint( e->pos(), e->pos() );

			// ensure to have a valid QRect
			QRect ourrect = startPoint.unite( endPoint ).normalize();

			// ensure to zoom in a valid region
			if ( ( ourrect.width() > m_rectSize ) &&
			     ( ourrect.height() > m_rectSize ) )
			{
				m_duringSelection = false;

				emit toggleZoomAction( false );

				selectionDone( ourrect );
			}
			else
				update();
		}
		else
		{
			m_isMoving = false;
			setCursor( KCursor::arrowCursor() );
		}
	}
}

void IsotopeTableView::mouseMoveEvent( QMouseEvent *e )
{
	if ( m_duringSelection )
	{
		QRect startPoint( m_firstPoint, m_firstPoint );
		QRect endPoint( e->pos(), e->pos() );

		m_selectedRegion = startPoint.unite( endPoint );
		update();
	}
	if ( m_isMoving )
	{
		QPoint now = m_scroll->mapToParent( e->pos() );
		QPoint diff = m_firstPoint - now;
//kdDebug() << "m_firstPoint: " << m_firstPoint << " - now: " << now << endl;
		m_scroll->scrollBy( diff.x(), diff.y() );
		m_firstPoint = now;
	}
}

void IsotopeTableView::slotZoomIn()
{
	m_rectSize = (int)( m_rectSize * 1.2 );
	updateIsoptopeRectList( true );
	drawInternally();
	repaint();
}

void IsotopeTableView::slotZoomOut()
{
	m_rectSize = (int)( m_rectSize * 0.8 );
	updateIsoptopeRectList( true );
	drawInternally();
	repaint();
}

void IsotopeTableView::slotToogleZoomMode( bool state )
{
	m_duringSelection = state;
}



NuclideLegend::NuclideLegend( QWidget* parent )
	: QWidget( parent )
{
	setMinimumSize( 300, 50 );
	setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
	resize( minimumSizeHint() );
}

void NuclideLegend::paintEvent( QPaintEvent* /*e*/ )
{
	QPainter p( this );
	QString text;

	p.fillRect( 10, 10, 10, 10, Qt::cyan );
	text = i18n( "%1- Decay" ).arg( QChar( 946 ) );
	p.drawText( 30, 20, text );

	p.fillRect( 10, 30, 10, 10, Qt::red );
	text =  i18n( "%1+ Decay" ).arg( QChar( 946) );
	p.drawText( 30, 40, text );

	p.fillRect( 120, 10, 10, 10, Qt::yellow );
	text =  i18n( "%1 Decay" ).arg( QChar( 945 ) );
	p.drawText( 140, 20, text );

	p.fillRect( 120, 30, 10, 10, Qt::green );
	text = i18n( "Acronym of Electron Capture Decay", "EC Decay" );
	p.drawText( 140, 40, text );

	p.fillRect( 230, 10, 10, 10, Qt::magenta );
	text =  i18n( "Stable" );
	p.drawText( 250, 20, text );
}

IsotopeScrollArea::IsotopeScrollArea( QWidget* parent )
	: QScrollArea( parent )
{
	viewport()->setPaletteBackgroundColor( parent->paletteBackgroundColor() );
	setFrameShape( QFrame::NoFrame );
	setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void IsotopeScrollArea::scrollBy( int x, int y )
{
	scrollContentsBy( x, y );
}

IsotopeTableDialog::IsotopeTableDialog( QWidget* parent )
	: KDialogBase( parent, "isotope-table", true, i18n( "Isotope Table" ), Help|Close, Close, true )
{
	QWidget *page = new QWidget( this );
	setMainWidget( page );

	QVBoxLayout *vbox = new QVBoxLayout( page , 0, -1 );

	IsotopeScrollArea *scrollarea = new IsotopeScrollArea( page );
	m_view = new IsotopeTableView( scrollarea, scrollarea );
	scrollarea->setWidget( m_view );

	NuclideLegend *legend = new NuclideLegend( page );

	m_ac = new KActionCollection( this );
	KToolBar *toolbar = new KToolBar( page, "toolbar", true, false );
	toolbar->setIconSize( 22 );
	KAction *a = KStdAction::zoomIn( m_view, SLOT( slotZoomIn() ), m_ac, "zoomin" );
	a->plug( toolbar );
	a = KStdAction::zoomOut( m_view, SLOT( slotZoomOut() ), m_ac, "zoomout" );
	a->plug( toolbar );
	KToggleAction *ta = new KToggleAction( i18n( "Select Zoom Area" ), "viewmagfit", 0, 0, 0, m_ac, "zoomselect" );
	connect( ta, SIGNAL( toggled( bool ) ), m_view, SLOT( slotToogleZoomMode( bool ) ) );
	connect( m_view, SIGNAL( toggleZoomAction( bool ) ), ta, SLOT( setChecked( bool ) ) );
	ta->plug( toolbar );

	vbox->addWidget( toolbar );
	vbox->addWidget( scrollarea );
	vbox->addWidget( legend );

	setMinimumSize( 750, 500 );
	resize( minimumSize() );

	update();
}

void IsotopeTableDialog::slotHelp()
{
	KToolInvocation::invokeHelp( QLatin1String( "isotope_table" ), QLatin1String( "kalzium" ) );
}

KActionCollection* IsotopeTableDialog::actionCollection()
{
	return m_ac;
}

#include "nuclideboard.moc"
