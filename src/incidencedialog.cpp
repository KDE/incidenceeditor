/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileCopyrightText: 2012 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencedialog.h"
using namespace Qt::Literals::StringLiterals;

#include "combinedincidenceeditor.h"
#include "editorconfig.h"
#include "incidencealarm.h"
#include "incidenceattachment.h"
#include "incidenceattendee.h"
#include "incidencecategories.h"
#include "incidencecompletionpriority.h"
#include "incidencedatetime.h"
#include "incidencedescription.h"
#include "incidenceeditor_debug.h"
#include "incidencerecurrence.h"
#include "incidenceresource.h"
#include "incidencesecrecy.h"
#include "incidencewhatwhere.h"
#include "templatemanagementdialog.h"
#include "ui_dialogdesktop.h"

#include "incidenceeditorsettings.h"

#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <Akonadi/CalendarUtils>
#include <Akonadi/CollectionComboBox>
#include <Akonadi/ETMCalendar>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>

#include <KCalUtils/Stringify>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include <KMessageBox>
#include <KSharedConfig>

#include <KWindowConfig>
#include <QCloseEvent>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>
#include <QTimeZone>
#include <QWindow>

using namespace IncidenceEditorNG;
namespace
{
const char myIncidenceDialogConfigGroupName[] = "IncidenceDialog";

IncidenceEditorNG::EditorItemManager::ItipPrivacyFlags toItemManagerFlags(bool sign, bool encrypt)
{
    IncidenceEditorNG::EditorItemManager::ItipPrivacyFlags flags;
    flags.setFlag(IncidenceEditorNG::EditorItemManager::ItipPrivacySign, sign);
    flags.setFlag(IncidenceEditorNG::EditorItemManager::ItipPrivacyEncrypt, encrypt);
    return flags;
}
}
namespace IncidenceEditorNG
{
enum Tabs {
    GeneralTab = 0,
    AttendeesTab,
    ResourcesTab,
    AlarmsTab,
    RecurrenceTab,
    AttachmentsTab
};

class IncidenceDialogPrivate : public ItemEditorUi
{
    IncidenceDialog *q_ptr;
    Q_DECLARE_PUBLIC(IncidenceDialog)

public:
    Ui::EventOrTodoDesktop *mUi = nullptr;
    Akonadi::CollectionComboBox *mCalSelector = nullptr;
    bool mCloseOnSave = false;

    EditorItemManager *mItemManager = nullptr;
    CombinedIncidenceEditor *mEditor = nullptr;
    IncidenceDateTime *mIeDateTime = nullptr;
    IncidenceAttendee *mIeAttendee = nullptr;
    IncidenceRecurrence *mIeRecurrence = nullptr;
    IncidenceResource *mIeResource = nullptr;
    bool mInitiallyDirty = false;
    Akonadi::Item mItem;
    [[nodiscard]] QString typeToString(const int type) const;

    IncidenceDialogPrivate(Akonadi::IncidenceChanger *changer, IncidenceDialog *qq);
    ~IncidenceDialogPrivate() override;

    /// General methods
    void handleAlarmCountChange(int newCount);
    void handleRecurrenceChange(IncidenceEditorNG::RecurrenceType type);
    void loadTemplate(const QString &templateName);
    void manageTemplates();
    void saveTemplate(const QString &templateName);
    void storeTemplatesInConfig(const QStringList &templateNames);
    void updateAttachmentCount(int newCount);
    void updateAttendeeCount(int newCount);
    void updateResourceCount(int newCount);
    void updateButtonStatus(bool isDirty);
    void showMessage(const QString &text, KMessageWidget::MessageType type);
    void slotInvalidCollection();
    void setCalendarCollection(const Akonadi::Collection &collection);

    /// ItemEditorUi methods
    [[nodiscard]] bool containsPayloadIdentifiers(const QSet<QByteArray> &partIdentifiers) const override;
    void handleItemSaveFinish(EditorItemManager::SaveAction);
    void handleItemSaveFail(EditorItemManager::SaveAction, const QString &errorMessage);
    [[nodiscard]] bool hasSupportedPayload(const Akonadi::Item &item) const override;
    [[nodiscard]] bool isDirty() const override;
    [[nodiscard]] bool isValid() const override;
    void load(const Akonadi::Item &item) override;
    Akonadi::Item save(const Akonadi::Item &item) override;
    [[nodiscard]] Akonadi::Collection selectedCollection() const override;

