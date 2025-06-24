/*
  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#pragma once

#include <QObject>
using namespace Qt::Literals::StringLiterals;

class KTimeZoneComboBoxTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void test_timeSpec();
    void test_selectTimeZoneFor();
    void test_applyTimeZoneTo();
    void test_convenience();
};
