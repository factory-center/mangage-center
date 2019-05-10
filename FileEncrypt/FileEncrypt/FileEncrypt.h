#pragma once

#include <QtWidgets/QDialog>
#include "ui_FileEncrypt.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QString>

class FileEncrypt : public QDialog
{
	Q_OBJECT

public:
	FileEncrypt(QWidget *parent = Q_NULLPTR);
	QString filePathName;
	void setTextEdit(QString textInfo);

private slots:
	void fileEncrypt();
	void fileDecrypt();

private:
	virtual void  dragEnterEvent(QDragEnterEvent *event);
	virtual void  dropEvent(QDropEvent *event);

private:
	Ui::FileEncryptClass ui;
};
