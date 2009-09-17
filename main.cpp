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


#include <QtCore>

#include <iostream>

#include "svgminifier.h"

void showHelp()
{
    std::cout << "Usage:" << std::endl << std::endl;
    std::cout << "svgmin [options] input-file [output-file]" << std::endl << std::endl;
    std::cout << "Options (* marks the default):" << std::endl;
    std::cout <<  std::endl;
    std::cout <<  "--style-to-xml=yes [*]    Converts style properties into XML attributes" << std::endl;
    std::cout <<  "--style-to-xml=no         Keeps all style properties" << std::endl;
    std::cout <<  std::endl;
    std::cout <<  "--remove-metadata         Removes all metadata" << std::endl;
    std::cout <<  "--keep-metadata [*]       Keeps any metadata" << std::endl;
    std::cout <<  std::endl;
    std::cout <<  "--remove-editor-data [*]  Removes all Inkscape/Sodipodi data" << std::endl;
    std::cout <<  "--keep-editor-data        Keeps all Inkscape/Sodipodi data" << std::endl;
    std::cout <<  std::endl;
    std::cout <<  "--remove-id=foo           Removes all ids which start with 'foo'" << std::endl;
    std::cout <<  "--keep-id=foo             Keeps all ids which start with 'foo'" << std::endl;
    std::cout <<  std::endl;
    std::cout <<  "The default is to remove the following ids:" << std::endl;
    std::cout <<  "g, circle, path, polygon, polyline, rect, text" << std::endl;
    std::cout <<  "To override any of these, use the --keep-id option." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        showHelp();
        return 0;
    }

    QCoreApplication app(argc, argv);

    QString inputFile;
    QString outputFile;

    SvgMinifier minifier;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg[0] == '-') {
            arg.remove(0, 1);
            if (arg == "h" || arg == "-help") {
                showHelp();
                return 0;
            }

            if (arg == "-style-to-xml=yes")
                minifier.setConvertStyle(true);
            if (arg == "-style-to-xml=no")
                minifier.setConvertStyle(false);

            if (arg == "-simplify-style=yes")
                minifier.setSimplifyStyle(true);
            if (arg == "-simplify-style=no")
                minifier.setSimplifyStyle(false);

            if (arg == "-keep-metadata")
                minifier.setKeepMetadata(true);
            if (arg == "-remove-metadata")
                minifier.setKeepMetadata(false);

            if (arg == "-keep-editor-data")
                minifier.setKeepEditorData(true);
            if (arg == "-remove-editor-data")
                minifier.setKeepEditorData(false);

            if (arg.startsWith("-remove-id="))
                minifier.removeId(arg.mid(11)); // "-remove-id="
            if (arg.startsWith("-keep-id="))
                minifier.keepId(arg.mid(9)); // "-keep-id="

        } else {
            if (inputFile.isEmpty())
                inputFile = arg;
            else
                outputFile = arg;
        }
    }

    minifier.setInputFile(inputFile);
    minifier.setOutputFile(outputFile);
    minifier.run();

    return 0;
}

