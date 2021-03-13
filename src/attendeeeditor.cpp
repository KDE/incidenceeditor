/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "attendeeeditor.h"

#include "incidenceeditor_debug.h"

using namespace IncidenceEditorNG;

AttendeeEditor::AttendeeEditor(QWidget *parent)
    : MultiplyingLineEditor(new AttendeeLineFactory(parent), parent)
{
    connect(this, &AttendeeEditor::lineAdded, this, &AttendeeEditor::slotLineAdded);

    addData();
}

void AttendeeEditor::slotLineAdded(KPIM::MultiplyingLine *line)
{
    auto att = qobject_cast<AttendeeLine *>(line);
    if (!att) {
        return;
    }

    connect(att, qOverload<>(&AttendeeLine::changed), this, &AttendeeEditor::slotCalculateTotal);
    connect(att, qOverload<const KCalendarCore::Attendee &, const KCalendarCore::Attendee &>(&AttendeeLine::changed), this, &AttendeeEditor::changed);
    connect(att, &AttendeeLine::editingFinished, this, &AttendeeEditor::editingFinished);
}

void AttendeeEditor::slotCalculateTotal()
{
    int empty = 0;
    int count = 0;

    const QList<KPIM::MultiplyingLine *> listLines = lines();
    for (KPIM::MultiplyingLine *line : listLines) {
        auto att = qobject_cast<AttendeeLine *>(line);
        if (att) {
            if (att->isEmpty()) {
                ++empty;
            } else {
                ++count;
            }
        }
    }
    Q_EMIT countChanged(count);
    // We always want at least one empty line
    if (empty == 0) {
        addData();
    }
}

AttendeeData::List AttendeeEditor::attendees() const
{
    const QVector<KPIM::MultiplyingLineData::Ptr> dataList = allData();
    AttendeeData::List attList; // clazy:exclude=inefficient-qlist
    // qCDebug(INCIDENCEEDITOR_LOG) << "num attendees:" << dataList.size();
    for (const KPIM::MultiplyingLineData::Ptr &datum : dataList) {
        AttendeeData::Ptr att = qSharedPointerDynamicCast<AttendeeData>(datum);
        if (!att) {
            continue;
        }
        attList << att;
    }
    return attList;
}

void AttendeeEditor::addAttendee(const KCalendarCore::Attendee &attendee)
{
    addData(AttendeeData::Ptr(new AttendeeData(attendee)));
}

void AttendeeEditor::removeAttendee(const AttendeeData::Ptr &attendee)
{
    removeData(attendee);
}

void AttendeeEditor::setActions(AttendeeLine::AttendeeActions actions)
{
    const QList<KPIM::MultiplyingLine *> listLines = lines();
    for (KPIM::MultiplyingLine *line : listLines) {
        auto att = qobject_cast<AttendeeLine *>(line);
        att->setActions(actions);
    }
}
