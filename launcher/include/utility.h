// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QNetworkRequest>

namespace Utility
{
QDir stateDirectory();
QString toWindowsPath(const QDir &dir);
void printRequest(const QString &type, const QNetworkRequest &request);
}