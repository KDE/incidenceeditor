/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "freebusyganttproxymodel.h"
using namespace Qt::Literals::StringLiterals;

#include <CalendarSupport/FreeBusyItemModel>

#include <KCalendarCore/FreeBusyPeriod>
#include <KGanttGraphicsView>

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
        return {};
    }
    QModelIndex const source_index = mapToSource(index);

    // if the index is not valid, then its a toplevel item, which is an attendee
    if (!source_index.parent().isValid()) {
        switch (role) {
        case KGantt::ItemTypeRole:
            return KGantt::TypeMulti;
        case Qt::DisplayRole:
            return source_index.data(Qt::DisplayRole);
        default:
            return {};
        }
    }

    // if the index is valid, then it corresponds to a free busy period
    auto period = sourceModel()->data(source_index, CalendarSupport::FreeBusyItemModel::FreeBusyPeriodRole).value<KCalendarCore::FreeBusyPeriod>();

    switch (role) {
    case KGantt::ItemTypeRole:
        return KGantt::TypeTask;
    case KGantt::StartTimeRole:
        return period.start().toLocalTime();
    case KGantt::EndTimeRole:
        return period.end().toLocalTime();
    case Qt::BackgroundRole:
        return QColor(Qt::red);
    case Qt::ToolTipRole:
        return tooltipify(period);
    case Qt::DisplayRole:
        return sourceModel()->data(source_index.parent(), Qt::DisplayRole);
    default:
        return {};
    }
}

QString FreeBusyGanttProxyModel::tooltipify(const KCalendarCore::FreeBusyPeriod &period) const
{
    QString toolTip = u"<qt>"_s;
    toolTip += QLatin1StringView("<b>") + i18nc("@info:tooltip", "Free/Busy Period") + QLatin1StringView("</b>");
    toolTip += u"<hr>"_s;
    if (!period.summary().isEmpty()) {
        toolTip += QLatin1StringView("<i>") + i18nc("@info:tooltip", "Summary:") + QLatin1StringView("</i>") + u"&nbsp;"_s;
        toolTip += period.summary();
        toolTip += u"<br>"_s;
    }
    if (!period.location().isEmpty()) {
        toolTip += QLatin1StringView("<i>") + i18nc("@info:tooltip", "Location:") + QLatin1StringView("</i>") + u"&nbsp;"_s;
        toolTip += period.location();
        toolTip += u"<br>"_s;
    }
    toolTip += u"<i>"_s + i18nc("@info:tooltip period start time", "Start:") + u"</i>"_s + QStringLiteral("&nbsp;");
    toolTip += QLocale().toString(period.start().toLocalTime(), QLocale::ShortFormat);
    toolTip += u"<br>"_s;
    toolTip += u"<i>"_s + i18nc("@info:tooltip period end time", "End:") + u"</i>"_s + QStringLiteral("&nbsp;");
    toolTip += QLocale().toString(period.end().toLocalTime(), QLocale::ShortFormat);
    toolTip += u"<br>"_s;
    toolTip += u"</qt>"_s;
    return toolTip;
}

#include "moc_freebusyganttproxymodel.cpp"
