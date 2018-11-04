import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.impl 2.4
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import se.aqba.qt.farming 1.0

ApplicationWindow {
	id: mainWindow
	visible: true
	width: 1024
	height: 768
	x: 0
	y: 0
	visibility: ApplicationWindow.AutomaticVisibility
	title: qsTr("Farming")

	Rectangle {
		id: frame
		anchors.fill: parent
		clip: true

		MapItemView {
			id: content
			anchors.fill: parent
		}
	}

	Settings {
		id: windowState
		property alias x: mainWindow.x
		property alias y: mainWindow.y
		property alias width: mainWindow.width
		property alias height: mainWindow.height
		property alias visibility: mainWindow.visibility
	}
}
