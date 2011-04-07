/***************************************************************************
    copyright            : (C) 2003, 2004, 2005, 2006, 2007 by Carsten Niehaus
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
#include "kalzium.h"

#include <kapplication.h>
#include <element.h>
#include <kdeeduglossary.h>
#include <kactioncollection.h>
#include "prefs.h"
#include "ui_settings_colors.h"
#include "ui_settings_gradients.h"
#include "ui_settings_units.h"
#include "ui_settings_calc.h"
#include "elementdataviewer.h"
#include "detailinfodlg.h"
#include "molcalcwidget.h"
#include "detailedgraphicaloverview.h"
#include "gradientwidget_impl.h"
#include "kalziumdataobject.h"
#include "isotopetabledialog.h"
#include "kalziumnumerationtype.h"
#include "kalziumschemetype.h"
#include "kalziumgradienttype.h"
#include "rsdialog.h"
#include "tablesdialog.h"
#include "legendwidget.h"
#include "exportdialog.h"
#include "search.h"
#include "searchwidget.h"
#include "tableinfowidget.h"
#include "psetables.h"
#include <config-kalzium.h>

#include "calculator/calculator.h"

#ifdef HAVE_FACILE
#include "eqchemview.h"
#endif

#ifdef HAVE_OPENBABEL2
#if defined(HAVE_EIGEN) && defined(HAVE_AVOGADRO)
#include "tools/moleculeview.h"
#endif
#include "tools/obconverter.h"
#endif

#include <QDockWidget>
#include <QToolBox>
#include <QKeyEvent>
#include <QRegExp>

#include <kmessagebox.h>
#include <kconfigdialog.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kselectaction.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <kicon.h>
#include <kservicetypetrader.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <KLocale>
#include <KPluginLoader>
#include <KTabWidget>

#define IDS_ELEMENTINFO     7

Kalzium::Kalzium() : KXmlGuiWindow( 0 )
{
    setObjectName( "KalziumMainWindow" );

    // adding the libkdeedu catalog
    KGlobal::locale()->insertCatalog( "libkdeedu" );

    // reading the elements from file
    KalziumDataObject::instance();

    Search *newsearch = new Search();
    KalziumDataObject::instance()->setSearch( newsearch );

    m_infoDialog = 0;
    m_toolboxCurrent = 0;
    m_exportDialog = 0;

    // Main pse-Table Tablewidget
    QWidget *pseTempWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout( pseTempWidget );
    layout->setMargin( 0 );
    layout->setSpacing( 2 );

    SearchWidget *searchWidget = new SearchWidget( pseTempWidget );
    searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));

    // Creating the periodic table
    m_periodicTable = new PeriodicTableView( pseTempWidget );

    // Connecting the search to the periodic table
    connect( newsearch, SIGNAL( searchChanged() ),
             KalziumElementProperty::instance(), SIGNAL( propertyChanged() ) );
    connect( newsearch, SIGNAL( searchReset() ),
             KalziumElementProperty::instance(), SIGNAL( propertyChanged() ) );

    layout->addWidget( searchWidget );
    layout->addWidget( m_periodicTable );

    setCentralWidget(pseTempWidget);

    connect(m_periodicTable->pseScene(), SIGNAL(elementChanged(int)), this, SLOT(openInformationDialog(int)));
    connect(m_periodicTable->pseScene(), SIGNAL(elementHovered(int)), this, SLOT(elementHover(int)));
    connect(this, SIGNAL(numerationChanged(int)),  m_periodicTable, SIGNAL(numerationChange(int)));

    // layouting
    setupSidebars();
    setupActions();

    // creating the glossary dialog and loading the glossaries we have
    m_glossarydlg = new GlossaryDialog( this );
    m_glossarydlg->setObjectName( QLatin1String( "glossary" ) );
    QString dir = KGlobal::dirs()->findResourceDir( "data", "kalzium/data/" );
    QString picturepath = dir + "kalzium/data/bg.jpg";
    KUrl u = KUrl::fromPath( dir + "kalzium/data/knowledge.xml" );
    Glossary *g = new Glossary( u );
    g->setName( i18n( "Knowledge" ) );
    g->setBackgroundPicture( picturepath );
    m_glossarydlg->addGlossary( g, true );
    u = KUrl::fromPath( dir + "kalzium/data/tools.xml" );
    g = new Glossary( u, dir + "kalzium/data/toolpics/" );
    g->setName( i18n( "Tools" ) );
    g->setBackgroundPicture( picturepath );
    m_glossarydlg->addGlossary( g, true );

    setupStatusBar();
}

void Kalzium::setupActions()
{
    export_action = actionCollection()->add<QAction>( "file_exporter" );
    export_action->setText( i18n("&Export Data...") );
    connect( export_action, SIGNAL( triggered( bool ) ), this, SLOT( slotShowExportDialog() ) );

    // the action for swiching look: color schemes and gradients
    QStringList schemes = KalziumElementProperty::instance()->schemeList();    /*KalziumSchemeTypeFactory::instance()->schemes();*/
    QStringList gradients = KalziumElementProperty::instance()->gradientList();

    // the action for swiching look: schemes
    look_action_schemes = actionCollection()->add<KSelectAction>( "view_look_onlyschemes" );
    look_action_schemes->setText( i18n( "&Scheme" ) );
    look_action_schemes->setItems( schemes );
    look_action_schemes->setToolBarMode( KSelectAction::MenuMode );
    look_action_schemes->setToolButtonPopupMode( QToolButton::InstantPopup );
    connect( look_action_schemes, SIGNAL( triggered( int ) ), this, SLOT( slotSwitchtoLookScheme( int ) ) );

    // the action for swiching look: gradients
    look_action_gradients = actionCollection()->add<KSelectAction>( "view_look_onlygradients" );
    look_action_gradients->setText( i18n( "&Gradients" ) );
    look_action_gradients->setItems( gradients );
    look_action_gradients->setToolBarMode( KSelectAction::MenuMode );
    look_action_gradients->setToolButtonPopupMode( QToolButton::InstantPopup );
    connect( look_action_gradients, SIGNAL( triggered( int ) ), this, SLOT( slotSwitchtoLookGradient( int ) ) );

    // the action for swiching tables
    QStringList table_schemes = pseTables::instance()->tables();
    table_action =  actionCollection()->add<KSelectAction>( "view_table" );
    table_action->setText( i18n( "&Tables" ) );
    table_action->setItems(table_schemes);
    table_action->setCurrentItem(Prefs::table());
    connect( table_action, SIGNAL( triggered( int ) ), this, SLOT( slotSwitchtoTable( int ) ) );

    // the actions for switching numeration
    numeration_action = actionCollection()->add<KSelectAction>( "view_numerationtype" );
    numeration_action->setText( i18n( "&Numeration" ) );
    numeration_action->setItems( KalziumNumerationTypeFactory::instance()->numerations() );
    numeration_action->setCurrentItem(Prefs::numeration());
    connect( numeration_action, SIGNAL( triggered( int ) ), this, SLOT( slotSwitchtoNumeration( int ) ) );

    m_EQSolverAction =  actionCollection()->addAction( "tools_eqsolver" );
    m_EQSolverAction->setText( i18n( "&Equation Solver..." ) );
    m_EQSolverAction->setIcon(  KIcon( "eqchem" ) );
    m_EQSolverAction->setWhatsThis( i18nc( "WhatsThis Help", "This tool allows you to solve chemical equations." ) );

