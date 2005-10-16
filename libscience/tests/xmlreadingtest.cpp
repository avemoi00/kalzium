/* Sample parsing with QT's SAX2 by Riku Leino <tsoots@gmail.com> */

#include "../elementparser.h"
#include "../element.h"
#include <kdebug.h>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 2 || argc > 2) {
		std::cout << "Usage: elements <XML_FILE>\n";
		return 1;
	}

	ElementSaxParser * parser = new ElementSaxParser();
	QFile xmlFile(argv[1]);
	QXmlInputSource source(xmlFile);
	QXmlSimpleReader reader;
	reader.setContentHandler(parser);
	reader.parse(source);

	QList<Element*> v = parser->getElements();

	foreach( Element* e, v ){
		if ( e )
			kdDebug() << "(" << e->number() << ", " <<
				       e->elementName() << ", " << e->symbol() << ") " << 
				", mass: " << e->mass() << 
				endl;
	}

	return 0;
}
