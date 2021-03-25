/*
  SPDX-FileCopyrightText: 2007 Bruno Virlet <bruno.virlet@gmail.com>
  SPDX-FileCopyrightText: 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "incidenceeditor_export.h"

#include <QComboBox>
#include <QTimeZone>

namespace IncidenceEditorNG
{
/**
 * A combobox that shows the system timezones available in QTimeZone
 * and provides methods to easily select the item corresponding to a given
 * QTimeZone or to retrieve the QTimeZone associated with the
 * selected item.
 */
class INCIDENCEEDITOR_EXPORT KTimeZoneComboBox : public QComboBox
{
    Q_OBJECT
public:
    /**
     * Creates a new time zone combobox.
     *
     * @param parent The parent widget.
     */
    explicit KTimeZoneComboBox(QWidget *parent = nullptr);

    /**
     * Destroys the time zone combobox.
     */
    ~KTimeZoneComboBox() override;

    /**
     * Selects the item in the combobox corresponding to the given @p zone.
     */
    void selectTimeZone(const QTimeZone &zone);

    /**
     * Convenience version of selectTimeZone(const QTimeZone &).
     * Selects the local time zone specified in the user settings.
     */
    void selectLocalTimeZone();

    /**
     * If @p floating is true, selects floating time zone, otherwise
     * if @zone is valid, selects @pzone time zone, if not selects
     * local time zone.
     */
    void setFloating(bool floating, const QTimeZone &zone = {});

    /**
     * Return the time zone associated with the currently selected item.
     */
    Q_REQUIRED_RESULT QTimeZone selectedTimeZone() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};
}

