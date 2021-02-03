/* SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "resourcemanagement.h"

#include <QApplication>
#include <QCommandLineParser>

using namespace IncidenceEditorNG;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    auto dialog = new ResourceManagement();

    dialog->show();

    return app.exec();
}
