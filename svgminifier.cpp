/*
  SVGMin - Aggressive SVG minifier

  Copyright (C) 2009 Ariya Hidayat (ariya.hidayat@gmail.com)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "svgminifier.h"

#include <QtCore/QIODevice>
#include <QtCore/QFile>
#include <QtCore/QStack>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

#include "qcssparser_p.h"

class SvgMinifier::Private
{
public:
    QIODevice *inputDevice;
    QIODevice *outputDevice;
    bool convertStyle;
    bool simplifyStyle;
    bool keepMetadata;
    bool keepEditorData;
    QStringList editorNamespaces;
    QStringList editorPrefixes;
    QStringList excludedId;
};

SvgMinifier::SvgMinifier()
{
    d = new Private;

    d->inputDevice = 0;
    d->outputDevice = 0;

    d->convertStyle = true;
    d->simplifyStyle = true;
    d->keepMetadata = true;
    d->keepEditorData = false;

    d->editorNamespaces << "http://www.inkscape.org/namespaces/inkscape";
    d->editorNamespaces << "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd";
    d->editorNamespaces << "http://ns.adobe.com/AdobeIllustrator/10.0/";
    d->editorNamespaces << "http://ns.adobe.com/Graphs/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/";
    d->editorNamespaces << "http://ns.adobe.com/Variables/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/SaveForWeb/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/Extensibility/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/Flows/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/ImageReplacement/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/GenericCustomNamespace/1.0/";
    d->editorNamespaces << "http://ns.adobe.com/XPath/1.0/";

    d->excludedId << "g";
    d->excludedId << "circle";
    d->excludedId << "path";
    d->excludedId << "polygon";
    d->excludedId << "polyline";
    d->excludedId << "rect";
    d->excludedId << "text";
}

SvgMinifier::~SvgMinifier()
{
    delete d;
}

void SvgMinifier::setInputDevice(QIODevice *device)
{
    d->inputDevice = device;
}

void SvgMinifier::setOutputDevice(QIODevice *device)
{
    d->outputDevice = device;
}

void SvgMinifier::setConvertStyle(bool convert)
{
    d->convertStyle = convert;
}

void SvgMinifier::setSimplifyStyle(bool simplify)
{
    d->simplifyStyle = simplify;
}

void SvgMinifier::setKeepMetadata(bool keep)
{
    d->keepMetadata = keep;
}

void SvgMinifier::setKeepEditorData(bool keep)
{
    d->keepEditorData = keep;
}

void SvgMinifier::removeId(const QString &id)
{
    if (!d->excludedId.contains(id))
        d->excludedId += id;
}

void SvgMinifier::keepId(const QString &id)
{
    d->excludedId.removeAll(id);
}

static bool listContains(const QStringList &list, const QStringRef &str)
{
    if (str.isEmpty())
        return false;
    QString check = str.toString();
    foreach (const QString &s, list)
        if (check.startsWith(s))
            return true;
    return false;
}


static QXmlStreamAttributes parseStyle(const QStringRef &styleRef)
{
    QXmlStreamAttributes attributes;
    QCss::Parser parser;

    parser.init(styleRef.toString());
    QString key;

    while (parser.hasNext()) {
        parser.skipSpace();

        if (!parser.hasNext())
            break;
        parser.next();

        QString name = parser.lexem();

        parser.skipSpace();
        if (!parser.test(QCss::COLON))
            break;

        parser.skipSpace();
        if (!parser.hasNext())
            break;

        const int firstSymbol = parser.index;
        int symbolCount = 0;
        do {
            parser.next();
            ++symbolCount;
        } while (parser.hasNext() && !parser.test(QCss::SEMICOLON));

        QString value;
        for (int i = firstSymbol; i < firstSymbol + symbolCount; ++i)
            value += parser.symbols.at(i).lexem();

        parser.skipSpace();

        attributes.append(name, value);
    }

    return attributes;
}

// convenient function to remove an attribute given the name
static QXmlStreamAttributes attrRemoved(const QXmlStreamAttributes &attributes,
                                        const QString &name)
{
    QXmlStreamAttributes result;
    result.reserve(qMax(1, attributes.count() - 1));

    foreach (const QXmlStreamAttribute &attr, attributes)
        if (attr.name() != name)
            result += attr;

    return result;
}

// take the value of the "style" attribute, parse it and then
// merged the result with other XML attributes
static QXmlStreamAttributes mergedStyle(const QXmlStreamAttributes &attributes)
{
    if (!attributes.hasAttribute("style"))
        return attributes;

    QXmlStreamAttributes result = attrRemoved(attributes, "style");

    QXmlStreamAttributes styles = parseStyle(attributes.value("style"));
    foreach (const QXmlStreamAttribute &attr, styles)
        if (!attributes.hasAttribute(attr.value().toString()))
            result += attr;

    return result;
}

static bool isDrawingNode(const QStringRef &str)
{
    if (str == QLatin1String("linearGradient"))
        return false;
    if (str == QLatin1String("path"))
        return true;
    if (str == QLatin1String("text"))
        return true;
    if (str == QLatin1String("g"))
        return true;
    if (str == QLatin1String("rect"))
        return true;
    if (str == QLatin1String("circle"))
        return true;
    if (str == QLatin1String("polygon"))
        return true;
    if (str == QLatin1String("polyline"))
        return true;
    return false;
}

void SvgMinifier::run()
{
    // fall back to standard input
    QFile standardInput;
    if (!d->inputDevice) {
        standardInput.open(stdin, QFile::ReadOnly);
        d->inputDevice = &standardInput;
    }

    // fall back to standard output
    QFile standardOutput;
    if (!d->outputDevice) {
        standardOutput.open(stdout, QFile::WriteOnly);
        d->outputDevice = &standardOutput;
    }

    QXmlStreamReader *xml = new QXmlStreamReader(d->inputDevice);
    xml->setNamespaceProcessing(false);
    QXmlStreamWriter *out = new QXmlStreamWriter(d->outputDevice);
    out->setAutoFormatting(true);

    bool skip;
    QStack<bool> skipElement;
    skipElement.push(false);

    QXmlStreamAttributes attr;

    while (!xml->atEnd()) {
        switch (xml->readNext()) {

        case QXmlStreamReader::StartDocument:
            out->writeStartDocument(xml->documentVersion().toString(),
                                    xml->isStandaloneDocument());
            break;

        case QXmlStreamReader::EndDocument:
            out->writeEndDocument();
            break;

        case QXmlStreamReader::StartElement:
            if (skipElement.top()) {
                skipElement.push(true);
            } else {
                attr = xml->attributes();

                if (xml->name() == "svg" && !d->keepEditorData)
                    foreach (const QXmlStreamAttribute &a, attr)
                        if (d->editorNamespaces.contains(a.value().toString()))
                            if (a.prefix() == "xmlns")
                                d->editorPrefixes += a.name().toString();

                if (d->editorPrefixes.count())
                    foreach (QString ns, d->editorPrefixes)
                        attr = attrRemoved(attr, ns);

                skip = d->editorPrefixes.contains(xml->prefix().toString());
                if (!skip && !d->keepMetadata)
                    skip = xml->name() == "metadata";

                skipElement.push(skip);
                if (!skip) {
                    const QStringRef &tag = xml->qualifiedName();
                    out->writeStartElement(tag.toString());
                    if (d->convertStyle)
                        attr = mergedStyle(attr);
                    foreach (const QXmlStreamAttribute &a, attr) {
                        if (d->editorPrefixes.contains(a.prefix().toString()))
                            continue;
                        if (a.qualifiedName() == "id" && isDrawingNode(tag))
                            if (listContains(d->excludedId, a.value()))
                                continue;
                        out->writeAttribute(a);
                    }
                }
            }
            break;

        case QXmlStreamReader::EndElement:
            skip = skipElement.pop();
            if (!skip)
                out->writeEndElement();
            break;

        case QXmlStreamReader::Characters:
            if (!skipElement.top()) {
                if (xml->isCDATA())
                    out->writeCDATA(xml->text().toString());
                else
                    out->writeCharacters(xml->text().toString());
            }
            break;

        case QXmlStreamReader::ProcessingInstruction:
            out->writeProcessingInstruction(xml->processingInstructionTarget().toString(),
                                            xml->processingInstructionData().toString());
            break;

        default:
            break;
        }
    }

    if (standardInput.isOpen())
        standardInput.close();

    if (standardOutput.isOpen())
        standardOutput.close();
}