#ifdef HAVE_FACILE
    connect( m_EQSolverAction, SIGNAL( triggered() ), this, SLOT( slotShowEQSolver() ) );
    m_EQSolverAction->setEnabled( true );
#else
    m_EQSolverAction->setEnabled( false );
#endif

    // tools actions
    m_pPlotAction = actionCollection()->addAction( "tools_plotdata" );
    m_pPlotAction->setText( i18n( "&Plot Data..." ) );
    m_pPlotAction->setIcon( KIcon( "plot" ) );
    connect( m_pPlotAction, SIGNAL( triggered() ), this, SLOT( slotPlotData() ) );

    // calculator actions
    m_pcalculator = actionCollection()->addAction( "tools_calculate" );
    m_pcalculator->setText( i18n( "Perform &Calculations..." ) );
    m_pcalculator->setIcon( KIcon( "calculate" ) );
    m_pcalculator->setWhatsThis( i18nc( "WhatsThis Help", "This is the calculator, it performs basic chemical calculations." ) );
    connect( m_pcalculator, SIGNAL( triggered() ), this, SLOT( showCalculator() ) );

    m_pIsotopeTableAction= actionCollection()->addAction( "tools_isotopetable" );
    m_pIsotopeTableAction->setText( i18n( "&Isotope Table..." ) );
    m_pIsotopeTableAction->setIcon(  KIcon( "isotopemap" ) );
    m_pIsotopeTableAction->setWhatsThis( i18nc( "WhatsThis Help", "This table shows all of the known isotopes of the chemical elements." ) );
    connect( m_pIsotopeTableAction, SIGNAL( triggered() ), this, SLOT( slotIsotopeTable() ) );

    m_pGlossaryAction = actionCollection()->addAction( "tools_glossary" );
    m_pGlossaryAction->setText(i18n( "&Glossary..." ) );
    m_pGlossaryAction->setIcon( KIcon( "glossary" ) );
    connect( m_pGlossaryAction, SIGNAL( triggered() ), this, SLOT( slotGlossary() ) );

    m_pRSAction = actionCollection()->addAction( "tools_rs" );
    m_pRSAction->setText( i18n( "&R/S Phrases..." ) );
    m_pRSAction->setIcon( KIcon( "kalzium_rs" ) );
    connect( m_pRSAction, SIGNAL( triggered() ), this, SLOT( slotRS() ) );

    m_pOBConverterAction = actionCollection()->addAction( "tools_obconverter" );
    m_pOBConverterAction->setText( i18n( "Convert chemical files..." ) );
    m_pOBConverterAction->setIcon( KIcon( "edit-copy" ) );
    m_pOBConverterAction->setWhatsThis( i18nc( "WhatsThis Help", "With this tool, you can convert files containing chemical data between various file formats." ) );
    connect( m_pOBConverterAction, SIGNAL( triggered() ), this, SLOT( slotOBConverter() ) );
