/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "resourcemanagement.h"
#include "ldaputils.h"
#include "resourcemodel.h"
#include "ui_resourcemanagement.h"
#include <CalendarSupport/FreeBusyItem>

#include "freebusyganttproxymodel.h"

#include <Akonadi/Calendar/FreeBusyManager>

#include <EventViews/AgendaView>

#include <KCalendarCore/Event>
#include <KCalendarCore/MemoryCalendar>

#include <KGantt/KGanttGraphicsView>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QColor>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QStringList>

using namespace IncidenceEditorNG;

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    ~FreebusyViewCalendar() override
    {
    }

    bool isValid(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        return isValid(incidence->uid());
    }

    bool isValid(const QString &incidenceIdentifier) const override
    {
        return incidenceIdentifier.startsWith(QLatin1String("fb-"));
    }

    QString displayName(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        Q_UNUSED(incidence)
        return QStringLiteral("Freebusy");
    }

    QColor resourceColor(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        bool ok = false;
        int status = incidence->customProperty(QStringLiteral("FREEBUSY").toLatin1(), QStringLiteral("STATUS").toLatin1()).toInt(&ok);

        if (!ok) {
            return QColor(85, 85, 85);
        }

        switch (status) {
        case KCalendarCore::FreeBusyPeriod::Busy:
            return QColor(255, 0, 0);
        case KCalendarCore::FreeBusyPeriod::BusyTentative:
        case KCalendarCore::FreeBusyPeriod::BusyUnavailable:
            return QColor(255, 119, 0);
        case KCalendarCore::FreeBusyPeriod::Free:
            return QColor(0, 255, 0);
        default:
            return QColor(85, 85, 85);
        }
    }

    QString iconForIncidence(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        Q_UNUSED(incidence)
        return QString();
    }

    KCalendarCore::Calendar::Ptr getCalendar() const override
    {
        return mCalendar;
    }

    KCalendarCore::Calendar::Ptr mCalendar;
};

ResourceManagement::ResourceManagement(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Resource Management"));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18nc("@action:button add resource to attendeelist", "Book resource"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ResourceManagement::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ResourceManagement::reject);

    mUi = new Ui_resourceManagement;

    auto w = new QWidget(this);
    mUi->setupUi(w);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(w);

    mainLayout->addWidget(buttonBox);

    mModel = new CalendarSupport::FreeBusyItemModel(this);
    mFreebusyCalendar.setModel(mModel);

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false, false);

    auto fbCalendar = new FreebusyViewCalendar();
    fbCalendar->mCalendar = mFreebusyCalendar.calendar();
    mFbCalendar = EventViews::ViewCalendar::Ptr(fbCalendar);
    mAgendaView->addCalendar(mFbCalendar);

    mUi->resourceCalender->addWidget(mAgendaView);

    QStringList attrs;
    attrs << QStringLiteral("cn") << QStringLiteral("mail") << QStringLiteral("owner") << QStringLiteral("givenname") << QStringLiteral("sn")
          << QStringLiteral("kolabDescAttribute") << QStringLiteral("description");
    auto resourcemodel = new ResourceModel(attrs, this);
    mUi->treeResults->setModel(resourcemodel);

    // This doesn't work till now :(-> that's why i use the click signal
    mUi->treeResults->setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel = mUi->treeResults->selectionModel();

    connect(mUi->resourceSearch, &QLineEdit::textChanged, this, &ResourceManagement::slotStartSearch);

    connect(mUi->treeResults, &QTreeView::clicked, this, &ResourceManagement::slotShowDetails);

    connect(resourcemodel, &ResourceModel::layoutChanged, this, &ResourceManagement::slotLayoutChanged);
    readConfig();
}

ResourceManagement::~ResourceManagement()
{
    writeConfig();
    delete mModel;
    delete mUi;
}

void ResourceManagement::readConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), "ResourceManagement");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void ResourceManagement::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), "ResourceManagement");
    group.writeEntry("Size", size());
    group.sync();
}

ResourceItem::Ptr ResourceManagement::selectedItem() const
{
    return mSelectedItem;
}

