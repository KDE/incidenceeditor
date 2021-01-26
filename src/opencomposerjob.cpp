/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "opencomposerjob.h"

#include <KLocalizedString>

#include <QDBusConnectionInterface>
#include <QDBusInterface>

using namespace IncidenceEditorNG;

OpenComposerJob::OpenComposerJob(QObject *parent,
                                 const QString &to,
                                 const QString &cc,
                                 const QString &bcc,
                                 const KMime::Message::Ptr &message,
                                 const KIdentityManagement::Identity &identity)
    : KJob(parent)
    , mTo(to)
    , mCc(cc)
    , mBcc(bcc)
    , mMessage(message)
    , mIdentity(identity)
{
}

OpenComposerJob::~OpenComposerJob()
{
}

void OpenComposerJob::start()
{
    Q_ASSERT(mMessage);

    unsigned int identity = mIdentity.uoid();

    QString subject = mMessage->subject()->asUnicodeString();
    QString body = QString::fromUtf8(mMessage->contents()[0]->body());

    QList<QVariant> messages;

    if (mMessage->contents().count() == 1) {
        const QString messageFile;
        const QStringList attachmentPaths;
        const QStringList customHeaders;
        const QString replyTo;
        const QString inReplyTo;
        bool hidden = false;

        messages << mTo << mCc << mBcc << subject << body << hidden << messageFile << attachmentPaths << customHeaders << replyTo << inReplyTo;
    } else {
        KMime::Content *attachment(mMessage->contents().at(1));
        QString attachName = attachment->contentType()->name();
        QByteArray attachCte = attachment->contentTransferEncoding()->as7BitString(false);
        QByteArray attachType = attachment->contentType()->mediaType();
        QByteArray attachSubType = attachment->contentType()->subType();
        QByteArray attachContDisp = attachment->contentDisposition()->as7BitString(false);
        QByteArray attachCharset = attachment->contentType()->charset();

        QByteArray attachParamAttr = "method";
        QString attachParamValue = attachment->contentType()->parameter(QStringLiteral("method"));
        QByteArray attachData = attachment->encodedBody();

        messages << mTo << mCc << mBcc << subject << body << attachName << attachCte << attachData << attachType << attachSubType << attachParamAttr
                 << attachParamValue << attachContDisp << attachCharset << identity;
    }

    // with D-Bus autostart, this will start kmail if it's not running yet
    QDBusInterface kmailObj(QStringLiteral("org.kde.kmail"), QStringLiteral("/KMail"), QStringLiteral("org.kde.kmail.kmail"));

    QDBusReply<int> composerDbusPath = kmailObj.callWithArgumentList(QDBus::AutoDetect, QStringLiteral("openComposer"), messages);

    if (!composerDbusPath.isValid()) {
        setError(KJob::UserDefinedError);
        setErrorText(i18nc("errormessage: dbus is running but still no connection kmail", "Cannot connect to email service"));
    }
    emitResult();
}
