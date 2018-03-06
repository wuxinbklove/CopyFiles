#pragma once

#include <QWidget>
#include "ui_OperateProcess.h"

class OperateProcess : public QWidget
{
	Q_OBJECT

public:
	OperateProcess(QWidget *parent = Q_NULLPTR);
	~OperateProcess();
	Ui::OperateProcess ui;

private:
	int curValue;
	int curMax;
	int curMin;

public:
	void start(int minValue, int maxValue);
	void addText(QString text , QColor textColor = QColor(10 , 10 ,10));
	void trigger();
	void reset();
};