    void reject(RejectReason reason, const QString &errorMessage = QString()) override;

private:
    // disable copy ctor
    IncidenceDialogPrivate(const IncidenceDialogPrivate &) = delete;
    IncidenceDialogPrivate &operator=(const IncidenceDialogPrivate &) = delete;
};
}

IncidenceDialogPrivate::IncidenceDialogPrivate(Akonadi::IncidenceChanger *changer, IncidenceDialog *qq)
    : q_ptr(qq)
    , mUi(new Ui::EventOrTodoDesktop)
    , mCalSelector(new Akonadi::CollectionComboBox(changer ? changer->entityTreeModel() : nullptr))
    , mItemManager(new EditorItemManager(this, changer))
    , mEditor(new CombinedIncidenceEditor(qq))
{
    Q_Q(IncidenceDialog);
    mUi->setupUi(q);
    mUi->mMessageWidget->hide();
    auto layout = new QGridLayout(mUi->mCalSelectorPlaceHolder);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mCalSelector);
    mCalSelector->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mUi->label->setBuddy(mCalSelector);
    q->connect(mCalSelector, &Akonadi::CollectionComboBox::currentChanged, q, &IncidenceDialog::handleSelectedCollectionChange);

    // Now instantiate the logic of the dialog. These editors update the ui, validate
    // fields and load/store incidences in the ui.
    auto ieGeneral = new IncidenceWhatWhere(mUi);
    mEditor->combine(ieGeneral);

    auto ieCategories = new IncidenceCategories(mUi);
    mEditor->combine(ieCategories);

    mIeDateTime = new IncidenceDateTime(mUi);
    mEditor->combine(mIeDateTime);

    auto ieCompletionPriority = new IncidenceCompletionPriority(mUi);
    mEditor->combine(ieCompletionPriority);

    auto ieDescription = new IncidenceDescription(mUi);
    mEditor->combine(ieDescription);

    auto ieAlarm = new IncidenceAlarm(mIeDateTime, mUi);
    mEditor->combine(ieAlarm);

    auto ieAttachments = new IncidenceAttachment(mUi);
    mEditor->combine(ieAttachments);

    mIeRecurrence = new IncidenceRecurrence(mIeDateTime, mUi);
    mEditor->combine(mIeRecurrence);

    auto ieSecrecy = new IncidenceSecrecy(mUi);
    mEditor->combine(ieSecrecy);

    mIeAttendee = new IncidenceAttendee(qq, mIeDateTime, mUi);
    mIeAttendee->setParent(qq);
    mEditor->combine(mIeAttendee);

    mIeResource = new IncidenceResource(mIeAttendee, mIeDateTime, mUi);
    mEditor->combine(mIeResource);

    // Set the default collection
    const Akonadi::Collection col(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());
    if (col.isValid()) {
        setCalendarCollection(col);
    }

    q->connect(mEditor, &CombinedIncidenceEditor::showMessage, q, [this](const QString &reason, KMessageWidget::MessageType msgType) {
        showMessage(reason, msgType);
    });
    q->connect(mEditor, &IncidenceEditor::dirtyStatusChanged, q, [this](bool isDirty) {
        updateButtonStatus(isDirty);
    });
    q->connect(mItemManager, &EditorItemManager::itemSaveFinished, q, [this](EditorItemManager::SaveAction action) {
        handleItemSaveFinish(action);
    });
    q->connect(mItemManager, &EditorItemManager::itemSaveFailed, q, [this](EditorItemManager::SaveAction action, const QString &message) {
        handleItemSaveFail(action, message);
    });
    q->connect(ieAlarm, &IncidenceAlarm::alarmCountChanged, q, [this](int newCount) {
        handleAlarmCountChange(newCount);
    });
    q->connect(mIeRecurrence, &IncidenceRecurrence::recurrenceChanged, q, [this](IncidenceEditorNG::RecurrenceType type) {
        handleRecurrenceChange(type);
    });
    q->connect(ieAttachments, &IncidenceAttachment::attachmentCountChanged, q, [this](int newCount) {
        updateAttachmentCount(newCount);
    });
    q->connect(mIeAttendee, &IncidenceAttendee::attendeeCountChanged, q, [this](int count) {
        updateAttendeeCount(count);
    });
    q->connect(mIeResource, &IncidenceResource::resourceCountChanged, q, [this](int count) {
        updateResourceCount(count);
    });
}

