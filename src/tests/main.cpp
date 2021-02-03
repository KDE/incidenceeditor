/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "incidencedefaults.h"
#include "incidencedialog.h"
#include "korganizereditorconfig.h"

#include <CalendarSupport/KCalPrefs>
#include <Item>
#include <akonadi/calendar/calendarsettings.h>

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <iostream>

using namespace IncidenceEditorNG;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("IncidenceEditorNGApp"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(QStringLiteral("new-event"), QStringLiteral("Creates a new event")));
    parser.addOption(QCommandLineOption(QStringLiteral("new-todo"), QStringLiteral("Creates a new todo")));
    parser.addOption(QCommandLineOption(QStringLiteral("new-journal"), QStringLiteral("Creates a new journal")));
    parser.addOption(QCommandLineOption(QStringLiteral("item"),
                                        QStringLiteral("Loads an existing item, or returns without doing anything "
                                                       "when the item is not an event or todo."),
                                        QStringLiteral("id")));
    parser.process(app);

    Akonadi::Item item(-1);

    IncidenceDefaults defaults;
    // Set the full emails manually here, to avoid that we get dependencies on
    // KCalPrefs all over the place.
    defaults.setFullEmails(CalendarSupport::KCalPrefs::instance()->fullEmails());
    // NOTE: At some point this should be generalized. That is, we now use the
    //       freebusy url as a hack, but this assumes that the user has only one
    //       groupware account. Which doesn't have to be the case necessarily.
    //       This method should somehow depend on the calendar selected to which
    //       the incidence is added.
    if (CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication()) {
        defaults.setGroupWareDomain(QUrl(Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl()).host());
    }

    if (parser.isSet(QStringLiteral("new-event"))) {
        std::cout << "Creating new event..." << std::endl;
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        defaults.setDefaults(event);
        item.setPayload<KCalendarCore::Event::Ptr>(event);
    } else if (parser.isSet(QStringLiteral("new-todo"))) {
        std::cout << "Creating new todo..." << std::endl;
        KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo);
        defaults.setDefaults(todo);
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
    } else if (parser.isSet(QStringLiteral("new-journal"))) {
        std::cout << "Creating new journal..." << std::endl;
        KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal);
        defaults.setDefaults(journal);
        item.setPayload<KCalendarCore::Journal::Ptr>(journal);
    } else if (parser.isSet(QStringLiteral("item"))) {
        bool ok = false;
        qint64 id = parser.value(QStringLiteral("item")).toLongLong(&ok);
        if (!ok) {
            std::cerr << "Invalid akonadi item id given." << std::endl;
            return 1;
        }

        item.setId(id);
        std::cout << "Trying to load Akonadi Item " << QString::number(id).toLatin1().data();
        std::cout << "..." << std::endl;
    } else {
        std::cerr << "Invalid usage." << std::endl << std::endl;
        return 1;
    }

    EditorConfig::setEditorConfig(new KOrganizerEditorConfig);

    auto dialog = new IncidenceDialog();

    Akonadi::Collection collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());

    if (collection.isValid()) {
        dialog->selectCollection(collection);
    }

    dialog->load(item); // The dialog will show up once the item is loaded.

    return app.exec();
}
