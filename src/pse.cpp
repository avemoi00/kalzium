/***************************************************************************
 *   Copyright (C) 2003 by Carsten Niehaus                                 *
 *   cniehaus@kde.org                                                      *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "pse.h"
#include "prefs.h"
#include "infodialog_small_impl.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <qtooltip.h>
#include <qstring.h>
#include <qlayout.h>

PSE::PSE(KalziumDataObject *data, QWidget *parent, const char *name)
 : QWidget(parent, name)
{
	d = data;

	setupBlockLists();
	setupPSEElementButtonsList();
	
	activateColorScheme( Prefs::colorschemebox() );
}

PSE::~PSE(){}

void PSE::slotUpdatePSE()
{
	kdDebug() << "slotUpdatePSE()" << endl;
	activateColorScheme( Prefs::colorschemebox() );
}

void PSE::setupBlockLists()
{
	EList::Iterator it = d->ElementList.begin();

	while ( it != d->ElementList.end() )
	{
		/*
		 * ElementButton( int number, Element *el, QWidget *parent)
		 */
		ElementButton *b = new ElementButton( (*it)->number() , *it, this );
		QToolTip::add(b, i18n("Name: %1").arg((*it)->elname().utf8()));
		b->sym = (*it)->symbol();

		if ( (*it)->block() == "s" )
			sBlockList.append( b );
		if ( (*it)->block() == "p" )
			pBlockList.append( b );
		if ( (*it)->block() == "d" )
			dBlockList.append( b );
		if ( (*it)->block() == "f" )
			fBlockList.append( b );
		++it;
	}
}

void PSE::setupPSEElementButtonsList(){}

