TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = engine \
	editor \
	app

app.depends = engine
editor.depends = engine