#ifndef HAVE_OPENBABEL2
    m_pOBConverterAction->setEnabled( false );
#endif

    m_pMoleculesviewer = actionCollection()->addAction( "tools_moleculeviewer" );
    m_pMoleculesviewer->setText( i18n( "Molecular Editor..." ) );
    m_pMoleculesviewer->setIcon( KIcon( "kalzium_molviewer" ) );
    m_pMoleculesviewer->setWhatsThis( i18nc( "WhatsThis Help", "This tool allows you to view and edit 3D molecular structures." ) );
    connect( m_pMoleculesviewer, SIGNAL( triggered() ), this, SLOT( slotMoleculeviewer() ) );
#if !defined(HAVE_OPENBABEL2) || !defined(HAVE_EIGEN) || !defined(HAVE_AVOGADRO)
    m_pMoleculesviewer->setEnabled( false );
#endif

    m_pTables = actionCollection()->addAction( "tools_tables" );
    m_pTables->setText( i18n( "&Tables..." ) );
    m_pTables->setIcon( KIcon( "kalzium_tables" ) );
    m_pTables->setWhatsThis(i18nc("WhatsThis Help", "This will open a dialog with listings of symbols and numbers related to chemistry."));

    connect( m_pTables, SIGNAL( triggered() ), this, SLOT( slotTables() ) );

    // other period view options
    m_pLegendAction = m_legendDock->toggleViewAction();
    actionCollection()->addAction( "view_legend", m_pLegendAction );
    m_pLegendAction->setIcon( KIcon( "legend" ) );
    m_pLegendAction->setWhatsThis( i18nc( "WhatsThis Help", "This will show or hide the legend for the periodic table." ) );

    m_SidebarAction = m_dockWin->toggleViewAction();
    actionCollection()->addAction( "view_sidebar", m_SidebarAction );
    m_SidebarAction->setIcon( KIcon( "sidebar" ) );
    m_SidebarAction->setWhatsThis( i18nc( "WhatsThis Help", "This will show or hide a sidebar with additional information and a set of tools." ) );

    m_SidebarAction = m_tableDock->toggleViewAction();
    actionCollection()->addAction( "view_tablebar", m_SidebarAction );
    m_SidebarAction->setIcon( KIcon( "kalzium_tables" ) );
    m_SidebarAction->setWhatsThis( i18nc( "WhatsThis Help", "This will show or hide a sidebar with additional information about the table." ) );

    // the standard actions
    actionCollection()->addAction( "saveAs", KStandardAction::saveAs(this, SLOT(slotExportTable()), actionCollection()));

    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());

    actionCollection()->addAction( "quit", KStandardAction::quit( kapp, SLOT (closeAllWindows()),actionCollection() ));

    m_legendWidget->LockWidget();


    slotSwitchtoLookGradient( KalziumElementProperty::instance()->gradientId() );
    slotSwitchtoLookScheme( KalziumElementProperty::instance()->schemeId() );

    slotSwitchtoNumeration( Prefs::numeration() );
    slotSwitchtoTable( Prefs::table() );

    m_legendWidget->UnLockWidget();
    m_legendWidget->updateContent();

    // set the shell's ui resource file
    setXMLFile("kalziumui.rc");
    setupGUI();
}

