// -*- coding: iso-8859-1 -*-
/*
 *   Author: etienne <etienne@none.org>
 *   Date: So. Jun 19 2011, 15:54:43
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *f
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

QGraphicsWidget {
  id: mainWidget

  signal itemClicked(variant elementItem)

  Item{
    id: mainItem


    Repeater{
      model: 120

      ElementItem {
	id: elementItem

	elementNumber: index + 1

	MouseArea {
	    id: itemMouse
	    anchors.fill: parent
	    onClicked: {
	        console.log("Clicked " + elementItem)
		mainWidget.itemClicked(elementItem)
	    }
        }
      }
    }
  }

}

