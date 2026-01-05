/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
 */

#pragma once

#include "incidenceeditor_export.h"

#include <KLDAPCore/LdapClient>

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
 * \brief The ResourceManagement class
 */
class INCIDENCEEDITOR_EXPORT ResourceManagement : public QDialog
{
    Q_OBJECT
public:
    /*!
     */
    explicit ResourceManagement(QWidget *parent = nullptr);
    /*!
     */
    ~ResourceManagement() override;

    /*!
     */
    [[nodiscard]] ResourceItem::Ptr selectedItem() const;

public Q_SLOTS:
    /*!
     */
    void slotDateChanged(const QDate &start, const QDate &end);

private:
    /* Shows the details of a resource
     *
     */
    INCIDENCEEDITOR_NO_EXPORT void showDetails(const KLDAPCore::LdapObject &, const KLDAPCore::LdapClient &client);

    QItemSelectionModel *selectionModel = nullptr;

    /* A new searchString is entered
     *
     */
    INCIDENCEEDITOR_NO_EXPORT void slotStartSearch(const QString &);

    /* A detail view is requested
     *
     */
    INCIDENCEEDITOR_NO_EXPORT void slotShowDetails(const QModelIndex &current);

    /*!
     * The Owner search is done
     */
    INCIDENCEEDITOR_NO_EXPORT void slotOwnerSearchFinished();

    INCIDENCEEDITOR_NO_EXPORT void slotLayoutChanged();

    INCIDENCEEDITOR_NO_EXPORT void readConfig();
    INCIDENCEEDITOR_NO_EXPORT void writeConfig();
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
