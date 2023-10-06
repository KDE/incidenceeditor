/*
  SPDX-FileCopyrightText: 2007 Bruno Virlet <bruno.virlet@gmail.com>
  SPDX-FileCopyrightText: 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "incidenceeditor_export.h"

#include <QComboBox>
#include <QTimeZone>

#include <memory>

namespace IncidenceEditorNG
{
class KTimeZoneComboBoxPrivate;

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
     * Selects the item in the combobox corresponding to the zone for the given @p datetime.
     */
    void selectTimeZoneFor(const QDateTime &dateTime);

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
     * Applies the selected timezone to the given QDateTime
     * This isn't the same as dt.setTimeZone(selectedTimeZone) because
     * of the "floating" special case.
     */
    void applyTimeZoneTo(QDateTime &dt) const;

    /**
     * Return the time zone associated with the currently selected item.
     */
    [[nodiscard]] QTimeZone selectedTimeZone() const;

    /**
     * Returns true if the selecting timezone is the floating time zone
     */
    [[nodiscard]] bool isFloating() const;

private:
    //@cond PRIVATE
    std::unique_ptr<KTimeZoneComboBoxPrivate> const d;
    //@endcond
};
}
