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
	bool isLocked;
	void closeEvent(QCloseEvent *event);
public slots:
	void OnBtnLock();
};

#endif // MUSEUMGUI_H
