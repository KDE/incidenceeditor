/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include "incidenceeditor_export.h"

#include <KLDAP/LdapClient>
#include <KLDAP/LdapClientSearch>

#include "resourceitem.h"
#include <CalendarSupport/FreeBusyCalendar>

#include <EventViews/ViewCalendar>

#include <QDialog>

class Ui_resourceManagement;

class QItemSelectionModel;

namespace EventViews
{
class AgendaView;
}

namespace IncidenceEditorNG
{
/**
 * @brief The ResourceManagement class
 */
class INCIDENCEEDITOR_EXPORT ResourceManagement : public QDialog
{
    Q_OBJECT
public:
    explicit ResourceManagement(QWidget *parent = nullptr);
    ~ResourceManagement() override;

    Q_REQUIRED_RESULT ResourceItem::Ptr selectedItem() const;

public Q_SLOTS:
    void slotDateChanged(const QDate &start, const QDate &end);

private:
    /* Shows the details of a resource
     *
     */
    void showDetails(const KLDAP::LdapObject &, const KLDAP::LdapClient &client);

    QItemSelectionModel *selectionModel = nullptr;

private:
    /* A new searchString is entered
     *
     */
    void slotStartSearch(const QString &);

    /* A detail view is requested
     *
     */
    void slotShowDetails(const QModelIndex &current);

    /**
     * The Owner search is done
     */
    void slotOwnerSearchFinished();

    void slotLayoutChanged();

private:
    void readConfig();
    void writeConfig();
    CalendarSupport::FreeBusyItemModel *mModel = nullptr;
    CalendarSupport::FreeBusyCalendar mFreebusyCalendar;
    ResourceItem::Ptr mOwnerItem;
    ResourceItem::Ptr mSelectedItem;
    EventViews::ViewCalendar::Ptr mFbCalendar;
    Ui_resourceManagement *mUi = nullptr;
    QMap<QModelIndex, KCalendarCore::Event::Ptr> mFbEvent;
    EventViews::AgendaView *mAgendaView = nullptr;
};
}