IncidenceDialogPrivate::~IncidenceDialogPrivate()
{
    delete mItemManager;
    delete mEditor;
    delete mUi;
}

void IncidenceDialogPrivate::slotInvalidCollection()
{
    showMessage(i18n("Select a valid collection first."), KMessageWidget::Warning);
}

void IncidenceDialogPrivate::setCalendarCollection(const Akonadi::Collection &collection)
{
    if (collection.isValid()) {
        mCalSelector->setDefaultCollection(collection);
    } else {
        mCalSelector->setCurrentIndex(0);
    }
}

void IncidenceDialogPrivate::showMessage(const QString &text, KMessageWidget::MessageType type)
{
    mUi->mMessageWidget->setText(text);
    mUi->mMessageWidget->setMessageType(type);
    mUi->mMessageWidget->show();
}

void IncidenceDialogPrivate::handleAlarmCountChange(int newCount)
{
    QString tabText;
    if (newCount > 0) {
        tabText = i18nc("@title:tab Tab to configure the reminders of an event or todo", "Reminder (%1)", newCount);
    } else {
        tabText = i18nc("@title:tab Tab to configure the reminders of an event or todo", "Reminder");
    }

    mUi->mTabWidget->setTabText(AlarmsTab, tabText);
}

void IncidenceDialogPrivate::handleRecurrenceChange(IncidenceEditorNG::RecurrenceType type)
{
    QString tabText = i18nc("@title:tab Tab to configure the recurrence of an event or todo", "Rec&urrence");

    // Keep this numbers in sync with the items in mUi->mRecurrenceTypeCombo. I
    // tried adding an enum to IncidenceRecurrence but for whatever reason I could
    // Qt not play nice with namespaced enums in signal/slot connections.
    // Anyways, I don't expect these values to change.
    switch (type) {
    case RecurrenceTypeNone:
        break;
    case RecurrenceTypeDaily:
        tabText += i18nc("@title:tab Daily recurring event, capital first letter only", " (D)");
        break;
    case RecurrenceTypeWeekly:
        tabText += i18nc("@title:tab Weekly recurring event, capital first letter only", " (W)");
        break;
    case RecurrenceTypeMonthly:
        tabText += i18nc("@title:tab Monthly recurring event, capital first letter only", " (M)");
        break;
    case RecurrenceTypeYearly:
        tabText += i18nc("@title:tab Yearly recurring event, capital first letter only", " (Y)");
        break;
    case RecurrenceTypeException:
        tabText += i18nc("@title:tab Exception to a recurring event, capital first letter only", " (E)");
        break;
    default:
        Q_ASSERT_X(false, "handleRecurrenceChange", "Fix your program");
    }

    mUi->mTabWidget->setTabText(RecurrenceTab, tabText);
}

QString IncidenceDialogPrivate::typeToString(const int type) const
{
    // Do not translate.
    switch (type) {
    case KCalendarCore::Incidence::TypeEvent:
        return u"Event"_s;
    case KCalendarCore::Incidence::TypeTodo:
        return u"Todo"_s;
    case KCalendarCore::Incidence::TypeJournal:
        return u"Journal"_s;
    default:
        return u"Unknown"_s;
    }
}

void IncidenceDialogPrivate::loadTemplate(const QString &templateName)
{
    Q_Q(IncidenceDialog);

    KCalendarCore::MemoryCalendar::Ptr const cal(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));

    const QString fileName =
        QStandardPaths::locate(QStandardPaths::GenericDataLocation, u"/korganizer/templates/"_s + typeToString(mEditor->type()) + u'/' + templateName);

    if (fileName.isEmpty()) {
        KMessageBox::error(q, i18nc("@info", "Unable to find template '%1'.", templateName));
        return;
    }

    KCalendarCore::ICalFormat format;
    if (!format.load(cal, fileName)) {
        KMessageBox::error(q, i18nc("@info", "Error loading template file '%1'.", fileName));
        return;
    }

    KCalendarCore::Incidence::List incidences = cal->incidences();
    if (incidences.isEmpty()) {
        KMessageBox::error(q, i18nc("@info", "Template does not contain a valid incidence."));
        return;
    }

    mIeDateTime->setActiveDate(QDate());
    KCalendarCore::Incidence::Ptr const newInc = KCalendarCore::Incidence::Ptr(incidences.first()->clone());
    newInc->setUid(KCalendarCore::CalFormat::createUniqueId());

    // We add a custom property so that some fields aren't loaded, dates for example
    newInc->setCustomProperty(QByteArray("kdepim"), "isTemplate", u"true"_s);
    mEditor->load(newInc);
    newInc->removeCustomProperty(QByteArray(), "isTemplate");
}

