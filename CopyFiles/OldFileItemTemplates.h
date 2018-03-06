#pragma once

#include <QWidget>
#include <QFileInfo>
#include <QMenu>
#include "ui_OldFileItemTemplates.h"


class OldFileItemTemplates : public QWidget
{
	Q_OBJECT

public:
	OldFileItemTemplates(QString filePath ,QWidget *parent = Q_NULLPTR);
	~OldFileItemTemplates();
	inline QString getAbsolutePath() { return fileInfo.absoluteFilePath(); }
	inline bool isNeedCreateLink() { return ui.checkBox->isChecked(); }

	Ui::OldFileItemTemplates ui;

private:
	QFileInfo fileInfo;
	QMenu contextMenu;

private slots:
	void onCheckClicked();
	void onContentMenuNeedShow(const QPoint&);
	void onContextMenuTriggered(QAction *);

signals:
	void emitCheckHasChanged(Qt::CheckState);
	void emitItemNeedDelete();
};
