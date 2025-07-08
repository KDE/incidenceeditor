/*
  SPDX-FileCopyrightText: 2020, 2022 Glen Ditchfield <GJDitchfield@acm.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QObject>
#include <QTest>

#include "combinedincidenceeditor.h"
#include "incidencedatetime.h"
#include "incidencedialog.h"
#include "ui_dialogdesktop.h"

using namespace IncidenceEditorNG;

class IncidenceDateTimeTest : public QObject
{
    Q_OBJECT

    IncidenceDialog *mDialog;
    QCheckBox *mAllDay;
    QCheckBox *mStartCheck;
    QCheckBox *mEndCheck;
    KDateComboBox *mStartDate;
    KTimeComboBox *mStartTime;
    KTimeZoneComboBox *mStartZone;
    KDateComboBox *mEndDate;
    KTimeComboBox *mEndTime;
    KTimeZoneComboBox *mEndZone;
    IncidenceEditor *mEditor;

public:
    IncidenceDateTimeTest()
    {
        mDialog = new IncidenceDialog();
        mAllDay = mDialog->findChild<QCheckBox *>(u"mWholeDayCheck"_s);
        QVERIFY2(mAllDay, "Couldn't find the 'All Day' checkbox.");
        mStartCheck = mDialog->findChild<QCheckBox *>(u"mStartCheck"_s);
        QVERIFY2(mStartCheck, "Couldn't find the 'Start' checkbox.");
        mEndCheck = mDialog->findChild<QCheckBox *>(u"mEndCheck"_s);
        QVERIFY2(mEndCheck, "Couldn't find the 'End' checkbox.");
        mStartDate = mDialog->findChild<KDateComboBox *>(u"mStartDateEdit"_s);
        QVERIFY2(mStartDate, "Couldn't find start date field.");
        mStartTime = mDialog->findChild<KTimeComboBox *>(u"mStartTimeEdit"_s);
        QVERIFY2(mStartTime, "Couldn't find start time field.");
        mStartZone = mDialog->findChild<KTimeZoneComboBox *>(u"mTimeZoneComboStart"_s);
        QVERIFY2(mStartZone, "Couldn't find start time zone field.");
        mEndDate = mDialog->findChild<KDateComboBox *>(u"mEndDateEdit"_s);
        QVERIFY2(mEndDate, "Couldn't find end date field.");
        mEndTime = mDialog->findChild<KTimeComboBox *>(u"mEndTimeEdit"_s);
        QVERIFY2(mEndTime, "Couldn't find end time field.");
        mEndZone = mDialog->findChild<KTimeZoneComboBox *>(u"mTimeZoneComboEnd"_s);
        QVERIFY2(mEndZone, "Couldn't find end time zone field.");
        mEditor = mDialog->findChild<IncidenceEditor *>();
        QVERIFY2(mEditor, "Couldn't find the combined editor.");
    }

private Q_SLOTS:

    void initTestCase()
    {
        qputenv("TZ", "Asia/Tokyo");
    }

    void testEventTimeValidation()
    {
        QLocale::setDefault(QLocale::c());

        const QDate date{2022, 04, 11};
        const QTime time{10, 11, 12};
        const QTimeZone zone{"Etc/UTC"};
        const QDateTime dt{date, time, zone};

        // Put the dialog into a known, valid state.
        KCalendarCore::Event::Ptr const event(new KCalendarCore::Event);
        event->setSummary(u"e"_s);
        event->setDtStart(dt);
        event->setDtEnd(dt);
        event->setAllDay(false);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Event::Ptr>(event);
        mDialog->load(item);
        QVERIFY(mEditor->isValid());

        auto validDate = mStartDate->currentText();
        auto invalidDate = mStartDate->currentText().replace(u"11"_s, QStringLiteral("31"));
        mStartDate->setCurrentText(invalidDate);
        QVERIFY2(!mEditor->isValid(), qPrintable(u"Didn't detect invalid start date "_s.append(invalidDate)));
        mStartDate->setCurrentText(validDate);
        QVERIFY2(mEditor->isValid(), qPrintable(validDate.append(u" considered invalid."_s)));

        auto validTime = mStartTime->currentText();
        auto invalidTime = mStartTime->currentText().replace(u"11"_s, QStringLiteral("61"));
        mStartTime->setCurrentText(invalidTime);
        QVERIFY2(!mEditor->isValid(), qPrintable(u"Didn't detect invalid start time "_s.append(invalidTime)));
        mStartTime->setCurrentText(validTime);
        QVERIFY2(mEditor->isValid(), qPrintable(validTime.append(u" considered invalid."_s)));

        validDate = mEndDate->currentText();
        invalidDate = mEndDate->currentText().replace(u"11"_s, QStringLiteral("31"));
        mEndDate->setCurrentText(invalidDate);
        QVERIFY2(!mEditor->isValid(), qPrintable(u"Didn't detect invalid end date "_s.append(invalidDate)));
        mEndDate->setCurrentText(validDate);
        QVERIFY2(mEditor->isValid(), qPrintable(validDate.append(u" considered invalid."_s)));

        validTime = mEndTime->currentText();
        invalidTime = mEndTime->currentText().replace(u"11"_s, QStringLiteral("61"));
        mEndTime->setCurrentText(invalidTime);
        QVERIFY2(!mEditor->isValid(), qPrintable(u"Didn't detect invalid end time "_s.append(invalidTime)));
        mEndTime->setCurrentText(validTime);
        QVERIFY2(mEditor->isValid(), qPrintable(validTime.append(u" considered invalid."_s)));
    }

    void testEventTimeOrdering()
    {
        QLocale const currentLocale;
        QLocale::setDefault(QLocale::c());

        const QDate date{2022, 04, 11};
        const QTime time{10, 11, 12};
        const QTimeZone zone{"Africa/Abidjan"}; // UTC+0.
        const QDateTime dt{date, time, zone};

        // Put the dialog into a known, valid state.
        KCalendarCore::Event::Ptr const event(new KCalendarCore::Event);
        event->setSummary(u"e"_s);
        event->setDtStart(dt);
        event->setDtEnd(dt);
        event->setAllDay(false);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Event::Ptr>(event);
        mDialog->load(item);
        QVERIFY(mEditor->isValid());

        mEndDate->setDate(mStartDate->date().addDays(-1));
        mEndTime->setTime(mStartTime->time());
        QVERIFY2(!mEditor->isValid(), "Didn't detect end date < start date");
        mEndDate->setDate(mStartDate->date());
        QVERIFY(mEditor->isValid());
        mEndTime->setTime(mStartTime->time().addSecs(-60));
        QVERIFY2(!mEditor->isValid(), "Didn't detect end time < start time");
        mEndTime->setTime(mStartTime->time());
        QVERIFY(mEditor->isValid());
        mEndZone->selectTimeZone(QTimeZone("Africa/Addis_Ababa")); // UTC+3; causes 3-hour shift in effective end time.
        QVERIFY2(!mEditor->isValid(), "Didn't detect end time < start time in different time zone");

        QLocale::setDefault(currentLocale);
    }

    void testLoadingTimelessTodo()
    {
        KCalendarCore::Todo::Ptr const todo{new KCalendarCore::Todo};
        todo->setDtStart(QDateTime());
        todo->setDtDue(QDateTime());
        todo->setAllDay(false);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
        mDialog->load(item);

        QVERIFY(!mAllDay->isEnabled());
        QVERIFY(mAllDay->isVisible());
        QVERIFY(mStartCheck->isEnabled());
        QVERIFY(mStartCheck->isVisible());
        QCOMPARE(mStartCheck->checkState(), Qt::Unchecked);
        QVERIFY(mEndCheck->isEnabled());
        QVERIFY(mEndCheck->isVisible());
        QCOMPARE(mEndCheck->checkState(), Qt::Unchecked);
    }

    void testLoadingTimedTodo()
    {
        const QDate date{2022, 04, 01};
        const QTime time{00, 00, 00};
        const QTimeZone zone{"Africa/Abidjan"};
        const QDateTime dt{date, time, zone};

        KCalendarCore::Todo::Ptr const todo{new KCalendarCore::Todo};
        todo->setDtStart(dt);
        todo->setDtDue(dt);
        todo->setAllDay(false);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
        mDialog->load(item);

        QVERIFY(mAllDay->isEnabled());
        QVERIFY(mAllDay->isVisible());
        QCOMPARE(mAllDay->checkState(), Qt::Unchecked);
        QVERIFY(mStartCheck->isEnabled());
        QVERIFY(mStartCheck->isVisible());
        QCOMPARE(mStartCheck->checkState(), Qt::Checked);
        QCOMPARE(mStartDate->date(), date);
        QCOMPARE(mStartTime->time(), time);
        QCOMPARE(mStartZone->selectedTimeZone(), zone);
        QVERIFY(mEndCheck->isEnabled());
        QVERIFY(mEndCheck->isVisible());
        QCOMPARE(mEndCheck->checkState(), Qt::Checked);
        QCOMPARE(mEndDate->date(), date);
        QCOMPARE(mEndTime->time(), time);
        QCOMPARE(mEndZone->selectedTimeZone(), zone);
    }

    void testLoadingAlldayTodo()
    {
        const QDate date{2022, 04, 01};
        const QTime time{00, 00, 00};
        const QTimeZone zone{"Africa/Abidjan"};
        const QDateTime dt{date, time, zone};

        KCalendarCore::Todo::Ptr const todo{new KCalendarCore::Todo};
        todo->setDtStart(dt);
        todo->setDtDue(dt);
        todo->setAllDay(true);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
        mDialog->load(item);

        QCOMPARE(mAllDay->isEnabled(), true);
        QVERIFY(mAllDay->isVisible());
        QCOMPARE(mAllDay->checkState(), Qt::Checked);
        QVERIFY(mStartCheck->isEnabled());
        QVERIFY(mStartCheck->isVisible());
        QCOMPARE(mStartCheck->checkState(), Qt::Checked);
        QCOMPARE(mStartDate->date(), date);
        QVERIFY(!mStartTime->isEnabled());
        QVERIFY(!mStartZone->isVisible());
        QVERIFY(mEndCheck->isEnabled());
        QVERIFY(mEndCheck->isVisible());
        QCOMPARE(mEndCheck->checkState(), Qt::Checked);
        QVERIFY(!mEndTime->isEnabled());
        QVERIFY(!mEndZone->isVisible());
    }
};

QTEST_MAIN(IncidenceDateTimeTest)
#include "incidencedatetimetest.moc"
