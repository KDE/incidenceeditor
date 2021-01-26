/*
  SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "groupwareuidelegate.h"
#include "incidencedialog.h"
#include "incidencedialogfactory.h"

#include <CalendarSupport/Utils>

#include "incidenceeditor_debug.h"
#include <Item>
using namespace IncidenceEditorNG;

void GroupwareUiDelegate::requestIncidenceEditor(const Akonadi::Item &item)
{
    // TODO_KDE5:
    // The GroupwareUiDelegate interface should be a QObject. Right now we have no way of emitting a
    // finished signal, so we have to use dialog->exec();

    const KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    if (!incidence) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Incidence is null, won't open the editor";
        return;
    }

    IncidenceDialog *dialog = IncidenceDialogFactory::create(/*needs initial saving=*/false, incidence->type(), nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose, false);
    dialog->setIsCounterProposal(true);
    dialog->load(item, QDate::currentDate());
    dialog->exec();
    dialog->deleteLater();
    Akonadi::Item newItem = dialog->item();
    if (newItem.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        KCalendarCore::IncidenceBase::Ptr newIncidence = newItem.payload<KCalendarCore::Incidence::Ptr>();
        *incidence.staticCast<KCalendarCore::IncidenceBase>() = *newIncidence;
    }
}

void GroupwareUiDelegate::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    // We don't need a calendar.
    Q_UNUSED(calendar)
}

void GroupwareUiDelegate::createCalendar()
{
    // We don't need a calendar
}