void PSE::activateColorScheme( const int nr )
{
	kdDebug() << nr << endl;

	if ( nr == 1) //normal view, no colors
	{
		ElementButton *button;
		QColor color = Prefs::noscheme();
		for ( button = m_PSEElementButtons.first() ; button; button = m_PSEElementButtons.next() )
		{
			button->setPaletteBackgroundColor( color );
		}
	}
	else if ( nr == 2) //groups view
	{
		ElementButton *button;
		const QColor color_1 = Prefs::group_1();
		const QColor color_2 = Prefs::group_2();
		const QColor color_3 = Prefs::group_3();
		const QColor color_4 = Prefs::group_4();
		const QColor color_5 = Prefs::group_5();
		const QColor color_6 = Prefs::group_6();
		const QColor color_7 = Prefs::group_7();
		const QColor color_8 = Prefs::group_8();

		static QString group;
		
		for ( button = m_PSEElementButtons.first() ; button; button = m_PSEElementButtons.next() )
		{
			group = button->e->group();

			if (group == QString("1")) {
				button->setPaletteBackgroundColor( color_1 );
				continue;
			}
			if (group == QString("2")){
				button->setPaletteBackgroundColor( color_2 );
				continue;
			}
			if (group == QString("3")){
				button->setPaletteBackgroundColor( color_3 );
				continue;
			}
			if (group == QString("4")){
				button->setPaletteBackgroundColor( color_4 );
				continue;
			}
			if (group == QString("5")) {
				button->setPaletteBackgroundColor( color_5 );
				continue;
			}
			if (group == QString("6")){
				button->setPaletteBackgroundColor( color_6 );
				continue;
			}
			if (group == QString("7")){
				button->setPaletteBackgroundColor( color_7 );
				continue;
			}
			if (group == QString("8")){
				button->setPaletteBackgroundColor( color_8 );
				continue;
			}
		}
	}
	else if ( nr == 3) //block view
	{
		ElementButton *button;
		const QColor color_s = Prefs::block_s();
		const QColor color_p = Prefs::block_p();
		const QColor color_d = Prefs::block_d();
		const QColor color_f = Prefs::block_f();

		static QString block;
		
		for ( button = m_PSEElementButtons.first() ; button; button = m_PSEElementButtons.next() )
		{
			block = button->e->block();

			if (block == QString("s")) {
				button->setPaletteBackgroundColor( color_s );
				continue;
			}
			if (block == QString("p")){
				button->setPaletteBackgroundColor( color_p );
				continue;
			}
			if (block == QString("d")){
				button->setPaletteBackgroundColor( color_d );
				continue;
			}
			if (block == QString("f")){
				button->setPaletteBackgroundColor( color_f );
				continue;
			}
		}
	}
	
	else if ( nr == 5) //acidic beh
	{
		ElementButton *button;
		const QColor color_ba = Prefs::beh_basic();
		const QColor color_ac = Prefs::beh_acidic();
		const QColor color_neu = Prefs::beh_neutral();
		const QColor color_amp = Prefs::beh_amphoteric();

		static QString acidicbeh;
		
		for ( button = m_PSEElementButtons.first() ; button; button = m_PSEElementButtons.next() )
		{
			acidicbeh = button->e->acidicbeh();

			if (acidicbeh == QString("0")) {
				button->setPaletteBackgroundColor( color_ac );
				continue;
			}
			if (acidicbeh == QString("1")){
				button->setPaletteBackgroundColor( color_ba );
				continue;
			}
			if (acidicbeh == QString("2")){
				button->setPaletteBackgroundColor( color_neu );
				continue;
			}
			if (acidicbeh == QString("3")){
				button->setPaletteBackgroundColor( color_amp );
				continue;
			}
		}
	}
	else if ( nr == 4) //state-of-matter
	{
		ElementButton *button;
		const QColor color_solid = Prefs::color_solid();
		const QColor color_liquid = Prefs::color_liquid();
		const QColor color_radioactive = Prefs::color_radioactive();
		const QColor color_vapor = Prefs::color_vapor();
		const QColor color_artificial = Prefs::color_artificial();

		static QString az;
		
		for ( button = m_PSEElementButtons.first() ; button; button = m_PSEElementButtons.next() )
		{
			az = button->e->az();

			if (az == QString("0")) {
				button->setPaletteBackgroundColor( color_solid );
				continue;
			}
			if (az == QString("1")){
				button->setPaletteBackgroundColor( color_liquid );
				continue;
			}
			if (az == QString("2")){
				button->setPaletteBackgroundColor( color_radioactive );
				continue;
			}
			if (az == QString("3")){
				button->setPaletteBackgroundColor( color_vapor );
				continue;
			}
			if (az == QString("4")){
				button->setPaletteBackgroundColor( color_artificial );
				continue;
			}
		}
	}
}


RegularPSE::RegularPSE(KalziumDataObject *data, QWidget *parent, const char *name)
 : PSE(data, parent, name)
{
	QVBoxLayout *vbox = new QVBoxLayout( this , 0 , -1 , "vbox" );
	
	QGridLayout *grid = new QGridLayout( 18 , 10 );
	
	ElementButton *button;
	for ( button = sBlockList.first() ; button ; button = sBlockList.next() )
	{
		grid->addWidget( button , button->e->y-1 , button->e->x-1 );
	}
	for ( button = pBlockList.first() ; button ; button = pBlockList.next() )
	{
		grid->addWidget( button , button->e->y-1 , button->e->x-1 );
	}
	for ( button = dBlockList.first() ; button ; button = dBlockList.next() )
	{
		grid->addWidget( button , button->e->y-1 , button->e->x-1 );
	}
	for ( button = fBlockList.first() ; button ; button = fBlockList.next() )
	{
		grid->addWidget( button , button->e->y-1 , button->e->x-1 );
	}

	vbox->addLayout( grid );

	for (  int n=0; n<18; n++ ) grid->addColSpacing(  n, 40 );
	for (  int n=0; n<10; n++ ) grid->addRowSpacing(  n, 40 );
	
	setupPSEElementButtonsList();
}

RegularPSE::~RegularPSE(){} 

/**
 * this method sets up the m_PSEElementButtons-list
 **/
