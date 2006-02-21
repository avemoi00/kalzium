#ifndef ISOTOPE_H
#define ISOTOPE_H
/***************************************************************************
 *   Copyright (C) 2005 by Carsten Niehaus                                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

class ChemicalDataObject;

#include <QString>

/**
 * @author Carsten Niehaus
 *
 * This class represents an Isotope with all its properties
 */
class Isotope
{
	public:
		/**
		 * Constructs a new empty isotope.
		 */
		Isotope();
		virtual ~Isotope();

		ChemicalDataObject* data() const;

		double mass() const;

		QString errorMargin() const;

		int parentElementNumber() const;

		QString parentElementSymbol() const;

		QString spin() const;

		QString magmoment() const;

		double halflife() const;
		
		/**
		 * add the ChemicalDataObject @p o
		 */
		void addData( ChemicalDataObject* o );

		void setNucleons( int number );
		
		/**
		 * @return the sum of protons and neutrons
		 */
		int nucleons() const;

		/**
		 * @return decay
		 */
		ChemicalDataObject* ecdecay() const{
			return m_ecdecay;
		}

		/**
		 * @return decay
		 */
		ChemicalDataObject* betaminusdecay() const{
			return m_betaminus;
		}
		
		/**
		 * @return decay
		 */
		ChemicalDataObject* betaplusdecay() const{
			return m_betaplus;
		}
		
		/**
		 * @return decay
		 */
		ChemicalDataObject* alphadecay() const{
			return m_alpha;
		}
	private:
		/**
		 * the symbol of the element the isotope belongs to
		 */
		ChemicalDataObject* m_parentElementSymbol;
		
		/**
		 * stores the infomation about the mass of the Isotope
		 */
		ChemicalDataObject* m_mass;

		/**
		 * stores the atomicNumber of the Isotope
		 */
		ChemicalDataObject* m_identifier;
		
		/**
		 * stores the spin of the Isotope
		 */
		ChemicalDataObject* m_spin;
		
		/**
		 * stores the magneticMoment of the Isotope
		 */
		ChemicalDataObject* m_magmoment;
		
		/**
		 * stores the halfLife of the Isotope
		 */
		ChemicalDataObject* m_halflife;
		
		int m_nucleons;

		ChemicalDataObject* m_ecdecay;
		ChemicalDataObject* m_betaplus;
		ChemicalDataObject* m_betaminus; 
		ChemicalDataObject* m_alpha;
};

#endif // ISOTOPE_H
