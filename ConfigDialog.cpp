#include <QFileDialog>
#include <QMessageBox>
#include "ConfigDialog.h"
#include "VirtualTouchScreen.h"

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent)
{
	ui_.setupUi(this);

	//setup
	mainWnd_ = dynamic_cast<VirtualTouchScreen*>(parent);
	if (NULL != mainWnd_) {
		ui_.doubleSpinBoxThreshold->setValue(mainWnd_->virtualScreenThreshold_);
		ui_.spinBoxOffsetX->setValue(mainWnd_->offset.x());
		ui_.spinBoxOffsetY->setValue(mainWnd_->offset.y());
		ui_.spinBoxScaleFactor->setValue(static_cast<int>(100*mainWnd_->scaleFactor));

		//setup icons list
		icons_["Fingerprint"] = ":/icons/fingerprint.png";
		icons_["Black Circle"] = ":/icons/circleclack.png";
		icons_["Blue Circle Empty"] = ":/icons/circleblueempty.png";
		icons_["Blue Circle Full"] = ":/icons/circlebluefull.png";
		icons_["Round Shape"] = ":/icons/round.png";
		QList<QString> keys = icons_.keys();
		int curIdx = icons_.count();
		int i = 0;
		foreach(const QString key, keys)
		{
			QString iconPath = icons_.value(key);
			if (iconPath == mainWnd_->fingerIcon_)
			{
				curIdx = i;
			}
			++i;
			QIcon iconImage(iconPath);
			if (!iconImage.isNull())
			{
				ui_.comboBoxFingerIcon->addItem(iconImage, key);
			}
		}
		if (curIdx >= icons_.count())
		{
			QIcon icon(mainWnd_->fingerIcon_);
			if (!icon.isNull())
			{
				int nbItems = icons_.count();
				ui_.comboBoxFingerIcon->insertItem(nbItems, icon, mainWnd_->fingerIcon_);
			}
			else
			{
				curIdx = 0;//use default icon
			}
		}
		ui_.comboBoxFingerIcon->setCurrentIndex(curIdx);
		ui_.spinBoxIconSize->setValue(mainWnd_->fingerIconSize_);
		ui_.checkBoxHideThumb->setChecked(mainWnd_->hideThumb_);
	}
	connect(ui_.pushButtonBrowse, SIGNAL(clicked(bool)), this, SLOT(onBrowse()));
	connect(ui_.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui_.buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(ui_.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(onApply()));
}

void ConfigDialog::onBrowse()
{
	QFileDialog openFile(NULL, "Select an Image File");//always in the center of the screen
	openFile.setFileMode(QFileDialog::ExistingFile);
	openFile.setViewMode(QFileDialog::Detail);
	openFile.setNameFilter("*.png *.jpg *.jpeg");
	QStringList fileNames;
	if (openFile.exec())
	{
		fileNames = openFile.selectedFiles();
		if (0 < fileNames.size())
		{
			QIcon icon(fileNames[0]);
			if (!icon.isNull())
			{
				int nbItems = icons_.count();
				if (nbItems < ui_.comboBoxFingerIcon->count()) {
					ui_.comboBoxFingerIcon->removeItem(nbItems);
				}
				ui_.comboBoxFingerIcon->insertItem(nbItems, icon, fileNames[0]);
				ui_.comboBoxFingerIcon->setCurrentIndex(nbItems);
			}
			else
			{
				QMessageBox::warning(this, "Virtual Touch Screen", "Cannot load cursor pixmap");
			}
		}
	}
}

void ConfigDialog::onOk()
{
	onApply();
	onCancel();
}

void ConfigDialog::onCancel()
{
	hide();
}

void ConfigDialog::onApply()
{
	if (NULL != mainWnd_) {
		mainWnd_->virtualScreenThreshold_ = ui_.doubleSpinBoxThreshold->value();
		mainWnd_->offset.setX(ui_.spinBoxOffsetX->value());
		mainWnd_->offset.setY(ui_.spinBoxOffsetY->value());
		mainWnd_->scaleFactor = ui_.spinBoxScaleFactor->value()/100.0;

		mainWnd_->fingerIcon_ = ui_.comboBoxFingerIcon->currentText();
		int index = ui_.comboBoxFingerIcon->currentIndex();
		if (index < icons_.count() && 0 <= index)
		{
			mainWnd_->fingerIcon_ = icons_.value(mainWnd_->fingerIcon_);
		}
		mainWnd_->fingerIconSize_ = ui_.spinBoxIconSize->value();
		mainWnd_->loadFingerIcons(mainWnd_->fingerIcon_, mainWnd_->fingerIconSize_);
		mainWnd_->hideThumb_ = ui_.checkBoxHideThumb->isChecked();
	} else {
		QMessageBox::warning(this, "Virtual Touch Screen", "Cannot apply settings");
	}
}
