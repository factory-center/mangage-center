#include "FileEncrypt.h"
#include "FileOpt.h"
#include <io.h>
#include <QMimeData>
#include <QMessageBox>
#include <QDateTime>
#include <QTextCursor>


FileEncrypt::FileEncrypt(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
	this->setAcceptDrops(true);
	this->setWindowTitle(QString::fromLocal8Bit("文件加解密工具 HRG"));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(fileEncrypt()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(fileDecrypt()));
}
void FileEncrypt::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}
void FileEncrypt::dropEvent(QDropEvent *event)
{
	QString name = event->mimeData()->urls().first().toString();
	ui.lineEdit->setText(name);
}


void FileEncrypt::fileEncrypt()
{
	QString filePathName = ui.lineEdit->text();
	if (filePathName.length() == 0)
	{
		QMessageBox::information(this, NULL, QString::fromLocal8Bit("文件不能为空"),QMessageBox::Ok);
		return;
	}
	if (filePathName.contains("///", Qt::CaseSensitive) == true)
	{
		filePathName = filePathName.mid(8);
	}
	filePathName = filePathName.replace("/", "\\");
	if (_access(filePathName.toLocal8Bit().data(), 0) == -1)
	{
		QMessageBox::information(this, NULL, QString::fromLocal8Bit("文件不存在"), QMessageBox::Ok);
		return;
	}

	if (encrypt(filePathName.toLocal8Bit().data(), 123321) == false)
	{
		QMessageBox::information(this, NULL, QString::fromLocal8Bit("文件异常"), QMessageBox::Ok);
		return;
	}
	

	setTextEdit(">>> " + filePathName);
	setTextEdit("<<< " + filePathName + "E");
	setTextEdit(QString::fromLocal8Bit("加密完成"));
	ui.lineEdit->clear();
}
void FileEncrypt::fileDecrypt()
{
	QString filePathName = ui.lineEdit->text();
	if (filePathName.length() == 0)
	{
		QMessageBox::information(this, NULL, QString::fromLocal8Bit("文件不能为空"), QMessageBox::Ok);
		return;
	}

	if (filePathName.contains("///", Qt::CaseSensitive) == true)
	{
		filePathName = filePathName.mid(8);
	}
	filePathName = filePathName.replace("/", "\\");

	if (_access(filePathName.toLocal8Bit().data(), 0) == -1)
	{
		QMessageBox::information(this, NULL, QString::fromLocal8Bit("文件不存在"), QMessageBox::Ok);
		return;
	}
	

	if (decrypt(filePathName.toLocal8Bit().data(), 123321) == false)
	{
		QMessageBox::information(this, NULL, QString::fromLocal8Bit("文件异常"), QMessageBox::Ok);
		return;
	}
	
	setTextEdit(">>> " + filePathName);
	setTextEdit("<<< " + filePathName.mid(0, filePathName.length()-1));
	setTextEdit(QString::fromLocal8Bit("解密完成"));
	ui.lineEdit->clear();
}

void FileEncrypt::setTextEdit(QString textInfo)
{

	QDateTime currentDateTime = QDateTime::currentDateTime();
	QString currentTime = currentDateTime.toString("hh:mm:ss");
	ui.textEdit->moveCursor(QTextCursor::Start);
	ui.textEdit->insertPlainText(currentTime + " " + textInfo + "\n");
}