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

public:
    IncidenceDateTimeTest()
    {
        mDialog = new IncidenceDialog();
        mAllDay = mDialog->findChild<QCheckBox *>(QStringLiteral("mWholeDayCheck"));
        QVERIFY2(mAllDay, "Couldn't find the 'All Day' checkbox.");
        mStartCheck = mDialog->findChild<QCheckBox *>(QStringLiteral("mStartCheck"));
        QVERIFY2(mStartCheck, "Couldn't find the 'Start' checkbox.");
        mEndCheck = mDialog->findChild<QCheckBox *>(QStringLiteral("mEndCheck"));
        QVERIFY2(mEndCheck, "Couldn't find the 'End' checkbox.");
        mStartDate = mDialog->findChild<KDateComboBox *>(QStringLiteral("mStartDateEdit"));
        QVERIFY2(mStartDate, "Couldn't find start date field.");
        mStartTime = mDialog->findChild<KTimeComboBox *>(QStringLiteral("mStartTimeEdit"));
        QVERIFY2(mStartTime, "Couldn't find start time field.");
        mStartZone = mDialog->findChild<KTimeZoneComboBox *>(QStringLiteral("mTimeZoneComboStart"));
        QVERIFY2(mStartZone, "Couldn't find start time zone field.");
        mEndDate = mDialog->findChild<KDateComboBox *>(QStringLiteral("mEndDateEdit"));
        QVERIFY2(mEndDate, "Couldn't find end date field.");
        mEndTime = mDialog->findChild<KTimeComboBox *>(QStringLiteral("mEndTimeEdit"));
        QVERIFY2(mEndTime, "Couldn't find end time field.");
        mEndZone = mDialog->findChild<KTimeZoneComboBox *>(QStringLiteral("mTimeZoneComboEnd"));
        QVERIFY2(mEndZone, "Couldn't find end time zone field.");
    }

private Q_SLOTS:

    void initTestCase()
    {
        qputenv("TZ", "Asia/Tokyo");
    }

    void testStartTimeValidation()
    {
        QLocale currentLocale;
        QLocale::setDefault(QLocale::c());

        const QDate date {2022, 04, 01};
        const QTime time {00, 00, 00};
        const QTimeZone zone {"Etc/UTC"};
        const QDateTime dt {date, time, zone};

        // Put the dialog into a known, valid state.
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        event->setSummary(QStringLiteral("e"));
        event->setDtStart(dt);
        event->setDtEnd(dt);
        event->setAllDay(false);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Event::Ptr>(event);
        mDialog->load(item);
        auto editor = mDialog->findChild<IncidenceEditor *>();
        QVERIFY2(editor, "Couldn't find the combined editor.");
        QCOMPARE(editor->metaObject()->className(), "IncidenceEditorNG::CombinedIncidenceEditor");
        QVERIFY(editor->isValid());

        mStartDate->setCurrentText(QStringLiteral("32 Jan 2000"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid start date.");
        mStartDate->setCurrentText(QStringLiteral("12 Jan 2000"));
        QVERIFY(editor->isValid());

        mStartTime->setCurrentText(QStringLiteral("12:99:00"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid start time.");
        mStartTime->setCurrentText(QStringLiteral("12:00:00"));
        QVERIFY(editor->isValid());

        mEndDate->setCurrentText(QStringLiteral("33 Jan 2000"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid end date.");
        mEndDate->setCurrentText(QStringLiteral("13 Jan 2000"));
        QVERIFY(editor->isValid());

        mEndTime->setCurrentText(QStringLiteral("12:99:00"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid end time.");
        mEndTime->setCurrentText(QStringLiteral("12:00:00"));
        QVERIFY(editor->isValid());

        mStartZone->selectTimeZone(QTimeZone("Africa/Abidjan"));     // UTC.
        mEndZone->selectTimeZone(QTimeZone("Africa/Abidjan"));
        QVERIFY(editor->isValid());

        mEndDate->setDate(mStartDate->date().addDays(-1));
        mEndTime->setTime(mStartTime->time());
        QVERIFY2(!editor->isValid(), "Didn't detect end date < start date");
        mEndDate->setDate(mStartDate->date());
        QVERIFY(editor->isValid());
        mEndTime->setTime(mStartTime->time().addSecs(-60));
        QVERIFY2(!editor->isValid(), "Didn't detect end time < start time");
        mEndTime->setTime(mStartTime->time());
        QVERIFY(editor->isValid());
        mEndZone->selectTimeZone(QTimeZone("Africa/Addis_Ababa"));   // UTC+3; causes 3-hour shift in effective end time.
        QVERIFY2(!editor->isValid(), "Didn't detect end time < start time in different time zone");

        QLocale::setDefault(currentLocale);
    }

    void testLoadingTimelessTodo()
    {
        KCalendarCore::Todo::Ptr todo {new KCalendarCore::Todo};
        todo->setDtStart(QDateTime());
        todo->setDtDue(QDateTime());
        todo->setAllDay(false);
        Akonadi::Item item;
        item.setPayload<KCalendarCore::Todo::Ptr>(todo);
        mDialog->load(item);

        QVERIFY( ! mAllDay->isEnabled());
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
        const QDate date {2022, 04, 01};
        const QTime time {00, 00, 00};
        const QTimeZone zone {"Africa/Abidjan"};
        const QDateTime dt {date, time, zone};

        KCalendarCore::Todo::Ptr todo {new KCalendarCore::Todo};
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
        const QDate date {2022, 04, 01};
        const QTime time {00, 00, 00};
        const QTimeZone zone {"Africa/Abidjan"};
        const QDateTime dt {date, time, zone};

        KCalendarCore::Todo::Ptr todo {new KCalendarCore::Todo};
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
        QVERIFY( ! mStartTime->isEnabled());
        QVERIFY( ! mStartZone->isVisible());
        QVERIFY(mEndCheck->isEnabled());
        QVERIFY(mEndCheck->isVisible());
        QCOMPARE(mEndCheck->checkState(), Qt::Checked);
        QVERIFY( ! mEndTime->isEnabled());
        QVERIFY( ! mEndZone->isVisible());
    }

};

QTEST_MAIN(IncidenceDateTimeTest)
#include "incidencedatetimetest.moc"
