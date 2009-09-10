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

#include <QtCore>

#include "qcssparser_p.h"

class SvgMinifier::Private
{
public:
    QString inFile;
    QString outFile;
    bool autoFormat;
    QStringList excludedTags;
    QStringList excludedPrefixes;
};

SvgMinifier::SvgMinifier()
{
    d = new Private;
    d->autoFormat = false;
}

SvgMinifier::~SvgMinifier()
{
    delete d;
}

void SvgMinifier::setInputFile(const QString &in)
{
    d->inFile = in;
}

void SvgMinifier::setOutputFile(const QString &out)
{
    d->outFile = out;
}

void SvgMinifier::setAutoFormat(bool format)
{
    d->autoFormat = format;
}

void SvgMinifier::addTagExclude(const QString &tag)
{
    d->excludedTags += tag;
}

void SvgMinifier::addPrefixExclude(const QString &prefix)
{
    d->excludedPrefixes += prefix;
}

static bool listContains(const QStringList &list, const QStringRef &str)
{
    if (str.isEmpty())
        return false;
    QString check = str.toString();
    foreach (const QString &s, list)
        if (s.startsWith(check))
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

// take the value of the "style" attribute, parse it and then
// merged the result with other XML attributes
static QXmlStreamAttributes mergedStyle(const QXmlStreamAttributes &attributes)
{
    if (!attributes.hasAttribute("style"))
        return attributes;

    QXmlStreamAttributes result;
    result.reserve(attributes.count());

    foreach (const QXmlStreamAttribute &attr, attributes)
        if (attr.name() != "style")
            result += attr;

    QXmlStreamAttributes styles = parseStyle(attributes.value("style"));
    foreach (const QXmlStreamAttribute &attr, styles)
        if (!attributes.hasAttribute(attr.value().toString()))
            result += attr;

    return result;
}

void SvgMinifier::run()
{
    QFile file;
    file.setFileName(d->inFile);
    if (!file.open(QFile::ReadOnly))
        return;

    QFile result;
    if (!d->outFile.isEmpty()) {
        result.setFileName(d->outFile);
        result.open(QFile::WriteOnly);
    } else {
        result.open(stdout, QFile::WriteOnly);
    }

    QXmlStreamReader *xml = new QXmlStreamReader(&file);
    xml->setNamespaceProcessing(false);
    QXmlStreamWriter *out = new QXmlStreamWriter(&result);
    out->setAutoFormatting(d->autoFormat);

    bool skip;
    QStack<bool> skipElement;
    skipElement.push(false);

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
                skip = listContains(d->excludedPrefixes, xml->prefix());
                skip = skip || d->excludedTags.contains(xml->name().toString());
                skipElement.push(skip);
                if (!skip) {
                    out->writeStartElement(xml->qualifiedName().toString());
                    QXmlStreamAttributes attr = mergedStyle(xml->attributes());
                    foreach (const QXmlStreamAttribute &a, attr)
                        if (!listContains(d->excludedPrefixes, a.prefix()))
                                out->writeAttribute(a);
                }
            }
            break;

        case QXmlStreamReader::EndElement:
            skip = skipElement.pop();
            if (!skip)
                out->writeEndElement();
            break;

        case QXmlStreamReader::Characters:
            if (!skipElement.top())
                out->writeCharacters(xml->text().toString());
            break;

        case QXmlStreamReader::ProcessingInstruction:
            out->writeProcessingInstruction(xml->processingInstructionTarget().toString(),
                                            xml->processingInstructionData().toString());
            break;

        default:
            break;
        }
    }

    file.close();
    result.close();
}