void IncidenceDialogPrivate::manageTemplates()
{
    Q_Q(IncidenceDialog);

    const QStringList &templates = IncidenceEditorNG::EditorConfig::instance()->templates(mEditor->type());

    QPointer<IncidenceEditorNG::TemplateManagementDialog> const dialog(
        new IncidenceEditorNG::TemplateManagementDialog(q, templates, KCalUtils::Stringify::incidenceType(mEditor->type())));

    q->connect(dialog, &TemplateManagementDialog::loadTemplate, q, [this](const QString &templateName) {
        loadTemplate(templateName);
    });
    q->connect(dialog, &TemplateManagementDialog::templatesChanged, q, [this](const QStringList &templates) {
        storeTemplatesInConfig(templates);
    });
    q->connect(dialog, &TemplateManagementDialog::saveTemplate, q, [this](const QString &templateName) {
        saveTemplate(templateName);
    });
    dialog->exec();
    delete dialog;
}

void IncidenceDialogPrivate::saveTemplate(const QString &templateName)
{
    Q_ASSERT(!templateName.isEmpty());

    KCalendarCore::MemoryCalendar::Ptr const cal(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));

    switch (mEditor->type()) {
    case KCalendarCore::Incidence::TypeEvent: {
        KCalendarCore::Event::Ptr const event(new KCalendarCore::Event());
        mEditor->save(event);
        cal->addEvent(KCalendarCore::Event::Ptr(event->clone()));
        break;
    }
    case KCalendarCore::Incidence::TypeTodo: {
        KCalendarCore::Todo::Ptr const todo(new KCalendarCore::Todo);
        mEditor->save(todo);
        cal->addTodo(KCalendarCore::Todo::Ptr(todo->clone()));
        break;
    }
    case KCalendarCore::Incidence::TypeJournal: {
        KCalendarCore::Journal::Ptr const journal(new KCalendarCore::Journal);
        mEditor->save(journal);
        cal->addJournal(KCalendarCore::Journal::Ptr(journal->clone()));
        break;
    }
    default:
        Q_ASSERT_X(false, "saveTemplate", "Fix your program");
    }

    QString fileName =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"/korganizer/templates/"_s + typeToString(mEditor->type()) + u'/';
    QDir().mkpath(fileName);
    fileName += templateName;

    KCalendarCore::ICalFormat format;
    format.save(cal, fileName);
}

void IncidenceDialogPrivate::storeTemplatesInConfig(const QStringList &templateNames)
{
    // I find this somewhat broken. templates() returns a reference, maybe it should
    // be changed by adding a setTemplates method.
    const QStringList origTemplates = IncidenceEditorNG::EditorConfig::instance()->templates(mEditor->type());
    const QString defaultPath =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"korganizer/templates/"_s + typeToString(mEditor->type()) + u'/';
    QDir().mkpath(defaultPath);
    for (const QString &tmpl : origTemplates) {
        if (!templateNames.contains(tmpl)) {
            const QString fileName = defaultPath + tmpl;
            QFile file(fileName);
            if (file.exists()) {
                file.remove();
            }
        }
    }

    IncidenceEditorNG::EditorConfig::instance()->templates(mEditor->type()) = templateNames;
    IncidenceEditorNG::EditorConfig::instance()->config()->save();
}

void IncidenceDialogPrivate::updateAttachmentCount(int newCount)
{
    if (newCount > 0) {
        mUi->mTabWidget->setTabText(AttachmentsTab, i18nc("@title:tab Tab to modify attachments of an event or todo", "Attac&hments (%1)", newCount));
    } else {
        mUi->mTabWidget->setTabText(AttachmentsTab, i18nc("@title:tab Tab to modify attachments of an event or todo", "Attac&hments"));
    }
}

void IncidenceDialogPrivate::updateAttendeeCount(int newCount)
{
    if (newCount > 0) {
        mUi->mTabWidget->setTabText(AttendeesTab, i18nc("@title:tab Tab to modify attendees of an event or todo", "&Attendees (%1)", newCount));
    } else {
        mUi->mTabWidget->setTabText(AttendeesTab, i18nc("@title:tab Tab to modify attendees of an event or todo", "&Attendees"));
    }
}

