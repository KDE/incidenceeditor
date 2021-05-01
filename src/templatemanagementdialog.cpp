/*
    SPDX-FileCopyrightText: 2005 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "templatemanagementdialog.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <QInputDialog>

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;

TemplateManagementDialog::TemplateManagementDialog(QWidget *parent, const QStringList &templates, const QString &incidenceType)
    : QDialog(parent)
    , m_templates(templates)
    , m_type(incidenceType)
{
    QString m_type_translated = i18n(qPrintable(m_type));
    setWindowTitle(i18nc("@title:window", "Manage %1 Templates", m_type_translated));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    auto mainLayout = new QVBoxLayout(this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TemplateManagementDialog::reject);
    setObjectName(QStringLiteral("template_management_dialog"));
    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &TemplateManagementDialog::slotHelp);
    auto widget = new QWidget(this);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);

    widget->setObjectName(QStringLiteral("template_management_dialog_base"));
    m_base.setupUi(widget);

    m_base.m_listBox->addItems(m_templates);
    m_base.m_listBox->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_base.m_buttonAdd, &QPushButton::clicked, this, &TemplateManagementDialog::slotAddTemplate);
    connect(m_base.m_buttonRemove, &QPushButton::clicked, this, &TemplateManagementDialog::slotRemoveTemplate);
    connect(m_base.m_buttonApply, &QPushButton::clicked, this, &TemplateManagementDialog::slotApplyTemplate);

    connect(m_base.m_listBox, &QListWidget::itemSelectionChanged, this, &TemplateManagementDialog::slotItemSelected);
    connect(m_base.m_listBox, &QListWidget::itemDoubleClicked, this, &TemplateManagementDialog::slotApplyTemplate);
    connect(okButton, &QPushButton::clicked, this, &TemplateManagementDialog::slotOk);

    m_base.m_buttonRemove->setEnabled(false);
    m_base.m_buttonApply->setEnabled(false);
}

void TemplateManagementDialog::slotHelp()
{
    QUrl url = QUrl(QStringLiteral("help:/")).resolved(QUrl(QStringLiteral("korganizer/entering-data.html")));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("anchor"), QStringLiteral("entering-data-events-template-buttons"));
    url.setQuery(query);
    // launch khelpcenter, or a browser for URIs not handled by khelpcenter
    QDesktopServices::openUrl(url);
}

void TemplateManagementDialog::slotItemSelected()
{
    m_base.m_buttonRemove->setEnabled(true);
    m_base.m_buttonApply->setEnabled(true);
}

void TemplateManagementDialog::slotAddTemplate()
{
    bool ok;
    bool duplicate = false;
    QString m_type_translated = i18n(qPrintable(m_type));
    const QString newTemplate = QInputDialog::getText(this,
                                                      i18n("Template Name"),
                                                      i18n("Please enter a name for the new template:"),
                                                      QLineEdit::Normal,
                                                      i18n("New %1 Template", m_type_translated),
                                                      &ok);
    if (newTemplate.isEmpty() || !ok) {
        return;
    }

    if (m_templates.contains(newTemplate)) {
        int rc = KMessageBox::warningContinueCancel(this,
                                                    i18n("A template with that name already exists, do you want to overwrite it?"),
                                                    i18n("Duplicate Template Name"),
                                                    KStandardGuiItem::overwrite());
        if (rc == KMessageBox::Cancel) {
            QTimer::singleShot(0, this, &TemplateManagementDialog::slotAddTemplate);
            return;
        }
        duplicate = true;
    }

    if (!duplicate) {
        int count = m_base.m_listBox->count();
        m_templates.append(newTemplate);
        m_base.m_listBox->addItem(newTemplate);
        QListWidgetItem *item = m_base.m_listBox->item(count);
        item->setSelected(true);
    }
    m_newTemplate = newTemplate;
    m_changed = true;

    // From this point on we need to keep the original event around until the
    // user has closed the dialog, applying a template would make little sense
    // buttonBox->button(QDialogButtonBox::Apply)->setEnabled( false );
    // neither does adding it again
    m_base.m_buttonAdd->setEnabled(false);
}

void TemplateManagementDialog::slotRemoveTemplate()
{
    QListWidgetItem *const item = m_base.m_listBox->selectedItems().first();
    if (!item) {
        return; // can't happen (TM)
    }

    int rc = KMessageBox::warningContinueCancel(this,
                                                i18n("Are you sure that you want to remove the template <b>%1</b>?", item->text()),
                                                i18n("Remove Template"),
                                                KStandardGuiItem::remove());

    if (rc == KMessageBox::Cancel) {
        return;
    }

    int current = m_base.m_listBox->row(item);

    m_templates.removeAll(item->text());
    m_base.m_listBox->takeItem(current);
    QListWidgetItem *newItem = m_base.m_listBox->item(qMax(current - 1, 0));
    if (newItem) {
        newItem->setSelected(true);
    }

    updateButtons();

    m_changed = true;
}

void TemplateManagementDialog::updateButtons()
{
    m_base.m_buttonAdd->setEnabled(true);
    const bool isNotEmpty = m_base.m_listBox->count() != 0;
    m_base.m_buttonRemove->setEnabled(isNotEmpty);
    m_base.m_buttonApply->setEnabled(isNotEmpty);
}

void TemplateManagementDialog::slotApplyTemplate()
{
    // Once the user has applied the current template to the event,
    // it makes no sense to add it again
    m_base.m_buttonAdd->setEnabled(false);
    QListWidgetItem *item = m_base.m_listBox->currentItem();
    if (item) {
        const QString &cur = item->text();
        if (!cur.isEmpty() && cur != m_newTemplate) {
            Q_EMIT loadTemplate(cur);
            slotOk();
        }
    }
}

void TemplateManagementDialog::slotOk()
{
    // failure is not an option *cough*
    if (!m_newTemplate.isEmpty()) {
        Q_EMIT saveTemplate(m_newTemplate);
    }
    if (m_changed) {
        Q_EMIT templatesChanged(m_templates);
    }
    accept();
}