void Kalzium::setupSidebars()
{
    setDockNestingEnabled(true);

    m_legendWidget = new LegendWidget( this );
    m_legendDock = new QDockWidget( i18n("Legend"), this );
    m_legendDock->setObjectName( QLatin1String( "kalzium-legend" ) );
    m_legendDock->setFeatures( QDockWidget::AllDockWidgetFeatures );
    m_legendDock->setWidget(m_legendWidget);

    connect (m_legendDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
             m_legendWidget, SLOT(setDockArea(Qt::DockWidgetArea)));
    connect (m_legendWidget, SIGNAL(elementMatched(int)),
             m_periodicTable, SLOT(slotSelectAdditionalElement(int)));
    connect (m_legendWidget, SIGNAL(resetElementMatch()),
             m_periodicTable, SLOT(slotUnSelectElements()));

    m_TableInfoWidget = new TableInfoWidget( this );
    m_tableDock = new QDockWidget( i18n("Table Information"), this );
    m_tableDock->setWidget(m_TableInfoWidget);
    m_tableDock->setObjectName( QLatin1String( "kalzium-tableinfo" ) );
    m_tableDock->setFeatures( QDockWidget::AllDockWidgetFeatures );


    m_dockWin = new QDockWidget(i18n("Information"), this );
    m_dockWin->setObjectName( QLatin1String( "kalzium-sidebar" ) );
    m_dockWin->setFeatures( QDockWidget::AllDockWidgetFeatures );
    m_dockWin->setMinimumWidth(250);

    m_toolbox = new QToolBox( m_dockWin );
    m_dockWin->setWidget( m_toolbox );

    m_detailWidget = new DetailedGraphicalOverview( m_toolbox );
    m_detailWidget->setObjectName( "DetailedGraphicalOverview" );
    m_detailWidget->setMinimumSize( 200, m_detailWidget->minimumSize().height() );

    m_toolbox->addItem( m_detailWidget, KIcon( "overview" ), i18n( "Overview" ) );

    m_gradientWidget = new GradientWidgetImpl( m_toolbox );
    m_gradientWidget->setObjectName( "viewtWidget" );

    connect( m_gradientWidget, SIGNAL( gradientValueChanged( double ) ),
             KalziumElementProperty::instance(), SLOT( setSliderValue( double ) ) );
    connect( m_gradientWidget->scheme_combo, SIGNAL( currentIndexChanged(int)),
             this, SLOT( slotSwitchtoLookScheme(int)));
    connect( m_gradientWidget->gradient_combo, SIGNAL( currentIndexChanged(int)),
             this, SLOT( slotSwitchtoLookGradient(int)));

    m_toolbox->addItem( m_gradientWidget, KIcon( "statematter" ), i18n( "View" ) );

    m_calcWidget = new MolcalcWidget( m_toolbox );
    m_calcWidget->setObjectName( "molcalcwidget" );
    m_calcWidget->hideExtra();
    m_toolbox->addItem( m_calcWidget, KIcon( "calculate" ), i18n( "Calculate" ) );

    addDockWidget( Qt::LeftDockWidgetArea, m_dockWin );
    addDockWidget( Qt::BottomDockWidgetArea, m_tableDock, Qt::Horizontal);
    addDockWidget( Qt::BottomDockWidgetArea, m_legendDock, Qt::Horizontal );

    m_tableDock->setVisible( false );
}

void Kalzium::slotExportTable()
{
    QString fileName = KFileDialog::getSaveFileName(QString(), i18n("*.png *.xpm *.jpg *.svg"), //
                       this,
                       i18n("Save Kalzium's Table In"));

    if (fileName.endsWith(".svg")) {
        m_periodicTable->generateSvg( fileName );
    } else {
        QPixmap pix = QPixmap::grabWidget( m_periodicTable );
        pix.save( fileName );
    }
}