void IncidenceDialogPrivate::updateResourceCount(int newCount)
{
    if (newCount > 0) {
        mUi->mTabWidget->setTabText(ResourcesTab, i18nc("@title:tab Tab to modify attendees of an event or todo", "&Resources (%1)", newCount));
    } else {
        mUi->mTabWidget->setTabText(ResourcesTab, i18nc("@title:tab Tab to modify attendees of an event or todo", "&Resources"));
    }
}

void IncidenceDialogPrivate::updateButtonStatus(bool isDirty)
{
    mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(isDirty || mInitiallyDirty);
}

bool IncidenceDialogPrivate::containsPayloadIdentifiers(const QSet<QByteArray> &partIdentifiers) const
{
    return partIdentifiers.contains(QByteArray("PLD:RFC822"));
}

void IncidenceDialogPrivate::handleItemSaveFail(EditorItemManager::SaveAction, const QString &errorMessage)
{
    Q_Q(IncidenceDialog);

    bool retry = false;

    if (!errorMessage.isEmpty()) {
        const QString message = i18nc("@info",
                                      "Unable to store the incidence in the calendar. Try again?\n\n "
                                      "Reason: %1",
                                      errorMessage);
        const int answer =
            KMessageBox::warningTwoActions(q, message, QString(), KGuiItem(i18nc("@action:button", "Retry"), u"dialog-ok"_s), KStandardGuiItem::cancel());
        retry = (answer == KMessageBox::ButtonCode::PrimaryAction);
    }

    if (retry) {
        mItemManager->save();
    } else {
        updateButtonStatus(isDirty());
        mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        mUi->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
    }
}

void IncidenceDialogPrivate::handleItemSaveFinish(EditorItemManager::SaveAction saveAction)
{
    Q_Q(IncidenceDialog);

    const Akonadi::Collection defaultCollection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());
    if ((mEditor->type() == KCalendarCore::Incidence::TypeEvent) && (mCalSelector->count() > 1) && !defaultCollection.isValid()) {
        const QString collectionName = mCalSelector->currentText();
        const QString message = xi18nc("@info",
                                       "<para>You have not set a default calendar for your events yet.</para>"
                                       "<para>Setting a default calendar will make creating new events faster and "
                                       "easier with less chance of filing them into the wrong folder.</para>"
                                       "<para>Would you like to set your default events calendar to "
                                       "<resource>%1</resource>?</para>",
                                       collectionName);
        const int answer = KMessageBox::questionTwoActions(q,
                                                           message,
                                                           i18nc("@title:window", "Set Default Calendar?"),
                                                           KGuiItem(i18nc("@action:button", "Set As Default"), u"dialog-ok"_s),
                                                           KGuiItem(i18nc("@action:button", "Do Not Set"), u"dialog-cancel"_s),
                                                           u"setDefaultCalendarCollection"_s);
        if (answer == KMessageBox::ButtonCode::PrimaryAction) {
            CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId(mItem.storageCollectionId());
        }
    }

    if (mCloseOnSave) {
        q->accept();
    } else {
        const Akonadi::Item item = mItemManager->item();
        Q_ASSERT(item.isValid());
        Q_ASSERT(item.hasPayload());
        Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
        // Now the item is successfully saved, reload it in the editor in order to
        // reset the dirty status of the editor.
        mEditor->load(item.payload<KCalendarCore::Incidence::Ptr>());
        mEditor->load(item);

        // Set the buttons to a reasonable state as well (ok and apply should be
        // disabled at this point).
        mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        mUi->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
        mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(isDirty());
    }

    if (saveAction == EditorItemManager::Create) {
        Q_EMIT q->incidenceCreated(mItemManager->item());
    }
}

bool IncidenceDialogPrivate::hasSupportedPayload(const Akonadi::Item &item) const
{
    return !Akonadi::CalendarUtils::incidence(item).isNull();
}

bool IncidenceDialogPrivate::isDirty() const
{
    if (mItem.isValid()) {
        return mEditor->isDirty() || mCalSelector->currentCollection().id() != mItem.storageCollectionId();
    } else {
        return mEditor->isDirty();
    }
}

