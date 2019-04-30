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
	//设置鼠标按下可移动窗口的区域，在子窗口中必须设置该区域
	void setAreaMovable(const QRect rt);

protected:
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

private:
	QRect m_areaMovable;//可移动窗口的区域，鼠标只有在该区域按下才能移动窗口
	bool m_bPressed;//鼠标按下标志（不分左右键）
	QPoint m_ptPress;//鼠标按下的初始位置

private:
	Ui::UpgradeProgramClass ui;
};
