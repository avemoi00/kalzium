#ifndef TIMEWIDGET_IMPL_H
#define TIMEWIDGET_IMPL_H

/***************************************************************************
    copyright            : (C) 2005 by Carsten Niehaus
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

#include <QWidget>
#include "timewidget.h"

/**
 * @author Carsten Niehaus
 * @author Pino Toscano
 */
class TimeWidgetIMPL : public QWidget, private Ui_TimeWidget
{
	Q_OBJECT

	public:
		/**
		 * @param parent The parent of this widget
		 */
		TimeWidgetIMPL( QWidget *parent = 0 );

		/**
		 * @return the currently selected date
		 */
		int date() const;

	private slots:
		/**
		 * this slot will react to the new year
		 * @param value the new year
		 */
		void slotChanged( int value );

	signals:
		/**
		 * emitted when the date changes
		 * @param value the new year
		 */
		void dateChanged( int value );
};
#endif // TIMEWIDGET_IMPL_H

