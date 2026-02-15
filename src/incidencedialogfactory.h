/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include <Akonadi/Collection>
#include <KCalendarCore/IncidenceBase>

namespace Akonadi
{
class IncidenceChanger;
}

namespace IncidenceEditorNG
{
class IncidenceDialog;

namespace IncidenceDialogFactory
{
/*!
 * Creates a new IncidenceDialog for given type. Returns 0 for unsupported types.
 *
 * \a needsSaving If true, the editor will be initially dirty, and needs saving.
 *                    Apply button will be turned on. This is used for example when
 *                    we fill the editor with data that's not yet in akonadi, like
 *                    the "Create To-do/Reminder" in KMail.
 * \a type   The Incidence type for which to create a dialog.
 * \a changer The IncidenceChanger to use for modifications.
 * \a parent The parent widget of the dialog
 * \a flags  The window flags for the dialog.
 *
 * TODO: Implement support for Journals.
 * Note There is no editor for Incidence::TypeFreeBusy
 */
INCIDENCEEDITOR_EXPORT IncidenceDialog *create(bool needsSaving,
                                               KCalendarCore::IncidenceBase::IncidenceType type,
                                               Akonadi::IncidenceChanger *changer,
                                               QWidget *parent = nullptr,
                                               Qt::WindowFlags flags = {});

/*!
 * Creates a new IncidenceDialog for editing a To-Do with default values.
 *
 * \a summary The title of the To-Do.
 * \a description The description of the To-Do.
 * \a attachments List of attachment file paths.
 * \a attendees List of attendee email addresses.
 * \a attachmentMimetypes MIME types for each attachment.
 * \a attachmentLabels Labels for each attachment.
 * \a inlineAttachment If true, attachments are embedded inline.
 * \a defaultCollection The default collection for saving the item.
 * \a cleanupAttachmentTemp If true, temporary attachment files will be cleaned up.
 * \a parent The parent widget.
 * \a flags The window flags.
 * \return A new IncidenceDialog instance for To-Do editing.
 */
INCIDENCEEDITOR_EXPORT IncidenceDialog *createTodoEditor(const QString &summary,
                                                         const QString &description,
                                                         const QStringList &attachments,
                                                         const QStringList &attendees,
                                                         const QStringList &attachmentMimetypes,
                                                         const QStringList &attachmentLabels,
                                                         bool inlineAttachment,
                                                         const Akonadi::Collection &defaultCollection,
                                                         bool cleanupAttachmentTemp,
                                                         QWidget *parent = nullptr,
                                                         Qt::WindowFlags flags = {});

/*!
 * Creates a new IncidenceDialog for editing an Event with default values.
 *
 * \a summary The title of the Event.
 * \a description The description of the Event.
 * \a attachments List of attachment file paths.
 * \a attendees List of attendee email addresses.
 * \a attachmentMimetypes MIME types for each attachment.
 * \a attachmentLabels Labels for each attachment.
 * \a inlineAttachment If true, attachments are embedded inline.
 * \a defaultCollection The default collection for saving the item.
 * \a cleanupAttachmentTempFiles If true, temporary attachment files will be cleaned up.
 * \a parent The parent widget.
 * \a flags The window flags.
 * \return A new IncidenceDialog instance for Event editing.
 */
INCIDENCEEDITOR_EXPORT IncidenceDialog *createEventEditor(const QString &summary,
                                                          const QString &description,
                                                          const QStringList &attachments,
                                                          const QStringList &attendees,
                                                          const QStringList &attachmentMimetypes,
                                                          const QStringList &attachmentLabels,
                                                          bool inlineAttachment,
                                                          const Akonadi::Collection &defaultCollection,
                                                          bool cleanupAttachmentTempFiles,
                                                          QWidget *parent = nullptr,
                                                          Qt::WindowFlags flags = {});
} // namespace IncidenceDialogFactory
} // namespace IncidenceEditorNG
