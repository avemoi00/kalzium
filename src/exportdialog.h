#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H
/***************************************************************************
    copyright            : (C) 2007 by Johannes Simon
    email                : johannes.simon@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QListWidget>
#include <QFile>
#include <element.h>

#include "kalziumdataobject.h"
#include "ui_exportdialog.h"

class KDialog;

class ElementListEntry : public QListWidgetItem
{
    public:
        ElementListEntry( Element * element );
        ~ElementListEntry();

        int       m_atomicNum;
        QString   m_name;
        Element * m_element;
};

class PropertyListEntry : public QListWidgetItem
{
    public:
        PropertyListEntry( const QString & name, ChemicalDataObject::BlueObelisk type );
        ~PropertyListEntry();

        ChemicalDataObject::BlueObelisk m_type;
};

/**
* @author: Johannes Simon
*/
class ExportDialog : public KDialog
{
    Q_OBJECT

    public:
        ExportDialog( QWidget * parent );
        ~ExportDialog();

        void populateElementList();
        void exportToHtml();
        void exportToXml();
        void exportToCsv();

    private:
        Ui::exportDialogForm ui;
        QTextStream * m_outputStream;
        

    public slots:
        void slotOkClicked();
        void slotHelpClicked();
};

#endif // EXPORTDIALOG_H