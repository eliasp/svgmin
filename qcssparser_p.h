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

#ifndef QCSSPARSER_P_H
#define QCSSPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QString>
#include <QtCore/QVector>


// This is actually a much more simplified version of QCssParser

// Also we do not want to export anything
#undef Q_GUI_EXPORT
#define Q_GUI_EXPORT
#undef Q_AUTOTEST_EXPORT
#define Q_AUTOTEST_EXPORT

namespace QCss
{


enum TokenType {
    NONE,

    S,

    CDO,
    CDC,
    INCLUDES,
    DASHMATCH,

    LBRACE,
    PLUS,
    GREATER,
    COMMA,

    STRING,
    INVALID,

    IDENT,

    HASH,

    ATKEYWORD_SYM,

    EXCLAMATION_SYM,

    LENGTH,

    PERCENTAGE,
    NUMBER,

    FUNCTION,

    COLON,
    SEMICOLON,
    RBRACE,
    SLASH,
    MINUS,
    DOT,
    STAR,
    LBRACKET,
    RBRACKET,
    EQUAL,
    LPAREN,
    RPAREN,
    OR
};

struct Q_GUI_EXPORT Symbol
{
    inline Symbol() : token(NONE), start(0), len(-1) {}
    TokenType token;
    QString text;
    int start, len;
    QString lexem() const;
};

class Q_AUTOTEST_EXPORT Scanner
{
public:
    static QString preprocess(const QString &input, bool *hasEscapeSequences = 0);
    static void scan(const QString &preprocessedInput, QVector<Symbol> *symbols);
};

class Q_GUI_EXPORT Parser
{
public:
    Parser();
    Parser(const QString &css, bool file = false);

    void init(const QString &css, bool file = false);
    inline void skipSpace() { while (test(S)) {}; }
    inline bool hasNext() const { return index < symbols.count(); }
    inline TokenType next() { return symbols.at(index++).token; }
    bool next(TokenType t);
    bool test(TokenType t);
    inline const Symbol &symbol() const { return symbols.at(index - 1); }
    inline QString lexem() const { return symbol().lexem(); }

    QVector<Symbol> symbols;
    int index;
    int errorIndex;
    bool hasEscapeSequences;
    QString sourcePath;
};

} // namespace QCss


#endif
