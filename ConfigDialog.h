#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include <QDialog>
#include "ui_configuration.h"

class VirtualTouchScreen;

class ConfigDialog : public QDialog {
	Q_OBJECT
public:
	ConfigDialog(QWidget *parent);
private slots:
	void onBrowse();
	void onOk();
	void onCancel();
	void onApply();
private:
	Ui_Dialog ui_;
	VirtualTouchScreen *mainWnd_;
	QMap<QString,QString> icons_;
};

#endif