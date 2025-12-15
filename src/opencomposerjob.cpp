/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "opencomposerjob.h"
using namespace Qt::Literals::StringLiterals;

#include <KLocalizedString>

#include <QDBusConnectionInterface>
#include <QDBusInterface>

using namespace IncidenceEditorNG;

OpenComposerJob::OpenComposerJob(QObject *parent,
                                 const QString &to,
                                 const QString &cc,
                                 const QString &bcc,
                                 const std::shared_ptr<KMime::Message> &message,
                                 const KIdentityManagementCore::Identity &identity)
    : KJob(parent)
    , mTo(to)
    , mCc(cc)
    , mBcc(bcc)
    , mMessage(message)
    , mIdentity(identity)
{
}

OpenComposerJob::~OpenComposerJob() = default;

void OpenComposerJob::start()
{
    Q_ASSERT(mMessage);

    unsigned int const identity = mIdentity.uoid();

    QString const subject = mMessage->subject()->asUnicodeString();
    QString const body = QString::fromUtf8(mMessage->contents()[0]->body());

    QList<QVariant> messages;

    if (mMessage->contents().count() == 1) {
        const QString messageFile;
        const QStringList attachmentPaths;
        const QStringList customHeaders;
        const QString replyTo;
        const QString inReplyTo;
        bool const hidden = false;

        messages << mTo << mCc << mBcc << subject << body << hidden << messageFile << attachmentPaths << customHeaders << replyTo << inReplyTo;
    } else {
        KMime::Content *attachment(mMessage->contents().at(1));
        QString const attachName = attachment->contentType()->name();
        QByteArray const attachCte = attachment->contentTransferEncoding()->as7BitString(false);
        QByteArray const attachType = attachment->contentType()->mediaType();
        QByteArray const attachSubType = attachment->contentType()->subType();
        QByteArray const attachContDisp = attachment->contentDisposition()->as7BitString(false);
        QByteArray const attachCharset = attachment->contentType()->charset();

        QByteArray const attachParamAttr = "method";
        QString const attachParamValue = attachment->contentType()->parameter("method");
        QByteArray const attachData = attachment->encodedBody();

        messages << mTo << mCc << mBcc << subject << body << attachName << attachCte << attachData << attachType << attachSubType << attachParamAttr
                 << attachParamValue << attachContDisp << attachCharset << identity;
    }

    // with D-Bus autostart, this will start kmail if it's not running yet
    QDBusInterface kmailObj(u"org.kde.kmail"_s, u"/KMail"_s, QStringLiteral("org.kde.kmail.kmail"));

    QDBusReply<int> const composerDbusPath = kmailObj.callWithArgumentList(QDBus::AutoDetect, u"openComposer"_s, messages);

    if (!composerDbusPath.isValid()) {
        setError(KJob::UserDefinedError);
        setErrorText(i18nc("errormessage: dbus is running but still no connection kmail", "Cannot connect to email service"));
    }
    emitResult();
}

#include "moc_opencomposerjob.cpp"
