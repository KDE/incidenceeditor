/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
 */

#pragma once

#include "incidenceeditor_export.h"

#include <Akonadi/IncidenceChanger>
#include <Akonadi/MessageQueueJob>
#include <KIdentityManagementCore/Identity>
namespace IncidenceEditorNG
{
class OpenComposerJob;
class IndividualMailDialog;

class IndividualMessageQueueJob : public Akonadi::MessageQueueJob
{
    Q_OBJECT
public:
    explicit IndividualMessageQueueJob(const KIdentityManagementCore::Identity &identity,
                                       const KCalendarCore::Attendee::List &update,
                                       const KCalendarCore::Attendee::List &edit,
                                       QObject *parent);

    void start() override;

private:
    void startQueueJob(const QStringList &messageTo, const QStringList &to, const QStringList &messageCc, const QStringList &cc);
    void startComposerJob(const QStringList &to, const QStringList &cc);
    void handleJobFinished(KJob *job);
    KCalendarCore::Attendee::List mUpdate;
    KCalendarCore::Attendee::List mEdit;
    KIdentityManagementCore::Identity mIdentity;
    Akonadi::MessageQueueJob *mQueueJob = nullptr;
    OpenComposerJob *mComposerJob = nullptr;
};

class IndividualMailITIPHandlerDialogDelegate : public Akonadi::ITIPHandlerDialogDelegate
{
    Q_OBJECT
public:
    explicit IndividualMailITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent);

    void openDialogIncidenceCreated(Recipient recipient,
                                    const QString &question,
                                    Action action = ActionAsk,
                                    const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) override;

    void openDialogIncidenceModified(bool attendeeStatusChanged,
                                     Recipient recipient,
                                     const QString &question,
                                     Action action = ActionAsk,
                                     const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                     const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) override;

    void openDialogIncidenceDeleted(Recipient recipient,
                                    const QString &question,
                                    Action action = ActionAsk,
                                    const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) override;

Q_SIGNALS:
    void setEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit);
    void setUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update);

protected:
    void
    openDialog(const QString &question, const KCalendarCore::Attendee::List &attendees, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo);

private:
    void onDialogClosed(int result);
    IndividualMailDialog *mDialog = nullptr;
};
/*!
 * \class IncidenceEditorNG::IndividualMailComponentFactory
 * \inmodule IncidenceEditor
 * \inheaderfile IncidenceEditor/IndividualMailComponentFactory
 */
class INCIDENCEEDITOR_EXPORT IndividualMailComponentFactory : public Akonadi::ITIPHandlerComponentFactory
{
    Q_OBJECT
public:
    /*!
     * Creates a new IndividualMailComponentFactory.
     * \a parent The parent object.
     */
    explicit IndividualMailComponentFactory(QObject *parent = nullptr);
    /*!
     * Creates a message queue job for sending iTIP messages.
     * \a incidence The incidence to create messages for.
     * \a identity The identity to use for sending.
     * \a parent The parent object.
     * \return A new MessageQueueJob instance.
     */
    Akonadi::MessageQueueJob *
    createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagementCore::Identity &identity, QObject *parent) override;

    /*!
     * Creates an iTIP handler dialog delegate.
     * \a incidence The incidence to handle.
     * \a method The iTIP method (request, reply, etc.).
     * \a parent The parent widget.
     * \return A new ITIPHandlerDialogDelegate instance.
     */
    Akonadi::ITIPHandlerDialogDelegate *
    createITIPHanderDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent) override;

public Q_SLOTS:
    /*!
     * Sets the attendees to edit for the incidence.
     * \a incidence The incidence being edited.
     * \a edit The list of attendees to edit.
     */
    void onSetEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit);
    /*!
     * Sets the attendees to update for the incidence.
     * \a incidence The incidence being updated.
     * \a update The list of attendees to update.
     */
    void onSetUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update);

private:
    QHash<QString, KCalendarCore::Attendee::List> mEdit;
    QHash<QString, KCalendarCore::Attendee::List> mUpdate;
};
}
