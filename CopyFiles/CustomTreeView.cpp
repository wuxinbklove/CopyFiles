#include "CustomTreeView.h"

CustomTreeView::CustomTreeView(QWidget *parent)
	: QTreeView(parent)
{
}

CustomTreeView::~CustomTreeView()
{
}

QModelIndexList CustomTreeView::getAllSelectedModel()
{
	return QTreeView::selectedIndexes();
}