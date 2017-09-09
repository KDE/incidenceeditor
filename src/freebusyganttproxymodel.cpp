/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "freebusyganttproxymodel.h"
#include "CalendarSupport/FreeBusyItemModel"

#include <KGantt/KGanttGraphicsView>
#include <KCalCore/FreeBusyPeriod>

#include <KLocalizedString>

#include <QLocale>

using namespace IncidenceEditorNG;

FreeBusyGanttProxyModel::FreeBusyGanttProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

QVariant FreeBusyGanttProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    QModelIndex source_index = mapToSource(index);

    // if the index is not valid, then its a toplevel item, which is an attendee
    if (!source_index.parent().isValid()) {
        switch (role) {
        case KGantt::ItemTypeRole:
            return KGantt::TypeMulti;
        case Qt::DisplayRole:
            return source_index.data(Qt::DisplayRole);
        default:
            return QVariant();
        }
    }

    // if the index is valid, then it corrsponds to a free busy period
    KCalCore::FreeBusyPeriod period
        = sourceModel()->data(source_index, CalendarSupport::FreeBusyItemModel::FreeBusyPeriodRole).
          value<KCalCore::FreeBusyPeriod>();

    switch (role) {
    case KGantt::ItemTypeRole:
        return KGantt::TypeTask;
    case KGantt::StartTimeRole:
        return period.start().toLocalZone().dateTime();
    case KGantt::EndTimeRole:
        return period.end().toLocalZone().dateTime();
    case Qt::BackgroundRole:
        return QColor(Qt::red);
    case Qt::ToolTipRole:
        return tooltipify(period);
    case Qt::DisplayRole:
        return sourceModel()->data(source_index.parent(), Qt::DisplayRole);
    default:
        return QVariant();
    }
}

QString FreeBusyGanttProxyModel::tooltipify(const KCalCore::FreeBusyPeriod &period) const
{
    QString toolTip = QStringLiteral("<qt>");
    toolTip += QStringLiteral("<b>") + i18nc("@info:tooltip", "Free/Busy Period") + QStringLiteral(
        "</b>");
    toolTip += QStringLiteral("<hr>");
    if (!period.summary().isEmpty()) {
        toolTip += QStringLiteral("<i>") + i18nc("@info:tooltip", "Summary:") + QStringLiteral(
            "</i>") + QStringLiteral("&nbsp;");
        toolTip += period.summary();
        toolTip += QStringLiteral("<br>");
    }
    if (!period.location().isEmpty()) {
        toolTip += QStringLiteral("<i>") + i18nc("@info:tooltip", "Location:") + QStringLiteral(
            "</i>") + QStringLiteral("&nbsp;");
        toolTip += period.location();
        toolTip += QStringLiteral("<br>");
    }
    toolTip += QStringLiteral("<i>")
               + i18nc("@info:tooltip period start time",
                       "Start:") + QStringLiteral("</i>") + QStringLiteral(
        "&nbsp;");
    toolTip += QLocale().toString(period.start().toLocalZone().dateTime(), QLocale::ShortFormat);
    toolTip += QStringLiteral("<br>");
    toolTip += QStringLiteral("<i>")
               + i18nc("@info:tooltip period end time",
                       "End:") + QStringLiteral("</i>") + QStringLiteral(
        "&nbsp;");
    toolTip += QLocale().toString(period.end().toLocalZone().dateTime(), QLocale::ShortFormat);
    toolTip += QStringLiteral("<br>");
    toolTip += QStringLiteral("</qt>");
    return toolTip;
}
