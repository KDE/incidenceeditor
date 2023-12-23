#! /bin/sh
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none

$EXTRACTRC `find . -name \*.ui -o -name \*.kcfg`  >> rc.cpp
$XGETTEXT `find . -name "*.cpp" -o -name "*.h" | grep -v '/tests/'`  -o $podir/libincidenceeditors6.pot
rm -f rc.cpp
