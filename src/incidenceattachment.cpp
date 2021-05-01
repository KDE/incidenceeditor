/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidenceattachment.h"
#include "attachmenteditdialog.h"
#include "attachmenticonview.h"
#include "ui_dialogdesktop.h"

#include <CalendarSupport/UriHandler>

#include <KContacts/VCardDrag>

#include <KMime/Message>

#include <KActionCollection>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProtocolManager>
#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QUrl>

#include <QClipboard>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>

using namespace IncidenceEditorNG;

IncidenceAttachment::IncidenceAttachment(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(nullptr)
    , mUi(ui)
    , mPopupMenu(new QMenu)
{
    setupActions();
    setupAttachmentIconView();
    setObjectName(QStringLiteral("IncidenceAttachment"));

    connect(mUi->mAddButton, &QPushButton::clicked, this, &IncidenceAttachment::addAttachment);
    connect(mUi->mRemoveButton, &QPushButton::clicked, this, &IncidenceAttachment::removeSelectedAttachments);
}

IncidenceAttachment::~IncidenceAttachment()
{
    delete mPopupMenu;
}

void IncidenceAttachment::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    mAttachmentView->clear();

    KCalendarCore::Attachment::List attachments = incidence->attachments();
    for (KCalendarCore::Attachment::List::ConstIterator it = attachments.constBegin(), end = attachments.constEnd(); it != end; ++it) {
        new AttachmentIconItem((*it), mAttachmentView);
    }

    mWasDirty = false;
}

void IncidenceAttachment::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    incidence->clearAttachments();

    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        auto attitem = dynamic_cast<AttachmentIconItem *>(item);
        Q_ASSERT(item);
        incidence->addAttachment(attitem->attachment());
    }
}

bool IncidenceAttachment::isDirty() const
{
    if (mLoadedIncidence) {
        if (mAttachmentView->count() != mLoadedIncidence->attachments().count()) {
            return true;
        }

        KCalendarCore::Attachment::List origAttachments = mLoadedIncidence->attachments();
        for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
            QListWidgetItem *item = mAttachmentView->item(itemIndex);
            Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));

            const KCalendarCore::Attachment listAttachment = static_cast<AttachmentIconItem *>(item)->attachment();

            for (int i = 0; i < origAttachments.count(); ++i) {
                const KCalendarCore::Attachment attachment = origAttachments.at(i);

                if (attachment == listAttachment) {
                    origAttachments.remove(i);
                    break;
                }
            }
        }
        // All attachments are removed from the list, meaning, the items in mAttachmentView
        // are equal to the attachments set on mLoadedIncidence.
        return !origAttachments.isEmpty();
    } else {
        // No incidence loaded, so if the user added attachments we're dirty.
        return mAttachmentView->count() != 0;
    }
}

int IncidenceAttachment::attachmentCount() const
{
    return mAttachmentView->count();
}

/// Private slots

void IncidenceAttachment::addAttachment()
{
    QPointer<QObject> that(this);
    auto item = new AttachmentIconItem(KCalendarCore::Attachment(), mAttachmentView);

    QPointer<AttachmentEditDialog> dialog(new AttachmentEditDialog(item, mAttachmentView));
    dialog->setWindowTitle(i18nc("@title", "Add Attachment"));
    auto dialogResult = dialog->exec();
    if (!that) {
        return;
    }

    if (dialogResult == QDialog::Rejected) {
        delete item;
    } else {
        Q_EMIT attachmentCountChanged(mAttachmentView->count());
    }
    delete dialog;

    checkDirtyStatus();
}

void IncidenceAttachment::copyToClipboard()
{
#ifndef QT_NO_CLIPBOARD
    QApplication::clipboard()->setMimeData(mAttachmentView->mimeData(), QClipboard::Clipboard);
#endif
}

void IncidenceAttachment::openURL(const QUrl &url)
{
    QString uri = url.url();
    CalendarSupport::UriHandler::process(uri);
}

void IncidenceAttachment::pasteFromClipboard()
{
#ifndef QT_NO_CLIPBOARD
    handlePasteOrDrop(QApplication::clipboard()->mimeData());
#endif
}

