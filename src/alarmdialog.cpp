/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "alarmdialog.h"
#include "editorconfig.h"
#include "ui_alarmdialog.h"

#include <KEmailAddress>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;

AlarmDialog::AlarmDialog(KCalendarCore::Incidence::IncidenceType incidenceType, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::AlarmDialog)
    , mIncidenceType(incidenceType)
{
    setWindowTitle(i18nc("@title:window", "Create a new reminder"));
    auto mainLayout = new QVBoxLayout(this);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AlarmDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AlarmDialog::reject);

    auto mainWidget = new QWidget(this);
    mUi->setupUi(mainWidget);
    mainLayout->addWidget(mainWidget);
    mainLayout->addWidget(buttonBox);

    const int defaultReminderTime = IncidenceEditorNG::EditorConfig::instance()->reminderTime();
    mUi->mAlarmOffset->setValue(defaultReminderTime);

    int defaultReminderUnits = IncidenceEditorNG::EditorConfig::instance()->reminderTimeUnits();
    if (defaultReminderUnits < 0 || defaultReminderUnits > 2) {
        defaultReminderUnits = 0; // minutes
    }

    mUi->mOffsetUnit->setCurrentIndex(defaultReminderUnits);
    mUi->mSoundFile->setMimeTypeFilters({QStringLiteral("audio/x-wav"), QStringLiteral("audio/x-mp3"), QStringLiteral("application/ogg")});

    if (IncidenceEditorNG::EditorConfig::instance()->defaultAudioFileReminders()) {
        mUi->mSoundFile->setUrl(IncidenceEditorNG::EditorConfig::instance()->audioFilePath());
    }

    fillCombo();
}

AlarmDialog::~AlarmDialog()
{
    delete mUi;
}

void AlarmDialog::load(const KCalendarCore::Alarm::Ptr &alarm)
{
    if (!alarm) {
        return;
    }

    setWindowTitle(i18nc("@title:window", "Edit existing reminder"));

    // Offsets
    int offset;
    int beforeafterpos = 0;

    if (alarm->hasEndOffset()) {
        beforeafterpos = 2;
        offset = alarm->endOffset().asSeconds();
    } else {
        // TODO: Also allow alarms at fixed times, not relative to start/end
        offset = alarm->startOffset().asSeconds();
    }
    // Negative offset means before the start/end...
    if (offset < 0) {
        offset = -offset;
    } else {
        ++beforeafterpos;
    }
    mUi->mBeforeAfter->setCurrentIndex(beforeafterpos);

    offset = offset / 60; // make minutes
    int useoffset = offset;

    if (offset % (24 * 60) == 0 && offset > 0) { // divides evenly into days?
        useoffset = offset / (24 * 60);
        mUi->mOffsetUnit->setCurrentIndex(2);
    } else if (offset % 60 == 0 && offset > 0) { // divides evenly into hours?
        useoffset = offset / 60;
        mUi->mOffsetUnit->setCurrentIndex(1);
    } else {
        useoffset = offset;
        mUi->mOffsetUnit->setCurrentIndex(0);
    }
    mUi->mAlarmOffset->setValue(useoffset);

    // Repeating
    mUi->mRepeats->setChecked(alarm->repeatCount() > 0);
    if (alarm->repeatCount() > 0) {
        mUi->mRepeatCount->setValue(alarm->repeatCount());
        mUi->mRepeatInterval->setValue(alarm->snoozeTime().asSeconds() / 60); // show as minutes
    }
    int id = 0;

    switch (alarm->type()) {
    case KCalendarCore::Alarm::Audio:
        mUi->mTypeCombo->setCurrentIndex(1);
        mUi->mSoundFile->setUrl(QUrl::fromLocalFile(alarm->audioFile()));
        id = 1;
        break;
    case KCalendarCore::Alarm::Procedure:
        mUi->mTypeCombo->setCurrentIndex(2);
        mUi->mApplication->setUrl(QUrl::fromLocalFile(alarm->programFile()));
        mUi->mAppArguments->setText(alarm->programArguments());
        id = 2;
        break;
    case KCalendarCore::Alarm::Email: {
        mUi->mTypeCombo->setCurrentIndex(3);
        KCalendarCore::Person::List addresses = alarm->mailAddresses();
        QStringList add;
        add.reserve(addresses.count());
        const KCalendarCore::Person::List::ConstIterator end(addresses.constEnd());
        for (KCalendarCore::Person::List::ConstIterator it = addresses.constBegin(); it != end; ++it) {
            add << (*it).fullName();
        }
        mUi->mEmailAddress->setText(add.join(QLatin1String(", ")));
        mUi->mEmailText->setPlainText(alarm->mailText());
        id = 3;
        break;
    }
    case KCalendarCore::Alarm::Display:
    case KCalendarCore::Alarm::Invalid:
    default:
        mUi->mTypeCombo->setCurrentIndex(0);
        mUi->mDisplayText->setPlainText(alarm->text());
        break;
    }

    mUi->mTypeStack->setCurrentIndex(id);
    if (alarm->audioFile().isEmpty() && IncidenceEditorNG::EditorConfig::instance()->defaultAudioFileReminders()) {
        mUi->mSoundFile->setUrl(IncidenceEditorNG::EditorConfig::instance()->audioFilePath());
    }
}