void Kalzium::slotGlossary()
{
    m_glossarydlg->show();
}

void Kalzium::slotRS()
{
    RSDialog *rs = new RSDialog( this );
    rs->setAttribute(Qt::WA_DeleteOnClose);
    rs->show();
}

void Kalzium::slotOBConverter()
{
#ifdef HAVE_OPENBABEL2
    KOpenBabel * d = new KOpenBabel(this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->show();
#endif
}

void Kalzium::slotMoleculeviewer()
{
#if defined(HAVE_OPENBABEL2) && defined(HAVE_EIGEN) && defined(HAVE_AVOGADRO)

    if (!QGLFormat::hasOpenGL()) {
        KMessageBox::error(0, i18n("This system does not support OpenGL."), i18n("Kalzium Error"));
        return;
    }

    MoleculeDialog * d = new MoleculeDialog( this );
    d->show();

#if 0
    KPluginLoader loader("libkalziumglpart" );
    KPluginFactory* factory = loader.factory();

    if (factory) {
        KParts::ReadOnlyPart *part = 0;
        part = static_cast<KParts::ReadOnlyPart*> ( factory->create( this, "KalziumGLPart" ) );

        part->widget()->show();
    }
#endif
#endif
}

void Kalzium::slotTables()
{
    TablesDialog *t = new TablesDialog( this );
    t->setAttribute(Qt::WA_DeleteOnClose);
    t->show();
}

void Kalzium::slotShowEQSolver()
{
#ifdef HAVE_FACILE
    EQChemDialog *dlg = new EQChemDialog( this );
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
#endif
}

void Kalzium::slotIsotopeTable()
{
    IsotopeTableDialog *ndialog = new IsotopeTableDialog( this );
    ndialog->setAttribute(Qt::WA_DeleteOnClose);
    ndialog->show();
}

void Kalzium::slotPlotData()
{
    ElementDataViewer *edw = new ElementDataViewer( this );
    edw->setAttribute(Qt::WA_DeleteOnClose);
    edw->show();
}

void Kalzium::showCalculator()
{
    calculator *Cal = new calculator ( this );
    Cal->setAttribute(Qt::WA_DeleteOnClose);
    Cal -> show();
}

void Kalzium::slotSwitchtoTable( int index )
{
    m_periodicTable->slotChangeTable(index);
    m_TableInfoWidget->setTableType( index );

    if ( m_infoDialog ) {
        m_infoDialog->setTableType( m_periodicTable->table() );
    }
    Prefs::setTable(index);
    Prefs::self()->writeConfig();
}

void Kalzium::slotSwitchtoNumeration( int index )
{
    emit numerationChanged(index);
    Prefs::setNumeration(index);
    Prefs::self()->writeConfig();
}

void Kalzium::slotSwitchtoLookGradient( int which )
{
    kDebug() << "slotSwitchtoLookGradient Kalzium";

    KalziumElementProperty::instance()->setGradient(which);

    look_action_gradients->blockSignals( true );
    m_gradientWidget->gradient_combo->blockSignals( true );

    look_action_gradients->setCurrentItem( which );
    m_gradientWidget->gradient_combo->setCurrentIndex( which );

    look_action_gradients->blockSignals( false );
    m_gradientWidget->gradient_combo->blockSignals( false );

    m_gradientWidget->slotGradientChanged();

    m_legendWidget->updateContent();
}

void Kalzium::slotSwitchtoLookScheme( int which )
{
    kDebug() << "slotSwitchtoLookScheme Kalzium";

    KalziumElementProperty::instance()->setScheme( which );

    m_gradientWidget->scheme_combo->blockSignals( true );
    look_action_schemes->blockSignals( true );

    look_action_schemes->setCurrentItem( which );
    m_gradientWidget->scheme_combo->setCurrentIndex( which );

    look_action_schemes->blockSignals( false );
    m_gradientWidget->scheme_combo->blockSignals( false );

    m_legendWidget->updateContent();
}

void Kalzium::showSettingsDialog()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    //KConfigDialog didn't find an instance of this dialog, so lets create it :
    KConfigDialog *dialog = new KConfigDialog(this,"settings", Prefs::self());
    connect( dialog, SIGNAL( settingsChanged( const QString &) ), m_gradientWidget, SLOT( slotGradientChanged()) );
    connect( dialog, SIGNAL( settingsChanged( const QString &) ), m_legendWidget, SLOT( updateContent() ) );

    // colors page
    Ui_setupColors ui_colors;
    QWidget *w_colors = new QWidget( 0 );
    w_colors->setObjectName( "colors_page" );
    ui_colors.setupUi( w_colors );
    dialog->addPage( w_colors, i18n( "Schemes" ), "preferences-desktop-color" );

    // gradients page
    Ui_setupGradients ui_gradients;
    QWidget *w_gradients = new QWidget( 0 );
    w_gradients->setObjectName( "gradients_page" );
    ui_gradients.setupUi( w_gradients );
    dialog->addPage( w_gradients, i18n( "Gradients" ), "preferences-desktop-color" );

    // units page
    Ui_setupUnits ui_units;
    QWidget *w_units = new QWidget( 0 );
    w_units->setObjectName( "units_page" );
    ui_units.setupUi( w_units );
    dialog->addPage( w_units, i18n( "Units" ), "system-run" );

    Ui_setupCalc ui_calc;
    QWidget *w_calc = new QWidget( 0 );
    ui_calc.setupUi( w_calc );
    dialog->addPage( w_calc, i18n("Calculator"), "accessories-calculator");

    // showing the dialog
    dialog->show();
}

