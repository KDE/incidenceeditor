/*
SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "individualmaildialog.h"

#include <KGuiItem>

#include <QComboBox>
#include <QObject>
#include <QTest>

using namespace IncidenceEditorNG;
using namespace Qt::Literals::StringLiterals;
class TestIndividualMailDialog : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDialog()
    {
        KCalendarCore::Attendee::List attendees;
        KGuiItem const buttonYes = KGuiItem(u"Send Email"_s);
        KGuiItem const buttonNo = KGuiItem(u"Do not send"_s);

        KCalendarCore::Attendee const attendee1(u"test1"_s, QStringLiteral("test1@example.com"));
        KCalendarCore::Attendee const attendee2(u"test2"_s, QStringLiteral("test2@example.com"));
        KCalendarCore::Attendee const attendee3(u"test3"_s, QStringLiteral("test3@example.com"));

        attendees << attendee1 << attendee2 << attendee3;

        IndividualMailDialog dialog(u"title"_s, attendees, buttonYes, buttonNo, nullptr);

        QCOMPARE(dialog.editAttendees().count(), 0);
        QCOMPARE(dialog.updateAttendees().count(), 3);

        // Just make sure, that the QCombobox is sorted like we think
        QComboBox *first = dialog.mAttendeeDecision[0].second;
        QCOMPARE((IndividualMailDialog::Decisions)first->itemData(0, Qt::UserRole).toInt(), IndividualMailDialog::Update);
        QCOMPARE((IndividualMailDialog::Decisions)first->itemData(1, Qt::UserRole).toInt(), IndividualMailDialog::NoUpdate);
        QCOMPARE((IndividualMailDialog::Decisions)first->itemData(2, Qt::UserRole).toInt(), IndividualMailDialog::Edit);

        // No update for first attendee, other default
        first->setCurrentIndex(1);
        QCOMPARE(dialog.editAttendees().count(), 0);
        QCOMPARE(dialog.updateAttendees().count(), 2);
        QVERIFY(dialog.updateAttendees().contains(attendee2));
        QVERIFY(dialog.updateAttendees().contains(attendee3));

        // edit for first attendee, other default
        first->setCurrentIndex(2);
        QCOMPARE(dialog.editAttendees().count(), 1);
        QCOMPARE(dialog.updateAttendees().count(), 2);
        QCOMPARE(dialog.editAttendees().constFirst(), attendee1);
    }
};

QTEST_MAIN(TestIndividualMailDialog)

#include "testindividualmaildialog.moc"
