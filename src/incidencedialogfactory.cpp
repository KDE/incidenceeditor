/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencedialogfactory.h"
#include "incidencedefaults.h"
#include "incidencedialog.h"

#include <Akonadi/Calendar/IncidenceChanger>
#include <Item>
#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>

using namespace IncidenceEditorNG;
using namespace KCalendarCore;

IncidenceDialog *IncidenceDialogFactory::create(bool needsSaving,
                                                KCalendarCore::IncidenceBase::IncidenceType type,
                                                Akonadi::IncidenceChanger *changer,
                                                QWidget *parent,
                                                Qt::WindowFlags flags)
{
    switch (type) {
    case KCalendarCore::IncidenceBase::TypeEvent: // Fall through
    case KCalendarCore::IncidenceBase::TypeTodo:
    case KCalendarCore::IncidenceBase::TypeJournal: {
        auto dialog = new IncidenceDialog(changer, parent, flags);

        // needs to be save to akonadi?, apply button should be turned on if so.
        dialog->setInitiallyDirty(needsSaving /* mInitiallyDirty */);

        return dialog;
    }
    default:
        return nullptr;
    }
}

IncidenceDialog *IncidenceDialogFactory::createTodoEditor(const QString &summary,
                                                          const QString &description,
                                                          const QStringList &attachments,
                                                          const QStringList &attendees,
                                                          const QStringList &attachmentMimetypes,
                                                          const QStringList &attachmentLabels,
                                                          bool inlineAttachment,
                                                          const Akonadi::Collection &defaultCollection,
                                                          bool cleanupAttachmentTempFiles,
                                                          QWidget *parent,
                                                          Qt::WindowFlags flags)
{
    IncidenceDefaults defaults = IncidenceDefaults::minimalIncidenceDefaults(cleanupAttachmentTempFiles);

    // if attach or attendee list is empty, these methods don't do anything, so
    // it's safe to call them in every case
    defaults.setAttachments(attachments, attachmentMimetypes, attachmentLabels, inlineAttachment);
    defaults.setAttendees(attendees);

    Todo::Ptr todo(new Todo);
    defaults.setDefaults(todo);

    todo->setSummary(summary);
    todo->setDescription(description);

    Akonadi::Item item;
    item.setPayload(todo);

    IncidenceDialog *dialog = create(true, /* no need for, we're not editing an existing to-do */
                                     KCalendarCore::Incidence::TypeTodo,
                                     nullptr,
                                     parent,
                                     flags);
    dialog->selectCollection(defaultCollection);
    dialog->load(item);
    return dialog;
}

IncidenceDialog *IncidenceDialogFactory::createEventEditor(const QString &summary,
                                                           const QString &description,
                                                           const QStringList &attachments,
                                                           const QStringList &attendees,
                                                           const QStringList &attachmentMimetypes,
                                                           const QStringList &attachmentLabels,
                                                           bool inlineAttachment,
                                                           const Akonadi::Collection &defaultCollection,
                                                           bool cleanupAttachmentTempFiles,
                                                           QWidget *parent,
                                                           Qt::WindowFlags flags)
{
    IncidenceDefaults defaults = IncidenceDefaults::minimalIncidenceDefaults(cleanupAttachmentTempFiles);

    // if attach or attendee list is empty, these methods don't do anything, so
    // it's safe to call them in every case
    defaults.setAttachments(attachments, attachmentMimetypes, attachmentLabels, inlineAttachment);
    defaults.setAttendees(attendees);

    Event::Ptr event(new Event);
    defaults.setDefaults(event);

    event->setSummary(summary);
    event->setDescription(description);

    Akonadi::Item item;
    item.setPayload(event);

    IncidenceDialog *dialog = create(false, // not needed for saving, as we're not editing an existing incidence
                                     KCalendarCore::Incidence::TypeEvent,
                                     nullptr,
                                     parent,
                                     flags);

    dialog->selectCollection(defaultCollection);
    dialog->load(item);

    return dialog;
}
