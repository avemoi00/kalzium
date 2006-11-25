#include <kparts/genericfactory.h>

#include <QStringList>

#include "kalziumglpart.h"

typedef KParts::GenericFactory<KalziumGLPart> KalziumGLPartFactory;

K_EXPORT_COMPONENT_FACTORY (libkalziumglpart, KalziumGLPartFactory)

KalziumGLPart::KalziumGLPart(QWidget* parentWidget, QObject* parent, const QStringList& args)
{
    kDebug() << "KalziumGLPart::KalziumGLPart()" << endl;
}

KalziumGLPart::~KalziumGLPart()
{
    kDebug() << "KalziumGLPart::~KalziumGLPart()" << endl;
}

KAboutData *KalziumGLPart::createAboutData()
{
    KAboutData* aboutData = new KAboutData( "kalzium", I18N_NOOP("KalziumGLPart"),
            "1.1.1", I18N_NOOP("A cool thing"),
            KAboutData::License_GPL,
            "(c) 2006, Carsten Niehaus", 0, "http://edu.kde.org/kalzium/index.php",
            "kalzium@kde.org");
    aboutData->addAuthor("Carsten Niehaus.",0, "cniehaus@kde.org");

    return aboutData;
}

bool KalziumGLPart::openUrl(const KUrl& url)
{
    kDebug() << "KalziumGLPart::openURL()" << endl;
}

bool KalziumGLPart::openFile()
{
}
