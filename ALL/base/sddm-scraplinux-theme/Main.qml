import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
	id: root
	width: 1280
	height: 720
	color: "#0a1c38"

	Column {
		anchors.centerIn: parent
		spacing: 24
		width: 360

		Text {
			text: "SCRAPLINUX LINUX"
			color: "#8fb8ff"
			font.pixelSize: 30
			font.bold: true
			font.letterSpacing: 2
			anchors.horizontalCenter: parent.horizontalCenter
		}

		ComboBox {
			id: userCombo
			width: parent.width
			model: userModel
			textRole: "name"
			currentIndex: userModel.lastIndex
		}

		TextField {
			id: passwordField
			width: parent.width
			placeholderText: "Password"
			echoMode: TextInput.Password
			focus: true
			onAccepted: sddm.login(userCombo.currentText, passwordField.text, sessionCombo.currentIndex)
		}

		ComboBox {
			id: sessionCombo
			width: parent.width
			model: sessionModel
			textRole: "name"
			currentIndex: sessionModel.lastIndex
		}

		Button {
			text: "Log In"
			width: parent.width
			onClicked: sddm.login(userCombo.currentText, passwordField.text, sessionCombo.currentIndex)
		}

		Row {
			spacing: 16
			anchors.horizontalCenter: parent.horizontalCenter
			Button {
				text: "Shutdown"
				visible: sddm.canPowerOff
				onClicked: sddm.powerOff()
			}
			Button {
				text: "Restart"
				visible: sddm.canReboot
				onClicked: sddm.reboot()
			}
		}
	}

	Connections {
		target: sddm
		function onLoginFailed() {
			passwordField.text = ""
			passwordField.focus = true
		}
	}

	Component.onCompleted: passwordField.forceActiveFocus()
}
