#pragma once

#include <QtWidgets/QDialog>
#include "ui_UpgradeProgram.h"


#include <QMouseEvent>
#include <QPoint>
#include <QRect>


class UpgradeProgram : public QDialog
{
	Q_OBJECT

public:
	UpgradeProgram(QWidget *parent = Q_NULLPTR);
	~UpgradeProgram();


private slots:
	void startProgress();
	void startExe();


public:
	//������갴�¿��ƶ����ڵ��������Ӵ����б������ø�����
	void setAreaMovable(const QRect rt);

protected:
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

private:
	QRect m_areaMovable;//���ƶ����ڵ��������ֻ���ڸ������²����ƶ�����
	bool m_bPressed;//��갴�±�־���������Ҽ���
	QPoint m_ptPress;//��갴�µĳ�ʼλ��

private:
	Ui::UpgradeProgramClass ui;
};