void IncidenceAttachment::removeSelectedAttachments()
{
    QList<QListWidgetItem *> selected;
    QStringList labels;
    selected.reserve(mAttachmentView->count());
    labels.reserve(mAttachmentView->count());

    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *it = mAttachmentView->item(itemIndex);
        if (it->isSelected()) {
            auto attitem = static_cast<AttachmentIconItem *>(it);
            if (attitem) {
                const KCalendarCore::Attachment att = attitem->attachment();
                labels << att.label();
                selected << it;
            }
        }
    }

    if (selected.isEmpty()) {
        return;
    }

    QString labelsStr = labels.join(QLatin1String("<nl/>"));

    if (KMessageBox::questionYesNo(nullptr,
                                   xi18nc("@info", "Do you really want to remove these attachments?<nl/>%1", labelsStr),
                                   i18nc("@title:window", "Remove Attachments?"),
                                   KStandardGuiItem::yes(),
                                   KStandardGuiItem::no(),
                                   QStringLiteral("calendarRemoveAttachments"))
        != KMessageBox::Yes) {
        return;
    }

    for (QList<QListWidgetItem *>::iterator it(selected.begin()), end(selected.end()); it != end; ++it) {
        int row = mAttachmentView->row(*it);
        QListWidgetItem *next = mAttachmentView->item(++row);
        QListWidgetItem *prev = mAttachmentView->item(--row);
        if (next) {
            next->setSelected(true);
        } else if (prev) {
            prev->setSelected(true);
        }
        delete *it;
    }

    mAttachmentView->update();
    Q_EMIT attachmentCountChanged(mAttachmentView->count());
    checkDirtyStatus();
}

void IncidenceAttachment::saveAttachment(QListWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));

    auto attitem = static_cast<AttachmentIconItem *>(item);
    if (attitem->attachment().isEmpty()) {
        return;
    }

    KCalendarCore::Attachment att = attitem->attachment();

    // get the saveas file name
    const QString saveAsFile = QFileDialog::getSaveFileName(nullptr, i18nc("@title", "Save Attachment"), att.label());

    if (saveAsFile.isEmpty()) {
        return;
    }

    QUrl sourceUrl;
    if (att.isUri()) {
        sourceUrl = QUrl(att.uri());
    } else {
        sourceUrl = attitem->tempFileForAttachment();
    }
    // save the attachment url
    auto job = KIO::file_copy(sourceUrl, QUrl::fromLocalFile(saveAsFile));
    if (!job->exec() && job->error()) {
        KMessageBox::error(nullptr, job->errorString());
    }
}

void IncidenceAttachment::saveSelectedAttachments()
{
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            saveAttachment(item);
        }
    }
}

void IncidenceAttachment::showAttachment(QListWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));
    auto attitem = static_cast<AttachmentIconItem *>(item);
    if (attitem->attachment().isEmpty()) {
        return;
    }

    const KCalendarCore::Attachment att = attitem->attachment();
    if (att.isUri()) {
        openURL(QUrl(att.uri()));
    } else {
        auto job = new KIO::OpenUrlJob(attitem->tempFileForAttachment(), att.mimeType());
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, mAttachmentView));
        job->setDeleteTemporaryFile(true);
        job->start();
    }
}

void IncidenceAttachment::showContextMenu(const QPoint &pos)    // clazy:exclude=function-args-by-value
{
    const bool enable = mAttachmentView->itemAt(pos) != nullptr;

    int numSelected = 0;
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            numSelected++;
        }
    }

    mOpenAction->setEnabled(enable);
    // TODO: support saving multiple attachments into a directory
    mSaveAsAction->setEnabled(enable && numSelected == 1);
#ifndef QT_NO_CLIPBOARD
    mCopyAction->setEnabled(enable && numSelected == 1);
    mCutAction->setEnabled(enable && numSelected == 1);
#endif
    mDeleteAction->setEnabled(enable);
    mEditAction->setEnabled(enable);
    mPopupMenu->exec(mAttachmentView->mapToGlobal(pos));
}

void IncidenceAttachment::showSelectedAttachments()
{
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            showAttachment(item);
        }
    }
}

void IncidenceAttachment::cutToClipboard()
{
#ifndef QT_NO_CLIPBOARD
    copyToClipboard();
    removeSelectedAttachments();
#endif
}

void IncidenceAttachment::editSelectedAttachments()
{
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));

            auto attitem = static_cast<AttachmentIconItem *>(item);
            if (attitem->attachment().isEmpty()) {
                return;
            }

            QPointer<AttachmentEditDialog> dialog(new AttachmentEditDialog(attitem, mAttachmentView, false));
            dialog->setModal(false);
            dialog->setAttribute(Qt::WA_DeleteOnClose, true);
            dialog->show();
        }
    }
}

void IncidenceAttachment::slotItemRenamed(QListWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(dynamic_cast<AttachmentIconItem *>(item));
    static_cast<AttachmentIconItem *>(item)->setLabel(item->text());
    checkDirtyStatus();
}

void IncidenceAttachment::slotSelectionChanged()
{
    bool selected = false;
    for (int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex) {
        QListWidgetItem *item = mAttachmentView->item(itemIndex);
        if (item->isSelected()) {
            selected = true;
            break;
        }
    }
    mUi->mRemoveButton->setEnabled(selected);
}

/// Private functions

