/*
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0

  NOTE: May, 2010. Extracted this code from
        kdepim/incidenceeditors/editorattachments.{h,cpp}
*/

#include <config-enterprise.h>

#include "attachmenticonview.h"
#include "incidenceeditor_debug.h"

#include <KIconLoader>
#include <KIconUtils>
#include <KUrlMimeData>
#include <QDir>
#include <QTemporaryFile>

#include <QDrag>
#include <QKeyEvent>
#include <QMimeData>
#include <QMimeDatabase>
using namespace Qt::Literals::StringLiterals;
using namespace IncidenceEditorNG;

AttachmentIconItem::AttachmentIconItem(const KCalendarCore::Attachment &att, QListWidget *parent)
    : QListWidgetItem(parent)
{
    if (!att.isEmpty()) {
        mAttachment = att;
    } else {
        // for the enterprise, inline attachments are the default
#if KDEPIM_ENTERPRISE_BUILD
        mAttachment = KCalendarCore::Attachment(QByteArray()); // use the non-uri constructor
        // as we want inline by default
#else
        mAttachment = KCalendarCore::Attachment(QString());
#endif
    }
    readAttachment();
    setFlags(flags() | Qt::ItemIsDragEnabled);
}

AttachmentIconItem::~AttachmentIconItem() = default;

KCalendarCore::Attachment AttachmentIconItem::attachment() const
{
    return mAttachment;
}

QString AttachmentIconItem::uri() const
{
    return mAttachment.uri();
}

QString AttachmentIconItem::savedUri() const
{
    return mSaveUri;
}

void AttachmentIconItem::setUri(const QString &uri)
{
    mSaveUri = uri;
    mAttachment.setUri(mSaveUri);
    readAttachment();
}

void AttachmentIconItem::setData(const QByteArray &data)
{
    mAttachment.setDecodedData(data);
    readAttachment();
}

QString AttachmentIconItem::mimeType() const
{
    return mAttachment.mimeType();
}

void AttachmentIconItem::setMimeType(const QString &mime)
{
    mAttachment.setMimeType(mime);
    readAttachment();
}

QString AttachmentIconItem::label() const
{
    return mAttachment.label();
}

void AttachmentIconItem::setLabel(const QString &description)
{
    if (mAttachment.label() == description) {
        return;
    }
    mAttachment.setLabel(description);
    readAttachment();
}

bool AttachmentIconItem::isBinary() const
{
    return mAttachment.isBinary();
}

QIcon AttachmentIconItem::icon() const
{
    QMimeDatabase const db;
    return icon(db.mimeTypeForName(mAttachment.mimeType()), mAttachment.uri(), mAttachment.isBinary());
}

QIcon AttachmentIconItem::icon(const QMimeType &mimeType, const QString &uri, bool binary)
{
    const QString iconStr = mimeType.iconName();
    QStringList overlays;
    if (!uri.isEmpty() && !binary) {
        overlays << u"emblem-link"_s;
    }

    return KIconUtils::addOverlays(QIcon::fromTheme(iconStr), overlays);
}

void AttachmentIconItem::readAttachment()
{
    setText(mAttachment.label());
    setFlags(flags() | Qt::ItemIsEditable);

    QMimeDatabase const db;
    if (mAttachment.mimeType().isEmpty() || !(db.mimeTypeForName(mAttachment.mimeType()).isValid())) {
        QMimeType attachmentMimeType;
        if (mAttachment.isUri()) {
            attachmentMimeType = db.mimeTypeForUrl(QUrl(mAttachment.uri()));
        } else {
            attachmentMimeType = db.mimeTypeForData(mAttachment.decodedData());
        }
        mAttachment.setMimeType(attachmentMimeType.name());
    }

    setIcon(icon());
}

AttachmentIconView::AttachmentIconView(QWidget *parent)
    : QListWidget(parent)
{
    setMovement(Static);
    setAcceptDrops(true);
    setSelectionMode(ExtendedSelection);
    setSelectionRectVisible(false);
    setIconSize(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge));
    setFlow(LeftToRight);
    setWrapping(true);
