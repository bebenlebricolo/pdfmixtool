/* Copyright (C) 2017-2021 Marco Scarpetta
 *
 * This file is part of PDF Mix Tool.
 *
 * PDF Mix Tool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PDF Mix Tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PDF Mix Tool. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include "mainwindow.h"
#include "mouseeventfilter.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < 0x060000
    qRegisterMetaTypeStreamOperators<Multipage>("Multipage");
#endif
    qRegisterMetaType<Multipage>();

    // Set application informations
    QApplication app(argc, argv);

    app.setApplicationName("pdfmixtool");
    app.setApplicationDisplayName("PDF Mix Tool");
    app.setOrganizationDomain("scarpetta.eu");
    app.setOrganizationName("PDFMixTool");
    app.setApplicationVersion("1.0.2");
    app.setDesktopFileName("eu.scarpetta.PDFMixTool");

    // Set up translations
    QTranslator translator;

    bool ok = translator.load(
                QString(":/tr/pdfmixtool_%1.qm").arg(QLocale::system().name()));

    if (ok) app.installTranslator(&translator);

    // Create and show the main window
    MainWindow main_window{};

    // parse input arguments
    if (argc > 1)
    {
        QStringList files;

        for (int i = 1; i < argc; i++)
        {
            QString filename(argv[i]);

            if (filename.endsWith(".pdf"))
                files.push_back(filename);
        }

        main_window.set_input_files(files);
    }

    main_window.show();

    return app.exec();
}
