/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "incidencedefaults.h"
using namespace Qt::Literals::StringLiterals;

#include "incidencedialog.h"
#include "korganizereditorconfig.h"

#include <Akonadi/Item>
#include <CalendarSupport/KCalPrefs>
#include <akonadi/calendarsettings.h>

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
    QCoreApplication::setApplicationName(u"IncidenceEditorNGApp"_s);
    QCoreApplication::setApplicationVersion(u"0.1"_s);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(u"new-event"_s, u"Creates a new event"_s));
    parser.addOption(QCommandLineOption(u"new-todo"_s, u"Creates a new todo"_s));
    parser.addOption(QCommandLineOption(u"new-journal"_s, u"Creates a new journal"_s));
    parser.addOption(QCommandLineOption(u"item"_s,
                                        QStringLiteral("Loads an existing item, or returns without doing anything "
                                                       "when the item is not an event or todo."),
                                        u"id"_s));
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

    // NOLINTBEGIN(performance-avoid-endl)
    if (parser.isSet(u"new-event"_s)) {
        std::cout << "Creating new event..." << std::endl;
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        defaults.setDefaults(event);
        item.setPayload<KCalendarCore::Event::Ptr>(event);
    } else if (parser.isSet(u"new-todo"_s)) {
        std::cout << "Creating new todo..." << std::endl;
        KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo);
        defaults.setDefaults(todo);
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
    } else if (parser.isSet(u"new-journal"_s)) {
        std::cout << "Creating new journal..." << std::endl;
        KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal);
        defaults.setDefaults(journal);
        item.setPayload<KCalendarCore::Journal::Ptr>(journal);
    } else if (parser.isSet(u"item"_s)) {
        bool ok = false;
        qint64 id = parser.value(u"item"_s).toLongLong(&ok);
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
    // NOLINTEND(performance-avoid-endl)

    EditorConfig::setEditorConfig(new KOrganizerEditorConfig);

    auto dialog = new IncidenceDialog();

    Akonadi::Collection collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());

    if (collection.isValid()) {
        dialog->selectCollection(collection);
    }

    dialog->load(item); // The dialog will show up once the item is loaded.

    return app.exec();
}