#ifndef QT_NO_DRAGANDDROP
    setDragDropMode(DragDrop);
    setDragEnabled(true);
#endif
    setEditTriggers(EditKeyPressed);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

QUrl AttachmentIconItem::tempFileForAttachment()
{
    if (mTempFile.isValid()) {
        return mTempFile;
    }
    QTemporaryFile *file = nullptr;

    QMimeDatabase const db;
    QStringList patterns = db.mimeTypeForName(mAttachment.mimeType()).globPatterns();

    if (!patterns.empty()) {
        file = new QTemporaryFile(QDir::tempPath() + "/attachementview_XXXXX"_L1 + patterns.first().remove(u'*'));
    } else {
        file = new QTemporaryFile();
    }
    file->setParent(listWidget());

    if (!file->open()) {
        qCWarning(INCIDENCEEDITOR_LOG) << " Impossible to create temporary file";
        return {};
    }
    file->setAutoRemove(true);
    // read-only not to give the idea that it could be written to
    file->setPermissions(QFile::ReadUser);
    file->write(QByteArray::fromBase64(mAttachment.data()));
    mTempFile = QUrl::fromLocalFile(file->fileName());
    file->close();
    return mTempFile;
}

QMimeData *AttachmentIconView::mimeData(const QList<QListWidgetItem *> &items) const // clazy:exclude=function-args-by-ref
{
    // create a list of the URL:s that we want to drag
    QList<QUrl> urls;
    QStringList labels;
    for (QListWidgetItem *it : items) {
        if (it->isSelected()) {
            auto item = static_cast<AttachmentIconItem *>(it);
            if (item->isBinary()) {
                urls.append(item->tempFileForAttachment());
            } else {
                urls.append(QUrl(item->uri()));
            }
            labels.append(QString::fromLatin1(QUrl::toPercentEncoding(item->label())));
        }
    }
    if (selectionMode() == NoSelection) {
        auto item = static_cast<AttachmentIconItem *>(currentItem());
        if (item) {
            urls.append(QUrl(item->uri()));
            labels.append(QString::fromLatin1(QUrl::toPercentEncoding(item->label())));
        }
    }

    QMap<QString, QString> metadata;
    metadata[u"labels"_s] = labels.join(u':');

    auto mimeData = new QMimeData;
    mimeData->setUrls(urls);
    KUrlMimeData::setMetaData(metadata, mimeData);
    return mimeData;
}

QMimeData *AttachmentIconView::mimeData() const
{
    return mimeData(selectedItems());
}

void AttachmentIconView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions)
#ifndef QT_NO_DRAGANDDROP
    QPixmap pixmap;
    if (selectedItems().size() > 1) {
        pixmap = QIcon::fromTheme(u"mail-attachment"_s).pixmap(64);
    }
    if (pixmap.isNull()) {
        pixmap = static_cast<AttachmentIconItem *>(currentItem())->icon().pixmap(64);
    }

    const QPoint hotspot(pixmap.width() / 2, pixmap.height() / 2);

    auto drag = new QDrag(this);
    drag->setMimeData(mimeData());

    drag->setPixmap(pixmap);
    drag->setHotSpot(hotspot);
    drag->exec(Qt::CopyAction);
#endif
}

void AttachmentIconView::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && currentItem() && state() != EditingState) {
        Q_EMIT itemDoubleClicked(currentItem()); // ugly, but itemActivated() also includes single click
        return;
    }
    QListWidget::keyPressEvent(event);
}

Qt::DropActions AttachmentIconView::supportedDropActions() const
{
    return Qt::CopyAction;
}

QStringList AttachmentIconView::mimeTypes() const
{
    return {QStringLiteral("text/uri-list")};
}

bool AttachmentIconView::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
    Q_UNUSED(index);
    if (action == Qt::IgnoreAction) {
        return true;
    }
    Q_EMIT dropMimeDataRequested(data);
    return true;
}

#include "moc_attachmenticonview.cpp"
