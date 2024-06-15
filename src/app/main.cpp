/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "incidencedefaults.h"
#include "incidencedialog.h"
#include "korganizereditorconfig.h"

#include <Akonadi/Item>
#include <CalendarSupport/KCalPrefs>
#include <akonadi/calendarsettings.h>

#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <iostream>

#include "kincidenceeditor_debug.h"

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
    parser.addOption(QCommandLineOption(QStringLiteral("stdio"), QStringLiteral("Read/Write ical from stdin and stdio.")));
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

    QString stdinData;

    // This is used in Akonadi Calendar ITIPHandler, do not delete
    if (parser.isSet(QStringLiteral("stdio"))) {
        QTextStream qtin(stdin);
        stdinData = qtin.readAll();
        if (!stdinData.isEmpty()) {
            KCalendarCore::ICalFormat format;
            KCalendarCore::Incidence::Ptr incidence = format.fromString(stdinData);
            if (incidence.isNull()) {
                qCWarning(KINCIDENCEEDITOR_LOG) << "Invalid input data.";
                return 1;
            }
            if (incidence->type() == KCalendarCore::IncidenceBase::TypeTodo) {
                item.setPayload<KCalendarCore::Todo::Ptr>(incidence.dynamicCast<KCalendarCore::Todo>());
            } else if (incidence->type() == KCalendarCore::IncidenceBase::TypeEvent) {
                item.setPayload<KCalendarCore::Event::Ptr>(incidence.dynamicCast<KCalendarCore::Event>());
            } else if (incidence->type() == KCalendarCore::IncidenceBase::TypeJournal) {
                item.setPayload<KCalendarCore::Journal::Ptr>(incidence.dynamicCast<KCalendarCore::Journal>());
            }
        } else {
            qCWarning(KINCIDENCEEDITOR_LOG) << "Invalid usage. No stdin.";
            return 1;
        }
    } else if (parser.isSet(QStringLiteral("new-event"))) {
        qCDebug(KINCIDENCEEDITOR_LOG) << "Creating new event...";
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        defaults.setDefaults(event);
        item.setPayload<KCalendarCore::Event::Ptr>(event);
    } else if (parser.isSet(QStringLiteral("new-todo"))) {
        qCDebug(KINCIDENCEEDITOR_LOG) << "Creating new todo...";
        KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo);
        defaults.setDefaults(todo);
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
    } else if (parser.isSet(QStringLiteral("new-journal"))) {
        qCDebug(KINCIDENCEEDITOR_LOG) << "Creating new journal...";
        KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal);
        defaults.setDefaults(journal);
        item.setPayload<KCalendarCore::Journal::Ptr>(journal);
    } else if (parser.isSet(QStringLiteral("item"))) {
        bool ok = false;
        qint64 id = parser.value(QStringLiteral("item")).toLongLong(&ok);
        if (!ok) {
            qCWarning(KINCIDENCEEDITOR_LOG) << "Invalid akonadi item id given.";
            return 1;
        }

        item.setId(id);
        qCDebug(KINCIDENCEEDITOR_LOG) << "Trying to load Akonadi Item " << id << "...";
    } else {
        qCWarning(KINCIDENCEEDITOR_LOG) << "Invalid usage.";
        return 1;
    }

    EditorConfig::setEditorConfig(new KOrganizerEditorConfig);

    auto dialog = new IncidenceDialog();

    Akonadi::Collection collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());

    if (collection.isValid()) {
        dialog->selectCollection(collection);
    }

    dialog->load(item); // The dialog will show up once the item is loaded.
    if (!stdinData.isEmpty()) {
        QObject::connect(dialog, &IncidenceDialog::incidenceCreated, dialog, [](const Akonadi::Item &item) {
            KCalendarCore::ICalFormat format;
            KCalendarCore::Incidence::Ptr incidence;
            if (item.hasPayload<KCalendarCore::Journal::Ptr>()) {
                incidence = item.payload<KCalendarCore::Journal::Ptr>();
            } else if (item.hasPayload<KCalendarCore::Todo::Ptr>()) {
                incidence = item.payload<KCalendarCore::Todo::Ptr>();
            } else if (item.hasPayload<KCalendarCore::Event::Ptr>()) {
                incidence = item.payload<KCalendarCore::Event::Ptr>();
            }
            std::cout << format.toString(incidence).toStdString();
        });
    }

    return app.exec();
}
