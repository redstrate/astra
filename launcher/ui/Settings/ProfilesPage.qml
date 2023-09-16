// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    title: i18n("General")

    FormCard.FormHeader {
        title: i18n("Profiles")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            model: LauncherCore.profileManager

            FormCard.FormButtonDelegate {
                required property var profile

                text: profile.name
                onClicked: applicationWindow().pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "ProfileSettings"), {
                    profile: profile
                })
            }
        }

        FormCard.FormDelegateSeparator {
            below: addProfileButton
        }

        FormCard.FormButtonDelegate {
            id: addProfileButton

            text: i18n("Add Profile")
            icon.name: "list-add"
            onClicked: {
                applicationWindow().currentSetupProfile = LauncherCore.profileManager.addProfile()
                applicationWindow().checkSetup()
            }
        }
    }
}