/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencedescription.h"
using namespace Qt::Literals::StringLiterals;

#include "ui_dialogdesktop.h"
#include <KPIMTextEdit/RichTextComposer>

#include "incidenceeditor_debug.h"
#include <KActionCollection>
#include <KLocalizedString>
#include <KToolBar>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG
{
class IncidenceDescriptionPrivate
{
public:
    IncidenceDescriptionPrivate() = default;

    QString mRealOriginalDescriptionEditContents;
    bool mRichTextEnabled = false;
};
}

IncidenceDescription::IncidenceDescription(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(nullptr)
    , mUi(ui)
    , d(new IncidenceDescriptionPrivate())
{
    setObjectName("IncidenceDescription"_L1);
    mUi->mRichTextLabel->setContextMenuPolicy(Qt::NoContextMenu);
    setupToolBar();
    connect(mUi->mRichTextLabel, &QLabel::linkActivated, this, &IncidenceDescription::toggleRichTextDescription);
    connect(mUi->mDescriptionEdit->richTextComposer(), &KPIMTextEdit::RichTextComposer::textChanged, this, &IncidenceDescription::checkDirtyStatus);
}

IncidenceDescription::~IncidenceDescription() = default;

void IncidenceDescription::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;

    d->mRealOriginalDescriptionEditContents.clear();

    if (incidence) {
        enableRichTextDescription(incidence->descriptionIsRich());
        if (incidence->descriptionIsRich()) {
            mUi->mDescriptionEdit->richTextComposer()->setHtml(incidence->richDescription());
            d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toHtml();
        } else {
            mUi->mDescriptionEdit->richTextComposer()->setPlainText(incidence->description());
            d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toPlainText();
        }
    } else {
        enableRichTextDescription(false);
        mUi->mDescriptionEdit->richTextComposer()->clear();
    }

    mWasDirty = false;
}

void IncidenceDescription::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    if (d->mRichTextEnabled) {
        incidence->setDescription(mUi->mDescriptionEdit->richTextComposer()->toHtml(), true);
    } else {
        incidence->setDescription(mUi->mDescriptionEdit->richTextComposer()->toPlainText(), false);
    }
}

bool IncidenceDescription::isDirty() const
{
    /* Sometimes, what you put in a KRichTextWidget isn't the same as what you get out.
       Line terminators (cr,lf) for example can be converted.

       So, to see if the user changed something, we can't compare the original incidence
       with the new editor content.

       Instead we compare the new editor content, with the original editor content, this way
       any transformation regarding non-printable chars will be irrelevant.
    */
    if (d->mRichTextEnabled) {
        return !mLoadedIncidence->descriptionIsRich() || d->mRealOriginalDescriptionEditContents != mUi->mDescriptionEdit->richTextComposer()->toHtml();
    } else {
        return mLoadedIncidence->descriptionIsRich() || d->mRealOriginalDescriptionEditContents != mUi->mDescriptionEdit->richTextComposer()->toPlainText();
    }
}

void IncidenceDescription::enableRichTextDescription(bool enable)
{
    d->mRichTextEnabled = enable;

    QString rt(i18nc("@action Enable or disable rich text editing", "Enable rich text"));
    QString placeholder(u"<a href=\"show\">%1 &gt;&gt;</a>"_s);

    if (enable) {
        rt = i18nc("@action Enable or disable rich text editing", "Disable rich text");
        placeholder = u"<a href=\"show\">&lt;&lt; %1</a>"_s;
        mUi->mDescriptionEdit->richTextComposer()->activateRichText();
        d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toHtml();
    } else {
        mUi->mDescriptionEdit->richTextComposer()->switchToPlainText();
        d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toPlainText();
    }

    placeholder = placeholder.arg(rt);
    mUi->mRichTextLabel->setText(placeholder);
    mUi->mDescriptionEdit->richTextComposer()->setEnableActions(enable);
    mUi->mEditToolBarPlaceHolder->setVisible(enable);
    checkDirtyStatus();
}

void IncidenceDescription::toggleRichTextDescription()
{
    enableRichTextDescription(!d->mRichTextEnabled);
}

void IncidenceDescription::setupToolBar()
{
#ifndef QT_NO_TOOLBAR
    auto collection = new KActionCollection(this);
    mUi->mDescriptionEdit->richTextComposer()->createActions(collection);

    auto mEditToolBar = new KToolBar(mUi->mEditToolBarPlaceHolder);
    mEditToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mEditToolBar->addAction(collection->action(u"format_text_bold"_s));
    mEditToolBar->addAction(collection->action(u"format_text_italic"_s));
    mEditToolBar->addAction(collection->action(u"format_text_underline"_s));
    mEditToolBar->addAction(collection->action(u"format_text_strikeout"_s));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(u"format_font_family"_s));
    mEditToolBar->addAction(collection->action(u"format_font_size"_s));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(u"format_text_foreground_color"_s));
    mEditToolBar->addAction(collection->action(u"format_text_background_color"_s));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(u"format_list_style"_s));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(u"format_align_left"_s));
    mEditToolBar->addAction(collection->action(u"format_align_center"_s));
    mEditToolBar->addAction(collection->action(u"format_align_right"_s));
    mEditToolBar->addAction(collection->action(u"format_align_justify"_s));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(u"format_painter"_s));
    mEditToolBar->addSeparator();
    mEditToolBar->addAction(collection->action(u"manage_link"_s));
    mUi->mDescriptionEdit->richTextComposer()->setEnableActions(false);

    auto layout = new QGridLayout(mUi->mEditToolBarPlaceHolder);
    layout->addWidget(mEditToolBar);
#endif

    // By default we don't show the rich text toolbar.
    mUi->mEditToolBarPlaceHolder->setVisible(false);
    d->mRichTextEnabled = false;
}

void IncidenceDescription::printDebugInfo() const
{
    // We're going to crash
    qCDebug(INCIDENCEEDITOR_LOG) << "RichText enabled " << d->mRichTextEnabled;

    if (mLoadedIncidence) {
        qCDebug(INCIDENCEEDITOR_LOG) << "Incidence description is rich " << mLoadedIncidence->descriptionIsRich();

        if (mLoadedIncidence->descriptionIsRich()) {
            qCDebug(INCIDENCEEDITOR_LOG) << "desc is rich, and it is <desc>" << mLoadedIncidence->richDescription() << "</desc>; "
                                         << "widget has <desc>" << mUi->mDescriptionEdit->richTextComposer()->toHtml() << "</desc>; "
                                         << "expr mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml() is "
                                         << (mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->richTextComposer()->toHtml());
        } else {
            qCDebug(INCIDENCEEDITOR_LOG) << "desc is not rich, and it is <desc>" << mLoadedIncidence->description() << "</desc>; "
                                         << "widget has <desc>" << mUi->mDescriptionEdit->richTextComposer()->toPlainText() << "</desc>; "
                                         << "expr mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText() is "
                                         << (mLoadedIncidence->description() != mUi->mDescriptionEdit->richTextComposer()->toPlainText());
        }
    } else {
        qCDebug(INCIDENCEEDITOR_LOG) << "Incidence is invalid";
    }
}

#include "moc_incidencedescription.cpp"
