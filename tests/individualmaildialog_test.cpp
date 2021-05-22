/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "individualmaildialog.h"

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
