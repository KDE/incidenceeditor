/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef INCIDENCEEDITOR_INDIVIDUALMAILCOMPONENTFACTORY_H
#define INCIDENCEEDITOR_INDIVIDUALMAILCOMPONENTFACTORY_H

#include "incidenceeditor_export.h"

#include <KIdentityManagement/Identity>
#include <mailtransportakonadi/messagequeuejob.h>
#include <Akonadi/Calendar/IncidenceChanger>
namespace IncidenceEditorNG {
class OpenComposerJob;
class IndividualMailDialog;

class IndividualMessageQueueJob : public MailTransport::MessageQueueJob
{
    Q_OBJECT
public:
    explicit IndividualMessageQueueJob(const KIdentityManagement::Identity &identity, const KCalendarCore::Attendee::List &update, const KCalendarCore::Attendee::List &edit, QObject *parent);

    void start() override;

private:
    void startQueueJob(const QStringList &messageTo, const QStringList &to, const QStringList &messageCc, const QStringList &cc);
    void startComposerJob(const QStringList &to, const QStringList &cc);
    void handleJobFinished(KJob *job);
    KCalendarCore::Attendee::List mUpdate;
    KCalendarCore::Attendee::List mEdit;
    KIdentityManagement::Identity mIdentity;
    MailTransport::MessageQueueJob *mQueueJob = nullptr;
    OpenComposerJob *mComposerJob = nullptr;
};

class IndividualMailITIPHandlerDialogDelegate : public Akonadi::ITIPHandlerDialogDelegate
{
    Q_OBJECT
public:

    explicit IndividualMailITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent);

    void openDialogIncidenceCreated(
        Recipient recipient, const QString &question, Action action = ActionAsk, const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer",
                                                                                                                            "Send Email")), const KGuiItem &buttonNo = KGuiItem(i18nc(
                                                                                                                                                                                    "@action:button dialog negative answer",
                                                                                                                                                                                    "Do Not Send")))
    override;

    void openDialogIncidenceModified(
        bool attendeeStatusChanged, Recipient recipient, const QString &question, Action action = ActionAsk, const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer",
                                                                                                                                                        "Send Email")), const KGuiItem &buttonNo = KGuiItem(i18nc(
                                                                                                                                                                                                                "@action:button dialog negative answer",
                                                                                                                                                                                                                "Do Not Send")))
    override;

    void openDialogIncidenceDeleted(
        Recipient recipient, const QString &question, Action action = ActionAsk, const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer",
                                                                                                                            "Send Email")), const KGuiItem &buttonNo = KGuiItem(i18nc(
                                                                                                                                                                                    "@action:button dialog negative answer",
                                                                                                                                                                                    "Do Not Send")))
    override;

Q_SIGNALS:
    void setEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit);
    void setUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update);

protected:
    void openDialog(const QString &question, const KCalendarCore::Attendee::List &attendees, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo);

private:
    void onDialogClosed(int result);
    IndividualMailDialog *mDialog = nullptr;
};

class INCIDENCEEDITOR_EXPORT IndividualMailComponentFactory : public Akonadi::
    ITIPHandlerComponentFactory
{
    Q_OBJECT
public:
    explicit IndividualMailComponentFactory(QObject *parent = nullptr);
    MailTransport::MessageQueueJob *createMessageQueueJob(
        const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagement::Identity &identity, QObject *parent) override;

    Akonadi::ITIPHandlerDialogDelegate *createITIPHanderDialogDelegate(
        const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent) override;

public Q_SLOTS:
    void onSetEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit);
    void onSetUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update);

private:
    QHash<QString, KCalendarCore::Attendee::List> mEdit;
    QHash<QString, KCalendarCore::Attendee::List> mUpdate;
};
}
#endif