void AlarmDialog::save(const KCalendarCore::Alarm::Ptr &alarm) const
{
    // Offsets
    int offset = mUi->mAlarmOffset->value() * 60; // minutes
    int offsetunit = mUi->mOffsetUnit->currentIndex();
    if (offsetunit >= 1) {
        offset *= 60; // hours
    }
    if (offsetunit >= 2) {
        offset *= 24; // days
    }
    if (offsetunit >= 3) {
        offset *= 7; // weeks
    }

    const int beforeafterpos = mUi->mBeforeAfter->currentIndex();
    if (beforeafterpos % 2 == 0) { // before -> negative
        offset = -offset;
    }

    // Note: if this triggers, fix the logic at the place causing it. It really makes
    // no sense to have both disabled.
    Q_ASSERT(mAllowBeginReminders || mAllowEndReminders);

    // TODO: Add possibility to specify a given time for the reminder

    // We assume that if mAllowBeginReminders is not set, that mAllowBeginReminders
    // is set.
    if (!mAllowBeginReminders) { // before or after DTDUE
        alarm->setEndOffset(KCalendarCore::Duration(offset));
    } else if (beforeafterpos == 0 || beforeafterpos == 1) { // before or after DTSTART
        alarm->setStartOffset(KCalendarCore::Duration(offset));
    } else if (beforeafterpos == 2 || beforeafterpos == 3) { // before or after DTEND/DTDUE
        alarm->setEndOffset(KCalendarCore::Duration(offset));
    }

    // Repeating
    if (mUi->mRepeats->isChecked()) {
        alarm->setRepeatCount(mUi->mRepeatCount->value());
        alarm->setSnoozeTime(mUi->mRepeatInterval->value() * 60); // convert back to seconds
    } else {
        alarm->setRepeatCount(0);
    }

    if (mUi->mTypeCombo->currentIndex() == 1) { // Audio
        alarm->setAudioAlarm(mUi->mSoundFile->url().toLocalFile());
    } else if (mUi->mTypeCombo->currentIndex() == 2) { // Application / script
        alarm->setProcedureAlarm(mUi->mApplication->url().toLocalFile(), mUi->mAppArguments->text());
    } else if (mUi->mTypeCombo->currentIndex() == 3) { // Email
        QStringList addresses = KEmailAddress::splitAddressList(mUi->mEmailAddress->text());
        KCalendarCore::Person::List add;
        add.reserve(addresses.count());
        for (QStringList::Iterator it = addresses.begin(), end = addresses.end(); it != end; ++it) {
            add << KCalendarCore::Person::fromFullName(*it);
        }
        // TODO: Add a subject line and possibilities for attachments
        alarm->setEmailAlarm(QString(), mUi->mEmailText->toPlainText(), add);
    } else { // Display
        alarm->setDisplayAlarm(mUi->mDisplayText->toPlainText());
    }
}

void AlarmDialog::fillCombo()
{
    QStringList items;

    if (mIncidenceType == KCalendarCore::Incidence::TypeTodo) {
        mUi->mBeforeAfter->clear();

        if (mAllowBeginReminders) {
            items << i18n("Before the to-do starts") << i18n("After the to-do starts");
        }

        if (mAllowEndReminders) {
            items << i18n("Before the to-do is due") << i18n("After the to-do is due");
        }
    } else {
        if (mAllowBeginReminders) {
            items << i18n("Before the event starts") << i18n("After the event starts");
        }
        if (mAllowEndReminders) {
            items << i18n("Before the event ends") << i18n("After the event ends");
        }
    }

    mUi->mBeforeAfter->clear();
    mUi->mBeforeAfter->addItems(items);
}

void AlarmDialog::setAllowBeginReminders(bool allow)
{
    mAllowBeginReminders = allow;
    fillCombo();
}

void AlarmDialog::setAllowEndReminders(bool allow)
{
    mAllowEndReminders = allow;
    fillCombo();
}

void AlarmDialog::setOffset(int offset)
{
    Q_ASSERT(offset > 0);
    mUi->mAlarmOffset->setValue(offset);
}

void AlarmDialog::setUnit(Unit unit)
{
    mUi->mOffsetUnit->setCurrentIndex(unit);
}

void AlarmDialog::setWhen(When when)
{
    Q_ASSERT(when <= mUi->mBeforeAfter->count());
    mUi->mBeforeAfter->setCurrentIndex(when);
}
