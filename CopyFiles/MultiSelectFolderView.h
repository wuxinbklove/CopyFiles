#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QStringList>
#include "ui_MultiSelectFolderView.h"

class MultiSelectFolderView : public QWidget
{
	Q_OBJECT

public:
	MultiSelectFolderView(QWidget *parent = Q_NULLPTR);
	~MultiSelectFolderView();
	QStringList getAllSelectedPaths();

private:
	Ui::MultiSelectFolderView ui;
	QFileSystemModel model;

private slots:
	void onBtnAddClicked();

signals:
	void emitBtnAddClicked();
};
