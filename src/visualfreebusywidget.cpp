/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "visualfreebusywidget.h"
#include "freebusyganttproxymodel.h"
#include <CalendarSupport/FreeBusyItemModel>

#include <KGantt/KGanttAbstractRowController>
#include <KGantt/KGanttDateTimeGrid>
#include <KGantt/KGanttGraphicsView>
#include <KGantt/KGanttView>

#include "incidenceeditor_debug.h"
#include <KLocalizedString>
#include <QComboBox>

#include <QHeaderView>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG
{
class RowController : public KGantt::AbstractRowController
{
private:
    static const int ROW_HEIGHT;
    QPointer<QAbstractItemModel> m_model;

public:
    RowController()
    {
        mRowHeight = 20;
    }

    void setModel(QAbstractItemModel *model)
    {
        m_model = model;
    }

    int headerHeight() const override
    {
        return 2 * mRowHeight + 10;
    }

    bool isRowVisible(const QModelIndex &) const override
    {
        return true;
    }

    bool isRowExpanded(const QModelIndex &) const override
    {
        return false;
    }

    KGantt::Span rowGeometry(const QModelIndex &idx) const override
    {
        return KGantt::Span(idx.row() * mRowHeight, mRowHeight);
    }

    int maximumItemHeight() const override
    {
        return mRowHeight / 2;
    }

    int totalHeight() const override
    {
        return m_model->rowCount() * mRowHeight;
    }

    QModelIndex indexAt(int height) const override
    {
        return m_model->index(height / mRowHeight, 0);
    }

    QModelIndex indexBelow(const QModelIndex &idx) const override
    {
        if (!idx.isValid()) {
            return QModelIndex();
        }
        return idx.model()->index(idx.row() + 1, idx.column(), idx.parent());
    }

    QModelIndex indexAbove(const QModelIndex &idx) const override
    {
        if (!idx.isValid()) {
            return QModelIndex();
        }
        return idx.model()->index(idx.row() - 1, idx.column(), idx.parent());
    }

    void setRowHeight(int height)
    {
        mRowHeight = height;
    }

private:
    int mRowHeight;
};

class GanttHeaderView : public QHeaderView
{
public:
    explicit GanttHeaderView(QWidget *parent = nullptr)
        : QHeaderView(Qt::Horizontal, parent)
    {
    }

    QSize sizeHint() const override
    {
        QSize s = QHeaderView::sizeHint();
        s.rheight() *= 2;
        return s;
    }
};
}

