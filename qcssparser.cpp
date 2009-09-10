/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcssparser_p.h"

// This is actually a much more simplified version of QCssParser
// See also qcssparser_p.h

#include <QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include "qcssscanner.cpp"

using namespace QCss;

static inline bool isHexDigit(const char c)
{
    return (c >= '0' && c <= '9')
           || (c >= 'a' && c <= 'f')
           || (c >= 'A' && c <= 'F')
           ;
}

QString Scanner::preprocess(const QString &input, bool *hasEscapeSequences)
{
    QString output = input;

    if (hasEscapeSequences)
        *hasEscapeSequences = false;

    int i = 0;
    while (i < output.size()) {
        if (output.at(i) == QLatin1Char('\\')) {

            ++i;
            // test for unicode hex escape
            int hexCount = 0;
            const int hexStart = i;
            while (i < output.size()
                   && isHexDigit(output.at(i).toLatin1())
                   && hexCount < 7) {
                ++hexCount;
                ++i;
            }
            if (hexCount == 0) {
                if (hasEscapeSequences)
                    *hasEscapeSequences = true;
                continue;
            }

            hexCount = qMin(hexCount, 6);
            bool ok = false;
            ushort code = output.mid(hexStart, hexCount).toUShort(&ok, 16);
            if (ok) {
                output.replace(hexStart - 1, hexCount + 1, QChar(code));
                i = hexStart;
            } else {
                i = hexStart;
            }
        } else {
            ++i;
        }
    }
    return output;
}

int QCssScanner_Generated::handleCommentStart()
{
    while (pos < input.size() - 1) {
        if (input.at(pos) == QLatin1Char('*')
            && input.at(pos + 1) == QLatin1Char('/')) {
            pos += 2;
            break;
        }
        ++pos;
    }
    return S;
}

void Scanner::scan(const QString &preprocessedInput, QVector<Symbol> *symbols)
{
    QCssScanner_Generated scanner(preprocessedInput);
    Symbol sym;
    int tok = scanner.lex();
    while (tok != -1) {
        sym.token = static_cast<QCss::TokenType>(tok);
        sym.text = scanner.input;
        sym.start = scanner.lexemStart;
        sym.len = scanner.lexemLength;
        symbols->append(sym);
        tok = scanner.lex();
    }
}

QString Symbol::lexem() const
{
    QString result;
    if (len > 0)
        result.reserve(len);
    for (int i = 0; i < len; ++i) {
        if (text.at(start + i) == QLatin1Char('\\') && i < len - 1)
            ++i;
        result += text.at(start + i);
    }
    return result;
}

Parser::Parser(const QString &css, bool isFile)
{
    init(css, isFile);
}

Parser::Parser()
{
    index = 0;
    errorIndex = -1;
    hasEscapeSequences = false;
}

void Parser::init(const QString &css, bool isFile)
{
    QString styleSheet = css;
    if (isFile) {
        QFile file(css);
        if (file.open(QFile::ReadOnly)) {
            sourcePath = QFileInfo(styleSheet).absolutePath() + QLatin1Char('/');
            QTextStream stream(&file);
            styleSheet = stream.readAll();
        } else {
            qWarning() << "QCss::Parser - Failed to load file " << css;
            styleSheet.clear();
        }
    } else {
        sourcePath.clear();
    }

    hasEscapeSequences = false;
    symbols.resize(0);
    symbols.reserve(8);
    Scanner::scan(Scanner::preprocess(styleSheet, &hasEscapeSequences), &symbols);
    index = 0;
    errorIndex = -1;
}

bool Parser::test(QCss::TokenType t)
{
    if (index >= symbols.count())
        return false;
    if (symbols.at(index).token == t) {
        ++index;
        return true;
    }
    return false;
}