bool IncidenceDialogPrivate::isValid() const
{
    Q_Q(const IncidenceDialog);
    if (mEditor->isValid()) {
        // Check if there's a selected collection.
        if (mCalSelector->currentCollection().isValid()) {
            return true;
        } else {
            qCWarning(INCIDENCEEDITOR_LOG) << "Select a collection first";
            Q_EMIT q->invalidCollection();
        }
    }

    return false;
}

void IncidenceDialogPrivate::load(const Akonadi::Item &item)
{
    Q_Q(IncidenceDialog);

    Q_ASSERT(hasSupportedPayload(item));

    if (CalendarSupport::hasJournal(item)) {
        // mUi->mTabWidget->removeTab(5);
        mUi->mTabWidget->removeTab(AttachmentsTab);
        mUi->mTabWidget->removeTab(RecurrenceTab);
        mUi->mTabWidget->removeTab(AlarmsTab);
        mUi->mTabWidget->removeTab(AttendeesTab);
        mUi->mTabWidget->removeTab(ResourcesTab);
    }

    mEditor->load(Akonadi::CalendarUtils::incidence(item));
    mEditor->load(item);

    const KCalendarCore::Incidence::Ptr incidence = Akonadi::CalendarUtils::incidence(item);
    const QStringList allEmails = IncidenceEditorNG::EditorConfig::instance()->allEmails();
    const KCalendarCore::Attendee me = incidence->attendeeByMails(allEmails);

    if (incidence->attendeeCount() > 1 // >1 because you won't drink alone
        && !me.isNull()
        && (me.status() == KCalendarCore::Attendee::NeedsAction || me.status() == KCalendarCore::Attendee::Tentative
            || me.status() == KCalendarCore::Attendee::InProcess)) {
        // Show the invitation bar: "You are invited [accept] [decline]"
        mUi->mInvitationBar->show();
    } else {
        mUi->mInvitationBar->hide();
    }

    qCDebug(INCIDENCEEDITOR_LOG) << "Loading item " << item.id() << "; parent " << item.parentCollection().id() << "; storage " << item.storageCollectionId();

    if (item.storageCollectionId() > -1) {
        mCalSelector->setDefaultCollection(Akonadi::Collection(item.storageCollectionId()));
    }

    if (!mCalSelector->mimeTypeFilter().contains(incidence->mimeType())) {
        mCalSelector->setMimeTypeFilter({incidence->mimeType()});
    }

    if (mEditor->type() == KCalendarCore::Incidence::TypeTodo) {
        q->setWindowIcon(QIcon::fromTheme(u"view-calendar-tasks"_s));
    } else if (mEditor->type() == KCalendarCore::Incidence::TypeEvent) {
        q->setWindowIcon(QIcon::fromTheme(u"view-calendar-day"_s));
    } else if (mEditor->type() == KCalendarCore::Incidence::TypeJournal) {
        q->setWindowIcon(QIcon::fromTheme(u"view-pim-journal"_s));
    }

    // Initialize tab's titles
    updateAttachmentCount(incidence->attachments().size());
    updateResourceCount(mIeResource->resourceCount());
    updateAttendeeCount(mIeAttendee->attendeeCount());
    handleRecurrenceChange(mIeRecurrence->currentRecurrenceType());
    handleAlarmCountChange(incidence->alarms().count());

    mItem = item;

    q->show();
}

Akonadi::Item IncidenceDialogPrivate::save(const Akonadi::Item &item)
{
    Q_ASSERT(mEditor->incidence<KCalendarCore::Incidence>());

    KCalendarCore::Incidence::Ptr const incidenceInEditor = mEditor->incidence<KCalendarCore::Incidence>();
    KCalendarCore::Incidence::Ptr const newIncidence(incidenceInEditor->clone());

    Akonadi::Item result = item;
    result.setMimeType(newIncidence->mimeType());

    // There's no editor that has the relatedTo property. We must set it here, by hand.
    // Otherwise it gets lost.
    // FIXME: Why don't we clone() incidenceInEditor then pass the clone to save(),
    // I wonder if we're not leaking other properties.
    newIncidence->setRelatedTo(incidenceInEditor->relatedTo());

    mEditor->save(newIncidence);
    mEditor->save(result);

    // Make sure that we don't loose uid for existing incidence
    newIncidence->setUid(mEditor->incidence<KCalendarCore::Incidence>()->uid());

    // Mark the incidence as changed
    if (mItem.isValid()) {
        newIncidence->setRevision(newIncidence->revision() + 1);
    }

    result.setPayload<KCalendarCore::Incidence::Ptr>(newIncidence);
    return result;
}

