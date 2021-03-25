/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include <QSortFilterProxyModel>

namespace KCalendarCore
{
class FreeBusyPeriod;
}

namespace IncidenceEditorNG
{
/**
 * This is a private proxy model, that wraps the free busy data exposed
 * by the FreeBusyItemModel for use by KDGantt2.
 *
 * This model exposes the FreeBusyPeriods, which are the child level nodes
 * in FreeBusyItemModel, as a list.
 *
 * @see FreeBusyItemMode
 * @see FreeBusyItem
 */
class INCIDENCEEDITOR_EXPORT FreeBusyGanttProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FreeBusyGanttProxyModel(QObject *parent = nullptr);
    Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_REQUIRED_RESULT QString tooltipify(const KCalendarCore::FreeBusyPeriod &period) const;
};
}

