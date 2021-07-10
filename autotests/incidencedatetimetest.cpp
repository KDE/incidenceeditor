/*
  SPDX-FileCopyrightText: 2020 Glen Ditchfield <GJDitchfield@acm.org>

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

private Q_SLOTS:
    void testStartTimeValidation()
    {
        QLocale currentLocale;
        QLocale::setDefault(QLocale::c());

        Akonadi::Item item;
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        item.setPayload<KCalendarCore::Event::Ptr>(event);
        auto dialog = new IncidenceDialog();
        dialog->load(item);

        // Put the dialog into a known, valid state.
        auto editor = dialog->findChild<IncidenceEditor *>();
        QVERIFY2(editor, "Couldn't find the combined editor.");
        QCOMPARE(editor->metaObject()->className(), "IncidenceEditorNG::CombinedIncidenceEditor");
        auto allDay = dialog->findChild<QCheckBox *>(QStringLiteral("mWholeDayCheck"));
        QVERIFY2(allDay, "Couldn't find the 'All Day' checkbox.");
        allDay->setCheckState(Qt::Unchecked);
        auto startCheck = dialog->findChild<QCheckBox *>(QStringLiteral("mStartCheck"));
        QVERIFY2(startCheck, "Couldn't find the 'Start' checkbox.");
        startCheck->setCheckState(Qt::Checked);
        auto endCheck = dialog->findChild<QCheckBox *>(QStringLiteral("mEndCheck"));
        QVERIFY2(endCheck, "Couldn't find the 'End' checkbox.");
        endCheck->setCheckState(Qt::Checked);
        auto summary = dialog->findChild<QLineEdit *>(QStringLiteral("mSummaryEdit"));
        QVERIFY2(summary, "Couldn't find the 'Summary' field.");
        summary->setText(QStringLiteral("e"));
        QVERIFY(editor->isValid());

        auto startDate = dialog->findChild<KDateComboBox *>(QStringLiteral("mStartDateEdit"));
        QVERIFY2(startDate, "Couldn't find start date field.");
        startDate->setCurrentText(QStringLiteral("32 Jan 2000"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid start date.");
        startDate->setCurrentText(QStringLiteral("12 Jan 2000"));
        QVERIFY(editor->isValid());

        auto startTime = dialog->findChild<KTimeComboBox *>(QStringLiteral("mStartTimeEdit"));
        QVERIFY2(startTime, "Couldn't find start time field.");
        startTime->setCurrentText(QStringLiteral("12:99:00"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid start time.");
        startTime->setCurrentText(QStringLiteral("12:00:00"));
        QVERIFY(editor->isValid());

        auto endDate = dialog->findChild<KDateComboBox *>(QStringLiteral("mEndDateEdit"));
        QVERIFY2(endDate, "Couldn't find end date field.");
        endDate->setCurrentText(QStringLiteral("33 Jan 2000"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid end date.");
        endDate->setCurrentText(QStringLiteral("13 Jan 2000"));
        QVERIFY(editor->isValid());

        auto endTime = dialog->findChild<KTimeComboBox *>(QStringLiteral("mEndTimeEdit"));
        QVERIFY2(endTime, "Couldn't find end time field.");
        endTime->setCurrentText(QStringLiteral("12:99:00"));
        QVERIFY2(!editor->isValid(), "Didn't detect invalid end time.");
        endTime->setCurrentText(QStringLiteral("12:00:00"));
        QVERIFY(editor->isValid());

        endDate->setDate(startDate->date().addDays(-1));
        endTime->setTime(startTime->time());
        QVERIFY2(!editor->isValid(), "Didn't detect end date < start date");
        endDate->setDate(startDate->date());
        QVERIFY(editor->isValid());
        endTime->setTime(startTime->time().addSecs(-60));
        QVERIFY2(!editor->isValid(), "Didn't detect end time < start time");
        endTime->setTime(startTime->time());
        QVERIFY(editor->isValid());

        QLocale::setDefault(currentLocale);
    }
};

QTEST_MAIN(IncidenceDateTimeTest)
#include "incidencedatetimetest.moc"
