#include "OperateProcess.h"
#include <QApplication>
#include <QFontMetrics>

OperateProcess::OperateProcess(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//this->setWindowFlags(Qt::Window);
	ui.listWidget->setAttribute(Qt::WA_TranslucentBackground, true);
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(hide()));
	curValue = 0;
	curMin = 0;
	curMax = 1;
}

OperateProcess::~OperateProcess()
{
}

void OperateProcess::start(int minValue, int maxValue)
{
	curMin = minValue;
	curMax = maxValue;
	curValue = minValue;
	ui.progressBar->setRange(minValue, maxValue);
	ui.progressBar->setValue(minValue);
	ui.listWidget->clear();
	ui.pushButton->setEnabled(false);
}

void OperateProcess::addText(QString text, QColor textColor)
{
	QListWidgetItem *newItem = new QListWidgetItem();
	newItem->setText(text);
	newItem->setForeground(textColor);
	newItem->setToolTip(text);
	ui.listWidget->addItem(newItem);
	ui.listWidget->repaint();
	ui.listWidget->scrollToBottom();
	if (text != QString::fromLocal8Bit("操作完成"))
		ui.label->setText(QString::fromLocal8Bit("正在进行拷贝..."));
	else
	{
		ui.label->setText(QString::fromLocal8Bit("操作完成"));
		ui.pushButton->setEnabled(true);
	}
	qApp->processEvents();
}

void OperateProcess::trigger()
{
	curValue++;
	ui.progressBar->setValue(curValue);
	qApp->processEvents();
}

void OperateProcess::reset()
{
	ui.progressBar->setRange(0, 1);
	ui.progressBar->setValue(0);
	ui.listWidget->clear();
	ui.pushButton->setEnabled(false);
}
