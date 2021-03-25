/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include <Collection>
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
/**
 * Creates a new IncidenceDialog for given type. Returns 0 for unsupported types.
 *
 * @param needsSaving If true, the editor will be initially dirty, and needs saving.
 *                    Apply button will be turned on. This is used for example when
 *                    we fill the editor with data that's not yet in akonadi, like
 *                    the "Create To-do/Reminder" in KMail.
 * @param type   The Incidence type for which to create a dialog.
 * @param parent The parent widget of the dialog
 * @param flags  The window flags for the dialog.
 *
 * TODO: Implement support for Journals.
 * NOTE: There is no editor for Incidence::TypeFreeBusy
 */
INCIDENCEEDITOR_EXPORT IncidenceDialog *create(bool needsSaving,
                                               KCalendarCore::IncidenceBase::IncidenceType type,
                                               Akonadi::IncidenceChanger *changer,
                                               QWidget *parent = nullptr,
                                               Qt::WindowFlags flags = {});

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