void IncidenceAttachment::handlePasteOrDrop(const QMimeData *mimeData)
{
    if (!mimeData) {
        return;
    }
    QList<QUrl> urls;
    bool probablyWeHaveUris = false;
    QStringList labels;

    if (KContacts::VCardDrag::canDecode(mimeData)) {
        KContacts::Addressee::List addressees;
        KContacts::VCardDrag::fromMimeData(mimeData, addressees);
        urls.reserve(addressees.count());
        labels.reserve(addressees.count());
        const KContacts::Addressee::List::ConstIterator end(addressees.constEnd());
        for (KContacts::Addressee::List::ConstIterator it = addressees.constBegin(); it != end; ++it) {
            urls.append(QUrl(QStringLiteral("uid:") + (*it).uid()));
            // there is some weirdness about realName(), hence fromUtf8
            labels.append(QString::fromUtf8((*it).realName().toLatin1()));
        }
        probablyWeHaveUris = true;
    } else if (mimeData->hasUrls()) {
        QMap<QString, QString> metadata;

        // QT5
        // urls = QList<QUrl>::fromMimeData( mimeData, &metadata );
        probablyWeHaveUris = true;
        labels = metadata[QStringLiteral("labels")].split(QLatin1Char(':'), Qt::SkipEmptyParts);
        const QStringList::Iterator end(labels.end());
        for (QStringList::Iterator it = labels.begin(); it != end; ++it) {
            *it = QUrl::fromPercentEncoding((*it).toLatin1());
        }
    } else if (mimeData->hasText()) {
        const QString text = mimeData->text();
        QStringList lst = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        urls.reserve(lst.count());
        QStringList::ConstIterator end(lst.constEnd());
        for (QStringList::ConstIterator it = lst.constBegin(); it != end; ++it) {
            urls.append(QUrl(*it));
        }
        probablyWeHaveUris = true;
    }
    QMenu menu;
    QAction *linkAction = nullptr, *cancelAction = nullptr;
    if (probablyWeHaveUris) {
        linkAction = menu.addAction(QIcon::fromTheme(QStringLiteral("insert-link")), i18nc("@action:inmenu", "&Link here"));
        // we need to check if we can reasonably expect to copy the objects
        bool weCanCopy = true;
        QList<QUrl>::ConstIterator end(urls.constEnd());
        for (QList<QUrl>::ConstIterator it = urls.constBegin(); it != end; ++it) {
            if (!(weCanCopy = KProtocolManager::supportsReading(*it))) {
                break; // either we can copy them all, or no copying at all
            }
        }
        if (weCanCopy) {
            menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "&Copy here"));
        }
    } else {
        menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "&Copy here"));
    }

    menu.addSeparator();
    cancelAction = menu.addAction(QIcon::fromTheme(QStringLiteral("process-stop")), i18nc("@action:inmenu", "C&ancel"));

    QByteArray data;
    QString mimeType;
    QString label;

    if (!mimeData->formats().isEmpty() && !probablyWeHaveUris) {
        mimeType = mimeData->formats().first();
        data = mimeData->data(mimeType);
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForName(mimeType);
        if (mime.isValid()) {
            label = mime.comment();
        }
    }

    QAction *ret = menu.exec(QCursor::pos());
    if (linkAction == ret) {
        QStringList::ConstIterator jt = labels.constBegin();
        const QList<QUrl>::ConstIterator jtEnd = urls.constEnd();
        for (QList<QUrl>::ConstIterator it = urls.constBegin(); it != jtEnd; ++it) {
            addUriAttachment((*it).url(), QString(), (jt == labels.constEnd() ? QString() : *(jt++)), true);
        }
    } else if (cancelAction != ret) {
        if (probablyWeHaveUris) {
            QList<QUrl>::ConstIterator end = urls.constEnd();
            for (QList<QUrl>::ConstIterator it = urls.constBegin(); it != end; ++it) {
                KIO::Job *job = KIO::storedGet(*it);
                // TODO verify if slot exist !
                connect(job, &KIO::Job::result, this, &IncidenceAttachment::downloadComplete);
            }
        } else { // we take anything
            addDataAttachment(data, mimeType, label);
        }
    }
}

void IncidenceAttachment::downloadComplete(KJob *)
{
    // TODO
}

