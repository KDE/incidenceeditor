/*
    SPDX-FileCopyrightText: 2005 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_template_management_dialog_base.h"

#include <KCalendarCore/IncidenceBase>

#include <QDialog>
namespace IncidenceEditorNG
{
class TemplateManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TemplateManagementDialog(QWidget *parent, const QStringList &templates, const QString &incidenceType);

Q_SIGNALS:
    /* Emitted whenever the user hits apply, indicating that the currently
       selected template should be loaded into to the incidence editor which
       triggered this.
    */
    void loadTemplate(const QString &templateName);

    /* Emitted whenever the user wants to add the current incidence as a
       template with the given name.
    */
    void saveTemplate(const QString &templateName);

    /* Emitted when the dialog changed the list of templates. Calling code
       can the replace the list that was handed in with the one this signal
       transports.
    */
    void templatesChanged(const QStringList &templates);

protected Q_SLOTS:
    void slotItemSelected();
    void slotAddTemplate();
    void slotRemoveTemplate();
    void slotApplyTemplate();
    void slotOk();

private:
    void slotHelp();
    void updateButtons();
    Ui::TemplateManagementDialog_base m_base;
    QStringList m_templates;
    QString m_type;
    QString m_newTemplate;
    bool m_changed = false;
};
}