VisualFreeBusyWidget::VisualFreeBusyWidget(CalendarSupport::FreeBusyItemModel *model, int spacing, QWidget *parent)
    : QWidget(parent)
{
    auto topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(spacing);

    // The control panel for the gantt widget
    QBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(topLayout->spacing());
    topLayout->addItem(controlLayout);

    auto label = new QLabel(i18nc("@label", "Scale: "), this);
    controlLayout->addWidget(label);

    mScaleCombo = new QComboBox(this);
    mScaleCombo->setToolTip(i18nc("@info:tooltip", "Set the Gantt chart zoom level"));
    mScaleCombo->setWhatsThis(xi18nc("@info:whatsthis",
                                     "Select the Gantt chart zoom level from one of the following:<nl/>"
                                     "'Hour' shows a range of several hours,<nl/>"
                                     "'Day' shows a range of a few days,<nl/>"
                                     "'Week' shows a range of a few months,<nl/>"
                                     "and 'Month' shows a range of a few years,<nl/>"
                                     "while 'Automatic' selects the range most "
                                     "appropriate for the current event or to-do."));
    mScaleCombo->addItem(i18nc("@item:inlistbox range in hours", "Hour"), QVariant::fromValue<int>(KGantt::DateTimeGrid::ScaleHour));
    mScaleCombo->addItem(i18nc("@item:inlistbox range in days", "Day"), QVariant::fromValue<int>(KGantt::DateTimeGrid::ScaleDay));
    mScaleCombo->addItem(i18nc("@item:inlistbox range in weeks", "Week"), QVariant::fromValue<int>(KGantt::DateTimeGrid::ScaleWeek));
    mScaleCombo->addItem(i18nc("@item:inlistbox range in months", "Month"), QVariant::fromValue<int>(KGantt::DateTimeGrid::ScaleMonth));
    mScaleCombo->addItem(i18nc("@item:inlistbox range is computed automatically", "Automatic"), QVariant::fromValue<int>(KGantt::DateTimeGrid::ScaleAuto));
    mScaleCombo->setCurrentIndex(0); // start with "hour"
    connect(mScaleCombo, qOverload<int>(&QComboBox::activated), this, &VisualFreeBusyWidget::slotScaleChanged);
    controlLayout->addWidget(mScaleCombo);

    auto button = new QPushButton(i18nc("@action:button", "Center on Start"), this);
    button->setToolTip(i18nc("@info:tooltip", "Center the Gantt chart on the event start date and time"));
    button->setWhatsThis(i18nc("@info:whatsthis",
                               "Click this button to center the Gantt chart on the start "
                               "time and day of this event."));
    connect(button, &QPushButton::clicked, this, &VisualFreeBusyWidget::slotCenterOnStart);
    controlLayout->addWidget(button);

    controlLayout->addStretch(1);

    button = new QPushButton(i18nc("@action:button", "Pick Date"), this);
    button->setToolTip(i18nc("@info:tooltip",
                             "Move the event to a date and time when all "
                             "attendees are available"));
    button->setWhatsThis(i18nc("@info:whatsthis",
                               "Click this button to move the event to a date "
                               "and time when all the attendees have time "
                               "available in their Free/Busy lists."));
    button->setEnabled(false);
    connect(button, &QPushButton::clicked, this, &VisualFreeBusyWidget::slotPickDate);
    controlLayout->addWidget(button);

    controlLayout->addStretch(1);

    button = new QPushButton(i18nc("@action:button reload freebusy data", "Reload"), this);
    button->setToolTip(i18nc("@info:tooltip", "Reload Free/Busy data for all attendees"));
    button->setWhatsThis(i18nc("@info:whatsthis",
                               "Pressing this button will cause the Free/Busy data for all "
                               "attendees to be reloaded from their corresponding servers."));
    controlLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &VisualFreeBusyWidget::manualReload);

    auto splitter = new QSplitter(Qt::Horizontal, this);
    connect(splitter, &QSplitter::splitterMoved, this, &VisualFreeBusyWidget::splitterMoved);
    mLeftView = new QTreeView(this);
    mLeftView->setModel(model);
    mLeftView->setHeader(new GanttHeaderView);
    mLeftView->header()->setStretchLastSection(true);
    mLeftView->setToolTip(i18nc("@info:tooltip", "Shows the tree list of all data"));
    mLeftView->setWhatsThis(i18nc("@info:whatsthis", "Shows the tree list of all data"));
    mLeftView->setRootIsDecorated(false);
    mLeftView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mLeftView->setContextMenuPolicy(Qt::CustomContextMenu);
    mGanttGraphicsView = new KGantt::GraphicsView(this);
    mGanttGraphicsView->setObjectName(QStringLiteral("mGanttGraphicsView"));
    mGanttGraphicsView->setToolTip(i18nc("@info:tooltip", "Shows the Free/Busy status of all attendees"));
    mGanttGraphicsView->setWhatsThis(i18nc("@info:whatsthis",
                                           "Shows the Free/Busy status of all attendees. "
                                           "Double-clicking on an attendee's entry in the "
                                           "list will allow you to enter the location of "
                                           "their Free/Busy Information."));
    mModel = new FreeBusyGanttProxyModel(this);
    mModel->setSourceModel(model);

    mRowController = new RowController;
    mRowController->setRowHeight(fontMetrics().height()); // TODO: detect

    mRowController->setModel(mModel);
    mGanttGraphicsView->setRowController(mRowController);

    mGanttGrid = new KGantt::DateTimeGrid;
    mGanttGrid->setScale(KGantt::DateTimeGrid::ScaleHour);
    mGanttGrid->setDayWidth(800);
    mGanttGrid->setRowSeparators(true);
    mGanttGraphicsView->setGrid(mGanttGrid);
    mGanttGraphicsView->setModel(mModel);
    mGanttGraphicsView->viewport()->setFixedWidth(800 * 30);

    splitter->addWidget(mLeftView);
    splitter->addWidget(mGanttGraphicsView);

    topLayout->addWidget(splitter);
    topLayout->setStretchFactor(splitter, 100);

    // Initially, show 15 days back and forth
    // set start to even hours, i.e. to 12:AM 0 Min 0 Sec
    const QDateTime horizonStart = QDateTime(QDateTime::currentDateTime().addDays(-15).date().startOfDay());
    mGanttGrid->setStartDateTime(horizonStart);

    connect(mLeftView, &QTreeView::customContextMenuRequested, this, &VisualFreeBusyWidget::showAttendeeStatusMenu);
}

VisualFreeBusyWidget::~VisualFreeBusyWidget()
{
}

void VisualFreeBusyWidget::showAttendeeStatusMenu()
{
}

void VisualFreeBusyWidget::slotCenterOnStart()
{
    auto grid = static_cast<KGantt::DateTimeGrid *>(mGanttGraphicsView->grid());
    int daysTo = grid->startDateTime().daysTo(mDtStart);
    mGanttGraphicsView->horizontalScrollBar()->setValue(daysTo * 800);
}

void VisualFreeBusyWidget::slotIntervalColorRectangleMoved(const QDateTime &start, const QDateTime &end)
{
    mDtStart = start;
    mDtEnd = end;
    Q_EMIT dateTimesChanged(start, end);
}

/*!
  This slot is called when the user clicks the "Pick a date" button.
*/
void VisualFreeBusyWidget::slotPickDate()
{
}

void VisualFreeBusyWidget::slotScaleChanged(int newScale)
{
    const QVariant var = mScaleCombo->itemData(newScale);
    Q_ASSERT(var.isValid());

    int value = var.toInt();
    mGanttGrid->setScale((KGantt::DateTimeGrid::Scale)value);
}

void VisualFreeBusyWidget::slotUpdateIncidenceStartEnd(const QDateTime &dtFrom, const QDateTime &dtTo)
{
    mDtStart = dtFrom;
    mDtEnd = dtTo;
    QDateTime horizonStart = QDateTime(dtFrom.addDays(-15).date().startOfDay());

    auto grid = static_cast<KGantt::DateTimeGrid *>(mGanttGraphicsView->grid());
    grid->setStartDateTime(horizonStart);
    slotCenterOnStart();
    mGanttGrid->setStartDateTime(horizonStart);
}

void VisualFreeBusyWidget::slotZoomToTime()
{
#if 0
    mGanttGraphicsView->zoomToFit();
#else
    qCDebug(INCIDENCEEDITOR_LOG) << "Disabled code, port to KDGantt2";
#endif
}

void VisualFreeBusyWidget::splitterMoved()
{
}
