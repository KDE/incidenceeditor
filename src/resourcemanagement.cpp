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

#include <Akonadi/FreeBusyManager>

#include <EventViews/AgendaView>

#include <KCalendarCore/Event>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <KWindowConfig>
#include <QColor>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QWindow>

using namespace IncidenceEditorNG;
namespace
{
static const char myResourceManagementConfigGroupName[] = "ResourceManagement";
}
class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    ~FreebusyViewCalendar() override = default;

    [[nodiscard]] bool isValid(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        return isValid(incidence->uid());
    }

    [[nodiscard]] bool isValid(const QString &incidenceIdentifier) const override
    {
        return incidenceIdentifier.startsWith(QLatin1StringView("fb-"));
    }

    [[nodiscard]] QString displayName(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        Q_UNUSED(incidence)
        return QStringLiteral("Freebusy");
    }

    [[nodiscard]] QColor resourceColor(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        bool ok = false;
        int status = incidence->customProperty(QStringLiteral("FREEBUSY").toLatin1(), QStringLiteral("STATUS").toLatin1()).toInt(&ok);

        if (!ok) {
            return {85, 85, 85};
        }

        switch (status) {
        case KCalendarCore::FreeBusyPeriod::Busy:
            return {255, 0, 0};
        case KCalendarCore::FreeBusyPeriod::BusyTentative:
        case KCalendarCore::FreeBusyPeriod::BusyUnavailable:
            return {255, 119, 0};
        case KCalendarCore::FreeBusyPeriod::Free:
            return {0, 255, 0};
        default:
            return {85, 85, 85};
        }
    }

    [[nodiscard]] QString iconForIncidence(const KCalendarCore::Incidence::Ptr &incidence) const override
    {
        Q_UNUSED(incidence)
        return {};
    }

    [[nodiscard]] KCalendarCore::Calendar::Ptr getCalendar() const override
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
    create(); // ensure a window is created
    windowHandle()->resize(QSize(600, 400));
    KConfigGroup group(KSharedConfig::openStateConfig(), QLatin1StringView(myResourceManagementConfigGroupName));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

void ResourceManagement::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), QLatin1StringView(myResourceManagementConfigGroupName));
    KWindowConfig::saveWindowSize(windowHandle(), group);
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

void ResourceManagement::showDetails(const KLDAPCore::LdapObject &obj, const KLDAPWidgets::LdapClient &client)
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
        if (key == QLatin1StringView("objectClass") || key == QLatin1StringView("email")) {
            continue;
        } else if (key == QLatin1StringView("owner")) {
            QStringList attrs;
            attrs << QStringLiteral("cn") << QStringLiteral("mail") << QStringLiteral("mobile") << QStringLiteral("telephoneNumber")
                  << QStringLiteral("kolabDescAttribute") << QStringLiteral("description");
            mOwnerItem = ResourceItem::Ptr(new ResourceItem(KLDAPCore::LdapDN(QString::fromUtf8(it.value().at(0))), attrs, client));
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

    const KLDAPCore::LdapObject &obj = mOwnerItem->ldapObject();
    const KLDAPCore::LdapAttrMap &ldapAttrMap = obj.attributes();
    for (auto it = ldapAttrMap.cbegin(), end = ldapAttrMap.cend(); it != end; ++it) {
        const QString &key = it.key();
        if (key == QLatin1StringView("objectClass") || key == QLatin1StringView("owner") || key == QLatin1StringView("givenname")
            || key == QLatin1StringView("sn")) {
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

#include "moc_resourcemanagement.cpp"