// void Kalzium::slotUpdateSettings()
// {
// }

void Kalzium::slotShowExportDialog()
{
    if (!m_exportDialog) {
        m_exportDialog = new ExportDialog( this );
    }
    m_exportDialog->show();
}

void Kalzium::setupStatusBar()
{
    statusBar()->insertItem( "", 0, 0 );
    statusBar()->setItemAlignment( 0, Qt::AlignRight );

    statusBar()->insertItem(  "" , IDS_ELEMENTINFO, 1 );
    statusBar()->setItemAlignment( IDS_ELEMENTINFO, Qt::AlignRight );
    statusBar()->show();
}

void Kalzium::elementHover( int num )
{
    //     extractIconicInformationAboutElement( num );

    Element *e = KalziumDataObject::instance()->element( num );
    statusBar()->changeItem( i18nc( "For example: \"Carbon (6), Mass: 12.0107 u\"", "%1 (%2), Mass: %3 u" ,
                                    e->dataAsString( ChemicalDataObject::name ) ,
                                    e->dataAsString( ChemicalDataObject::atomicNumber ) ,
                                    e->dataAsString( ChemicalDataObject::mass ) ) , IDS_ELEMENTINFO );

    m_detailWidget->setElement( num );
}

// FIXME What is that function for? Does not seem to do anything usefull... yet?
void Kalzium::extractIconicInformationAboutElement( int elementNumber )
{
    QString setname = "school";
    QString pathname = KGlobal::dirs()->findResourceDir( "appdata", "data/iconsets/" ) + "data/iconsets/";
    QString filename = pathname + setname + '/' + "iconinformation.txt";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString infoline;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString tmp = in.readLine();
        if ( tmp.startsWith( QString::number( elementNumber ) ) )
            infoline = tmp;
    }

    QString realText = "Moin dies ist ein test!";
//X         QString realText = infoline.remove( QRegExp("\\d+ ") );
}

void Kalzium::openInformationDialog( int number )
{
    if (m_infoDialog) {
        m_infoDialog->setElement( number );
    } else {
        m_infoDialog = new DetailedInfoDlg(number, this );

        // Remove the selection when this dialog finishes or hides.
        connect(m_infoDialog, SIGNAL(hidden()),
                m_periodicTable, SLOT(slotUnSelectElements()));
        connect(m_infoDialog, SIGNAL(elementChanged(int)),
                m_periodicTable, SLOT(slotSelectOneElement(int)));
        connect(m_infoDialog, SIGNAL(elementChanged(int)),
                this, SLOT(elementHover(int)));
    }

    m_infoDialog->setTableType( m_periodicTable->table() );
    m_infoDialog->show();
}


Kalzium::~Kalzium()
{
    delete m_periodicTable;
    delete m_infoDialog;
    delete m_TableInfoWidget;
    delete m_legendWidget;
    delete m_gradientWidget;

    delete m_dockWin;
    delete m_legendDock;
    delete m_tableDock;
}

QSize Kalzium::sizeHint() const
{
    return QSize(700, 500);
}


#include "kalzium.moc"
