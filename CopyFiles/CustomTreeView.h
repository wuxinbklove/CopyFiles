#pragma once

#include <QTreeView>

class CustomTreeView : public QTreeView
{
	Q_OBJECT

public:
	CustomTreeView(QWidget *parent);
	~CustomTreeView();

	QModelIndexList getAllSelectedModel();
};