void IncidenceAttachment::setupActions()
{
    auto ac = new KActionCollection(this);
    //  ac->addAssociatedWidget( this );

    mOpenAction = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18nc("@action:inmenu open the attachment in a viewer", "&Open"), this);
    connect(mOpenAction, &QAction::triggered, this, &IncidenceAttachment::showSelectedAttachments);
    ac->addAction(QStringLiteral("view"), mOpenAction);
    mPopupMenu->addAction(mOpenAction);

    mSaveAsAction =
        new QAction(QIcon::fromTheme(QStringLiteral("document-save-as")), i18nc("@action:inmenu save the attachment to a file", "Save As..."), this);
    connect(mSaveAsAction, &QAction::triggered, this, &IncidenceAttachment::saveSelectedAttachments);
    mPopupMenu->addAction(mSaveAsAction);
    mPopupMenu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    mCopyAction = KStandardAction::copy(this, &IncidenceAttachment::copyToClipboard, ac);
    mPopupMenu->addAction(mCopyAction);

    mCutAction = KStandardAction::cut(this, &IncidenceAttachment::cutToClipboard, ac);
    mPopupMenu->addAction(mCutAction);

    QAction *action = KStandardAction::paste(this, &IncidenceAttachment::pasteFromClipboard, ac);
    mPopupMenu->addAction(action);
    mPopupMenu->addSeparator();
#endif

    mDeleteAction = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18nc("@action:inmenu remove the attachment", "&Remove"), this);
    connect(mDeleteAction, &QAction::triggered, this, &IncidenceAttachment::removeSelectedAttachments);
    ac->addAction(QStringLiteral("remove"), mDeleteAction);
    mDeleteAction->setShortcut(Qt::Key_Delete);
    mPopupMenu->addAction(mDeleteAction);
    mPopupMenu->addSeparator();

    mEditAction = new QAction(QIcon::fromTheme(QStringLiteral("document-properties")),
                              i18nc("@action:inmenu show a dialog used to edit the attachment", "&Properties..."),
                              this);
    connect(mEditAction, &QAction::triggered, this, &IncidenceAttachment::editSelectedAttachments);
    ac->addAction(QStringLiteral("edit"), mEditAction);
    mPopupMenu->addAction(mEditAction);
}

void IncidenceAttachment::setupAttachmentIconView()
{
    mAttachmentView = new AttachmentIconView;
    mAttachmentView->setWhatsThis(i18nc("@info:whatsthis",
                                        "Displays items (files, mail, etc.) that "
                                        "have been associated with this event or to-do."));

    connect(mAttachmentView, &AttachmentIconView::itemDoubleClicked, this, &IncidenceAttachment::showAttachment);
    connect(mAttachmentView, &AttachmentIconView::itemChanged, this, &IncidenceAttachment::slotItemRenamed);
    connect(mAttachmentView, &AttachmentIconView::itemSelectionChanged, this, &IncidenceAttachment::slotSelectionChanged);
    connect(mAttachmentView, &AttachmentIconView::customContextMenuRequested, this, &IncidenceAttachment::showContextMenu);

    auto layout = new QGridLayout(mUi->mAttachmentViewPlaceHolder);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mAttachmentView);
}

// void IncidenceAttachmentEditor::addAttachment( KCalendarCore::Attachment *attachment )
// {
//   new AttachmentIconItem( attachment, mAttachmentView );
// }

void IncidenceAttachment::addDataAttachment(const QByteArray &data, const QString &mimeType, const QString &label)
{
    auto item = new AttachmentIconItem(KCalendarCore::Attachment(), mAttachmentView);

    QString nlabel = label;
    if (mimeType == QLatin1String("message/rfc822")) {
        // mail message. try to set the label from the mail Subject:
        KMime::Message msg;
        msg.setContent(data);
        msg.parse();
        nlabel = msg.subject()->asUnicodeString();
    }

    item->setData(data);
    item->setLabel(nlabel);
    if (mimeType.isEmpty()) {
        QMimeDatabase db;
        item->setMimeType(db.mimeTypeForData(data).name());
    } else {
        item->setMimeType(mimeType);
    }

    checkDirtyStatus();
}

void IncidenceAttachment::addUriAttachment(const QString &uri, const QString &mimeType, const QString &label, bool inLine)
{
    if (!inLine) {
        auto item = new AttachmentIconItem(KCalendarCore::Attachment(), mAttachmentView);
        item->setUri(uri);
        item->setLabel(label);
        if (mimeType.isEmpty()) {
            if (uri.startsWith(QLatin1String("uid:"))) {
                item->setMimeType(QStringLiteral("text/directory"));
            } else if (uri.startsWith(QLatin1String("kmail:"))) {
                item->setMimeType(QStringLiteral("message/rfc822"));
            } else if (uri.startsWith(QLatin1String("urn:x-ical"))) {
                item->setMimeType(QStringLiteral("text/calendar"));
            } else if (uri.startsWith(QLatin1String("news:"))) {
                item->setMimeType(QStringLiteral("message/news"));
            } else {
                QMimeDatabase db;
                item->setMimeType(db.mimeTypeForUrl(QUrl(uri)).name());
            }
        }
    } else {
        auto job = KIO::storedGet(QUrl(uri));
        KJobWidgets::setWindow(job, nullptr);
        if (job->exec()) {
            const QByteArray data = job->data();
            addDataAttachment(data, mimeType, label);
        }
    }
}