void RegularPSE::setupPSEElementButtonsList()
{
	ElementButton *button;
	
	for ( button = sBlockList.first() ; button ; button = sBlockList.next() )
	{
		m_PSEElementButtons.append( button );
	}
	for ( button = pBlockList.first() ; button ; button = pBlockList.next() )
	{
		m_PSEElementButtons.append( button );
	}
	for ( button = dBlockList.first() ; button ; button = dBlockList.next() )
	{
		m_PSEElementButtons.append( button );
	}
	for ( button = fBlockList.first() ; button ; button = fBlockList.next() )
	{
		m_PSEElementButtons.append( button );
	}
}

SimplifiedPSE::SimplifiedPSE(KalziumDataObject *data, QWidget *parent, const char *name)
 : PSE(data, parent, name)
{
	d = data;
	QVBoxLayout *vbox = new QVBoxLayout( this , 0 , -1 , "vbox" );
	
	QGridLayout *grid = new QGridLayout( 18 , 17  );
	
	ElementButton *button;

	for ( button = sBlockList.first() ; button ; button = sBlockList.next() )
	{
		grid->addWidget( button , button->e->s_y-1 , button->e->s_x-1 );
	}
	for ( button = pBlockList.first() ; button ; button = pBlockList.next() )
	{
		grid->addWidget( button , button->e->s_y-1 , button->e->s_x-1 );
	}
	for ( button = dBlockList.first() ; button ; button = dBlockList.next() )
	{
		button->hide();
	}
	for ( button = fBlockList.first() ; button ; button = fBlockList.next() )
	{
		button->hide();
	}
	vbox->addLayout( grid );

	for (  int n=0; n<8; n++ ) grid->addColSpacing(  n, 40 );
	for (  int n=0; n<7; n++ ) grid->addRowSpacing(  n, 40 );
	
	setupPSEElementButtonsList();
}

SimplifiedPSE::~SimplifiedPSE(){}

/**
 * this method sets up the m_PSEElementButtons-list
 **/
void SimplifiedPSE::setupPSEElementButtonsList()
{
	kdDebug() << "SimplifiedPSE::setupPSEElementButtonsList()" << endl;
	ElementButton *button;
	
	for ( button = sBlockList.first() ; button ; button = sBlockList.next() )
	{
		m_PSEElementButtons.append( button );
	}
	for ( button = pBlockList.first() ; button ; button = pBlockList.next() )
	{
		m_PSEElementButtons.append( button );
	}
	kdDebug() << "Anzahl der Buttons nach dem Erstellen: " << m_PSEElementButtons.count() << endl;
	
}

MendeljevPSE::MendeljevPSE(KalziumDataObject *data, QWidget *parent, const char *name)
 : PSE(data, parent, name)
{
	d = data;
	QVBoxLayout *vbox = new QVBoxLayout( this , 0 , -1 , "vbox" );
	
	QGridLayout *grid = new QGridLayout( 18 , 17  );
	
	ElementButton *button;

	for ( button = sBlockList.first() ; button ; button = sBlockList.next() )
	{
		grid->addWidget( button , button->e->s_y-1 , button->e->s_x-1 );
	}
	for ( button = pBlockList.first() ; button ; button = pBlockList.next() )
	{
		grid->addWidget( button , button->e->s_y-1 , button->e->s_x-1 );
	}
	for ( button = dBlockList.first() ; button ; button = dBlockList.next() )
	{
		button->hide();
	}
	for ( button = fBlockList.first() ; button ; button = fBlockList.next() )
	{
		button->hide();
	}
	vbox->addLayout( grid );

	for (  int n=0; n<8; n++ ) grid->addColSpacing(  n, 40 );
	for (  int n=0; n<7; n++ ) grid->addRowSpacing(  n, 40 );

	setupPSEElementButtonsList();
}

MendeljevPSE::~MendeljevPSE(){}

void MendeljevPSE::setupPSEElementButtonsList()
{
	kdDebug() << "MendeljevPSE::setupPSEElementButtonsList()" << endl;
}

#include "pse.moc"
