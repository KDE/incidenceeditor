/*
  Copyright (C) 2007 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright 2008-2009 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef INCIDENCEEDITOR_KTIMEZONECOMBOBOX_H
#define INCIDENCEEDITOR_KTIMEZONECOMBOBOX_H

#include "incidenceeditor_export.h"

#include <QComboBox>
#include <QTimeZone>

namespace IncidenceEditorNG {
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
    ~KTimeZoneComboBox();

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

#endif