Akonadi::Collection IncidenceDialogPrivate::selectedCollection() const
{
    return mCalSelector->currentCollection();
}

void IncidenceDialogPrivate::reject(RejectReason reason, const QString &errorMessage)
{
    Q_UNUSED(reason)

    Q_Q(IncidenceDialog);
    qCCritical(INCIDENCEEDITOR_LOG) << "Rejecting:" << errorMessage;
    q->deleteLater();
}

/// IncidenceDialog

IncidenceDialog::IncidenceDialog(Akonadi::IncidenceChanger *changer, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , d_ptr(new IncidenceDialogPrivate(changer, this))
{
    Q_D(IncidenceDialog);
    setAttribute(Qt::WA_DeleteOnClose);

    d->mUi->mTabWidget->setCurrentIndex(0);
    d->mUi->mSummaryEdit->setFocus();

    d->mUi->buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip", "Save current changes"));
    d->mUi->buttonBox->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@action:button", "Save changes and close dialog"));
    d->mUi->buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(i18nc("@action:button", "Discard changes and close dialog"));
    d->mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    auto defaultButton = d->mUi->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    defaultButton->setText(i18nc("@action:button", "&Templates…"));
    defaultButton->setIcon(QIcon::fromTheme(u"project-development-new-template"_s));
    defaultButton->setToolTip(i18nc("@info:tooltip", "Manage templates for this item"));
    defaultButton->setWhatsThis(i18nc("@info:whatsthis",
                                      "Push this button to show a dialog that helps "
                                      "you manage a set of templates. Templates "
                                      "can make creating new items easier and faster "
                                      "by putting your favorite default values into "
                                      "the editor automatically."));

    connect(d->mUi->buttonBox, &QDialogButtonBox::clicked, this, &IncidenceDialog::slotButtonClicked);

    setModal(false);

    connect(d->mUi->mAcceptInvitationButton, &QAbstractButton::clicked, d->mIeAttendee, &IncidenceAttendee::acceptForMe);
    connect(d->mUi->mAcceptInvitationButton, &QAbstractButton::clicked, d->mUi->mInvitationBar, &QWidget::hide);
    connect(d->mUi->mDeclineInvitationButton, &QAbstractButton::clicked, d->mIeAttendee, &IncidenceAttendee::declineForMe);
    connect(d->mUi->mDeclineInvitationButton, &QAbstractButton::clicked, d->mUi->mInvitationBar, &QWidget::hide);
    connect(this, &IncidenceDialog::invalidCollection, this, [d]() {
        d->slotInvalidCollection();
    });
    readConfig();
}

IncidenceDialog::~IncidenceDialog()
{
    writeConfig();
}

void IncidenceDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), QLatin1StringView(myIncidenceDialogConfigGroupName));
    KWindowConfig::saveWindowSize(windowHandle(), group);
}

