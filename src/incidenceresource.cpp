/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "incidenceresource.h"
#include "attendeecomboboxdelegate.h"
#include "attendeelineeditdelegate.h"
#include "incidencedatetime.h"
#include "resourcemanagement.h"
#include "resourcemodel.h"

#include "ui_dialogdesktop.h"

#include <KDescendantsProxyModel>
#include <KEmailAddress>
#include <QCompleter>

using namespace IncidenceEditorNG;

class SwitchRoleProxy : public QSortFilterProxyModel
{
public:
    explicit SwitchRoleProxy(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        QVariant d;
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            d = QSortFilterProxyModel::data(index, ResourceModel::FullName);
            return d;
        }
        d = QSortFilterProxyModel::data(index, role);
        return d;
    }
};

IncidenceResource::IncidenceResource(IncidenceAttendee *ieAttendee, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(nullptr)
    , mUi(ui)
    , dataModel(ieAttendee->dataModel())
    , mDateTime(dateTime)
    , resourceDialog(new ResourceManagement())
{
    setObjectName(QStringLiteral("IncidenceResource"));
    connect(resourceDialog, &ResourceManagement::accepted, this, &IncidenceResource::dialogOkPressed);

    connect(mDateTime, &IncidenceDateTime::startDateChanged, this, &IncidenceResource::slotDateChanged);
    connect(mDateTime, &IncidenceDateTime::endDateChanged, this, &IncidenceResource::slotDateChanged);

    QStringList attrs;
    attrs << QStringLiteral("cn") << QStringLiteral("mail");

    completer = new QCompleter(this);
    auto model = new ResourceModel(attrs, this);

    auto proxyModel = new KDescendantsProxyModel(this);
    proxyModel->setSourceModel(model);
    auto proxyModel2 = new SwitchRoleProxy(this);
    proxyModel2->setSourceModel(proxyModel);

    completer->setModel(proxyModel2);
    completer->setCompletionRole(ResourceModel::FullName);
    completer->setWrapAround(false);
    mUi->mNewResource->setCompleter(completer);

    auto attendeeDelegate = new AttendeeLineEditDelegate(this);

    auto filterProxyModel = new ResourceFilterProxyModel(this);
    filterProxyModel->setDynamicSortFilter(true);
    filterProxyModel->setSourceModel(dataModel);

    mUi->mResourcesTable->setModel(filterProxyModel);
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Role, ieAttendee->roleDelegate());
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::FullName, attendeeDelegate);
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Status, ieAttendee->stateDelegate());
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Response, ieAttendee->responseDelegate());

    connect(mUi->mFindResourcesButton, &QPushButton::clicked, this, &IncidenceResource::findResources);
    connect(mUi->mBookResourceButton, &QPushButton::clicked, this, &IncidenceResource::bookResource);
    connect(filterProxyModel, &ResourceFilterProxyModel::layoutChanged, this, &IncidenceResource::layoutChanged);
    connect(filterProxyModel, &ResourceFilterProxyModel::layoutChanged, this, &IncidenceResource::updateCount);
    connect(filterProxyModel, &ResourceFilterProxyModel::rowsInserted, this, &IncidenceResource::updateCount);
    connect(filterProxyModel, &ResourceFilterProxyModel::rowsRemoved, this, &IncidenceResource::updateCount);
    // only update when FullName is changed
    connect(filterProxyModel, &ResourceFilterProxyModel::dataChanged, this, &IncidenceResource::updateCount);
}

IncidenceResource::~IncidenceResource()
{
    delete resourceDialog;
}

void IncidenceResource::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence)
    slotDateChanged();
}

void IncidenceResource::slotDateChanged()
{
    resourceDialog->slotDateChanged(mDateTime->startDate(), mDateTime->endDate());
}

void IncidenceResource::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence)
    // all logic inside IncidenceAtendee (using same model)
}

bool IncidenceResource::isDirty() const
{
    // all logic inside IncidenceAtendee (using same model)
    return false;
}

void IncidenceResource::bookResource()
{
    if (mUi->mNewResource->text().trimmed().isEmpty()) {
        return;
    }
    QString name, email;
    KEmailAddress::extractEmailAddressAndName(mUi->mNewResource->text(), email, name);
    KCalendarCore::Attendee attendee(name, email);
    attendee.setCuType(KCalendarCore::Attendee::Resource);
    dataModel->insertAttendee(dataModel->rowCount(), attendee);
}

void IncidenceResource::findResources()
{
    resourceDialog->show();
}

void IncidenceResource::dialogOkPressed()
{
    ResourceItem::Ptr item = resourceDialog->selectedItem();
    if (item) {
        const QString name = QString::fromLatin1(item->ldapObject().value(QStringLiteral("cn")));
        const QString email = QString::fromLatin1(item->ldapObject().value(QStringLiteral("mail")));
        KCalendarCore::Attendee attendee(name, email);
        attendee.setCuType(KCalendarCore::Attendee::Resource);
        dataModel->insertAttendee(dataModel->rowCount(), attendee);
    }
}

void IncidenceResource::layoutChanged()
{
    QHeaderView *headerView = mUi->mResourcesTable->horizontalHeader();
    headerView->setSectionHidden(AttendeeTableModel::CuType, true);
    headerView->setSectionHidden(AttendeeTableModel::Name, true);
    headerView->setSectionHidden(AttendeeTableModel::Email, true);
    headerView->setSectionResizeMode(AttendeeTableModel::Role, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(AttendeeTableModel::FullName, QHeaderView::Stretch);
    headerView->setSectionResizeMode(AttendeeTableModel::Available, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(AttendeeTableModel::Status, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(AttendeeTableModel::Response, QHeaderView::ResizeToContents);
}

void IncidenceResource::updateCount()
{
    Q_EMIT resourceCountChanged(resourceCount());
}

int IncidenceResource::resourceCount() const
{
    int c = 0;
    QModelIndex index;
    QAbstractItemModel *model = mUi->mResourcesTable->model();
    if (!model) {
        return 0;
    }
    const int nbRow = model->rowCount(QModelIndex());
    for (int i = 0; i < nbRow; ++i) {
        index = model->index(i, AttendeeTableModel::FullName);
        if (!model->data(index).toString().isEmpty()) {
            ++c;
        }
    }
    return c;
}
