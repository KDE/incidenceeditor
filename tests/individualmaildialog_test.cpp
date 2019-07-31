/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "../src/individualmaildialog.h"

#include <KGuiItem>
#include <QApplication>

using namespace IncidenceEditorNG;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KCalendarCore::Attendee::List attendees;
    KGuiItem buttonYes = KGuiItem(QStringLiteral("Send Email"));
    KGuiItem buttonNo = KGuiItem(QStringLiteral("Do not send"));

    KCalendarCore::Attendee attendee1(QStringLiteral("test1"), QStringLiteral("test1@example.com"));
    KCalendarCore::Attendee attendee2(QStringLiteral("test2"), QStringLiteral("test2@example.com"));
    KCalendarCore::Attendee attendee3(QStringLiteral("test3"), QStringLiteral("test3@example.com"));

    attendees << attendee1 << attendee2 << attendee3;

    IndividualMailDialog dialog(QStringLiteral("title"), attendees, buttonYes, buttonNo, nullptr);
    dialog.show();
    return app.exec();
}