void ResourceManagement::slotStartSearch(const QString &text)
{
    (static_cast<ResourceModel *>(mUi->treeResults->model()))->startSearch(text);
}

void ResourceManagement::slotShowDetails(const QModelIndex &current)
{
    auto item = current.model()->data(current, ResourceModel::Resource).value<ResourceItem::Ptr>();
    mSelectedItem = item;
    showDetails(item->ldapObject(), item->ldapClient());
}

void ResourceManagement::showDetails(const KLDAP::LdapObject &obj, const KLDAP::LdapClient &client)
{
    // Clean up formDetails
    QLayoutItem *child = nullptr;
    while ((child = mUi->formDetails->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    mUi->groupOwner->setHidden(true);

    // Fill formDetails with data
    for (auto it = obj.attributes().cbegin(), end = obj.attributes().cbegin(); it != end; ++it) {
        const QString &key = it.key();
        if (key == QLatin1String("objectClass") || key == QLatin1String("email")) {
            continue;
        } else if (key == QLatin1String("owner")) {
            QStringList attrs;
            attrs << QStringLiteral("cn") << QStringLiteral("mail") << QStringLiteral("mobile") << QStringLiteral("telephoneNumber")
                  << QStringLiteral("kolabDescAttribute") << QStringLiteral("description");
            mOwnerItem = ResourceItem::Ptr(new ResourceItem(KLDAP::LdapDN(QString::fromUtf8(it.value().at(0))), attrs, client));
            connect(mOwnerItem.data(), &ResourceItem::searchFinished, this, &ResourceManagement::slotOwnerSearchFinished);
            mOwnerItem->startSearch();
            continue;
        }
        QStringList list;
        const QList<QByteArray> values = it.value();
        list.reserve(values.count());
        for (const QByteArray &value : values) {
            list << QString::fromUtf8(value);
        }
        mUi->formDetails->addRow(translateLDAPAttributeForDisplay(key), new QLabel(list.join(QLatin1Char('\n'))));
    }

    QString name = QString::fromUtf8(obj.attributes().value(QStringLiteral("cn"))[0]);
    QString email = QString::fromUtf8(obj.attributes().value(QStringLiteral("mail"))[0]);
    KCalendarCore::Attendee attendee(name, email);
    CalendarSupport::FreeBusyItem::Ptr freebusy(new CalendarSupport::FreeBusyItem(attendee, this));
    mModel->clear();
    mModel->addItem(freebusy);
}

void ResourceManagement::slotLayoutChanged()
{
    const int columnCount = mUi->treeResults->model()->columnCount(QModelIndex());
    for (int i = 1; i < columnCount; ++i) {
        mUi->treeResults->setColumnHidden(i, true);
    }
}

void ResourceManagement::slotOwnerSearchFinished()
{
    // Clean up formDetails
    QLayoutItem *child = nullptr;
    while ((child = mUi->formOwner->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    mUi->groupOwner->setHidden(false);

    const KLDAP::LdapObject &obj = mOwnerItem->ldapObject();
    const KLDAP::LdapAttrMap &ldapAttrMap = obj.attributes();
    for (auto it = ldapAttrMap.cbegin(), end = ldapAttrMap.cend(); it != end; ++it) {
        const QString &key = it.key();
        if (key == QLatin1String("objectClass") || key == QLatin1String("owner") || key == QLatin1String("givenname") || key == QLatin1String("sn")) {
            continue;
        }
        QStringList list;
        const QList<QByteArray> values = it.value();
        list.reserve(values.count());
        for (const QByteArray &value : values) {
            list << QString::fromUtf8(value);
        }
        mUi->formOwner->addRow(translateLDAPAttributeForDisplay(key), new QLabel(list.join(QLatin1Char('\n'))));
    }
}

void ResourceManagement::slotDateChanged(const QDate &start, const QDate &end)
{
    if (start.daysTo(end) < 7) {
        mAgendaView->showDates(start, start.addDays(7));
    }
    mAgendaView->showDates(start, end);
}