void IncidenceDialog::readConfig()
{
    create(); // ensure a window is created
    windowHandle()->resize(QSize(500, 500));
    KConfigGroup const group(KSharedConfig::openStateConfig(), QLatin1StringView(myIncidenceDialogConfigGroupName));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

void IncidenceDialog::load(const Akonadi::Item &item, const QDate &activeDate)
{
    Q_D(IncidenceDialog);
    d->mIeDateTime->setActiveDate(activeDate);
    if (item.isValid()) { // We're editing
        d->mItemManager->load(item);
    } else { // We're creating
        Q_ASSERT(d->hasSupportedPayload(item));
        d->load(item);
        show();
    }
}

void IncidenceDialog::selectCollection(const Akonadi::Collection &collection)
{
    Q_D(IncidenceDialog);
    d->setCalendarCollection(collection);
}

void IncidenceDialog::setIsCounterProposal(bool isCounterProposal)
{
    Q_D(IncidenceDialog);
    d->mItemManager->setIsCounterProposal(isCounterProposal);
}

QObject *IncidenceDialog::typeAheadReceiver() const
{
    Q_D(const IncidenceDialog);
    return d->mUi->mSummaryEdit;
}

void IncidenceDialog::slotButtonClicked(QAbstractButton *button)
{
    Q_D(IncidenceDialog);

    // NOLINTBEGIN(bugprone-branch-clone)
    if (d->mUi->buttonBox->button(QDialogButtonBox::Ok) == button) {
        if (d->isDirty() || d->mInitiallyDirty) {
            d->mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            d->mUi->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
            d->mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
            d->mCloseOnSave = true;
            d->mInitiallyDirty = false;
            d->mItemManager->save(toItemManagerFlags(d->mUi->mSignItip->isChecked(), d->mUi->mEncryptItip->isChecked()));
        } else {
            close();
        }
    } else if (d->mUi->buttonBox->button(QDialogButtonBox::Apply) == button) {
        d->mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        d->mUi->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
        d->mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

        d->mCloseOnSave = false;
        d->mInitiallyDirty = false;
        d->mItemManager->save(toItemManagerFlags(d->mUi->mSignItip->isChecked(), d->mUi->mEncryptItip->isChecked()));
    } else if (d->mUi->buttonBox->button(QDialogButtonBox::Cancel) == button) {
        if (d->isDirty()
            && KMessageBox::questionTwoActions(this,
                                               i18nc("@info", "Do you really want to cancel?"),
                                               i18nc("@title:window", "KOrganizer Confirmation"),
                                               KGuiItem(i18nc("@action:button", "Cancel Editing"), u"dialog-ok"_s),
                                               KGuiItem(i18nc("@action:button", "Do Not Cancel"), u"dialog-cancel"_s))
                == KMessageBox::ButtonCode::PrimaryAction) {
            QDialog::reject(); // Discard current changes
        } else if (!d->isDirty()) {
            QDialog::reject(); // No pending changes, just close the dialog.
        } // else { // the user wasn't finished editing after all }
    } else if (d->mUi->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
        d->manageTemplates();
    } else {
        Q_ASSERT(false); // Shouldn't happen
    }
    // NOLINTEND(bugprone-branch-clone)
}

void IncidenceDialog::reject()
{
    /* cppcheck-suppress constVariablePointer */
    Q_D(IncidenceDialog);

    // NOLINTBEGIN(bugprone-branch-clone)
    if (d->isDirty()
        && KMessageBox::questionTwoActions(this,
                                           i18nc("@info", "Do you really want to cancel?"),
                                           i18nc("@title:window", "KOrganizer Confirmation"),
                                           KGuiItem(i18nc("@action:button", "Cancel Editing"), u"dialog-ok"_s),
                                           KGuiItem(i18nc("@action:button", "Do Not Cancel"), u"dialog-cancel"_s))
            == KMessageBox::ButtonCode::PrimaryAction) {
        QDialog::reject(); // Discard current changes
    } else if (!d->isDirty()) {
        QDialog::reject(); // No pending changes, just close the dialog.
    }
    // NOLINTEND(bugprone-branch-clone)
}

void IncidenceDialog::closeEvent(QCloseEvent *event)
{
    /* cppcheck-suppress constVariablePointer */
    Q_D(IncidenceDialog);

    // NOLINTBEGIN(bugprone-branch-clone)
    if (d->isDirty()
        && KMessageBox::questionTwoActions(this,
                                           i18nc("@info", "Do you really want to cancel?"),
                                           i18nc("@title:window", "KOrganizer Confirmation"),
                                           KGuiItem(i18nc("@action:button", "Cancel Editing"), u"dialog-ok"_s),
                                           KGuiItem(i18nc("@action:button", "Do Not Cancel"), u"dialog-cancel"_s))
            == KMessageBox::ButtonCode::PrimaryAction) {
        QDialog::reject(); // Discard current changes
        QDialog::closeEvent(event);
    } else if (!d->isDirty()) {
        QDialog::reject(); // No pending changes, just close the dialog.
        QDialog::closeEvent(event);
    } else {
        event->ignore();
    }
    // NOLINTEND(bugprone-branch-clone)
}

void IncidenceDialog::setInitiallyDirty(bool initiallyDirty)
{
    Q_D(IncidenceDialog);
    d->mInitiallyDirty = initiallyDirty;
}

Akonadi::Item IncidenceDialog::item() const
{
    Q_D(const IncidenceDialog);
    return d->mItemManager->item();
}

void IncidenceDialog::handleSelectedCollectionChange(const Akonadi::Collection &collection)
{
    Q_D(IncidenceDialog);
    if (d->mItem.parentCollection().isValid()) {
        d->mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(collection.id() != d->mItem.parentCollection().id());
    }
}

#include "moc_incidencedialog.cpp"
