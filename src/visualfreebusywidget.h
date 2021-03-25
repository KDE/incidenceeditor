/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QWidget>

namespace KGantt
{
class DateTimeGrid;
class GraphicsView;
}

namespace CalendarSupport
{
class FreeBusyItemModel;
}

class QComboBox;
class QTreeView;

namespace IncidenceEditorNG
{
class FreeBusyGanttProxyModel;
class RowController;

class VisualFreeBusyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualFreeBusyWidget(CalendarSupport::FreeBusyItemModel *model, int spacing = 8, QWidget *parent = nullptr);
    ~VisualFreeBusyWidget() override;

public Q_SLOTS:
    void slotUpdateIncidenceStartEnd(const QDateTime &, const QDateTime &);

Q_SIGNALS:
    void dateTimesChanged(const QDateTime &, const QDateTime &);
    void manualReload();

protected Q_SLOTS:
    void slotScaleChanged(int);
    void slotCenterOnStart();
    void slotZoomToTime();
    void slotPickDate();
    void showAttendeeStatusMenu();
    void slotIntervalColorRectangleMoved(const QDateTime &start, const QDateTime &end);

private:
    void splitterMoved();
    KGantt::GraphicsView *mGanttGraphicsView = nullptr;
    QTreeView *mLeftView = nullptr;
    RowController *mRowController = nullptr;
    KGantt::DateTimeGrid *mGanttGrid = nullptr;

    QComboBox *mScaleCombo = nullptr;
    FreeBusyGanttProxyModel *mModel = nullptr;

    QDateTime mDtStart, mDtEnd;
};
}
