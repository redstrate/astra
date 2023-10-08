// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami

import zone.xiv.astra

Kirigami.Page {
    property var gameInstaller

    title: i18n("Logging in...")

    Kirigami.LoadingPlaceholder {
        id: placeholder

        text: "Logging in..."

        anchors.centerIn: parent
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Login Error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().pageStack.layers.pop()
        onRejected: applicationWindow().pageStack.layers.pop()
    }

    Connections {
        target: LauncherCore

        function onStageChanged(message) {
            placeholder.text = message
        }

        function onStageIndeterminate() {
            placeholder.determinate = false
        }

        function onStageDeterminate(min, max, value) {
            placeholder.determinate = true
            placeholder.progressBar.value = value
            placeholder.progressBar.from = min
            placeholder.progressBar.to = max
        }

        function onLoginError(message) {
            errorDialog.subtitle = message
            errorDialog.open()
        }
    }
}