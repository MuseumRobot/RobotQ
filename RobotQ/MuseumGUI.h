#ifndef MUSEUMGUI_H
#define MUSEUMGUI_H

#include <QDialog>
#include "ui_MuseumGUI.h"

class MuseumGUI : public QDialog{
	Q_OBJECT

public:
	MuseumGUI(QWidget *parent = 0);
	~MuseumGUI();
	Ui::MuseumGUI ui;
	void closeEvent(QCloseEvent *event);
};

#endif // MUSEUMGUI_H
