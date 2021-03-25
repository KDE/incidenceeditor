/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <KMime/Message>

#include <KIdentityManagement/Identity>
#include <KJob>

namespace IncidenceEditorNG
{
// Opens a Composer with a mail with one attachment (constructed my ITIPHandler)
class OpenComposerJob : public KJob
{
    Q_OBJECT

public:
    explicit OpenComposerJob(QObject *parent,
                             const QString &to,
                             const QString &cc,
                             const QString &bcc,
                             const KMime::Message::Ptr &message,
                             const KIdentityManagement::Identity &identity);
    ~OpenComposerJob() override;

    void start() override;

private:
    QString mDBusService;
    QString mError;
    const QString mTo, mCc, mBcc;
    const KMime::Message::Ptr mMessage;
    const KIdentityManagement::Identity mIdentity;
};
}
