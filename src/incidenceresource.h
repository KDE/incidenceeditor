/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include "attendeetablemodel.h"
#include "incidenceattendee.h"
#include "incidenceeditor-ng.h"

namespace Ui
{
class EventOrTodoDesktop;
}
class QCompleter;
namespace IncidenceEditorNG
{
class ResourceManagement;

class IncidenceResource : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceResource(IncidenceAttendee *mIeAttendee, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui);
    ~IncidenceResource() override;

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    bool isDirty() const override;

    /** return the count of resources */
    Q_REQUIRED_RESULT int resourceCount() const;

Q_SIGNALS:
    /** is emitted it the count of the resources is changed.
     * @arg: new count of resources.
     */
    void resourceCountChanged(int);

private:
    void findResources();
    void bookResource();
    void layoutChanged();
    void updateCount();

    void slotDateChanged();

    void dialogOkPressed();
    Ui::EventOrTodoDesktop *mUi = nullptr;

    /** completer for findResources */
    QCompleter *completer = nullptr;

    /** used dataModel to rely on*/
    AttendeeTableModel *dataModel = nullptr;
    IncidenceDateTime *mDateTime = nullptr;

    ResourceManagement *resourceDialog = nullptr;
};
}

