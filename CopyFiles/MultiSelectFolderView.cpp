#include "MultiSelectFolderView.h"
#include <QApplication>
#include <QFileIconProvider>
#include <QList>
#include <list>

MultiSelectFolderView::MultiSelectFolderView(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::Dialog);

	model.setRootPath("");
	model.iconProvider()->setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
	model.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);
	QString rootPath = qApp->applicationDirPath();
	ui.treeView_folderTree->setModel(&model);
	/*if (!rootPath.isEmpty()) {
		const QModelIndex rootIndex = model.index(QDir::cleanPath("C"));
		if (rootIndex.isValid())
			ui.treeView_folderTree->setRootIndex(rootIndex);
	}*/

	ui.treeView_folderTree->setAnimated(false);
	ui.treeView_folderTree->setIndentation(20);
	ui.treeView_folderTree->setSortingEnabled(true);
	ui.treeView_folderTree->setColumnWidth(0, this->width() / 3);
	ui.treeView_folderTree->show();

	connect(ui.button_add, SIGNAL(clicked()), this, SLOT(onBtnAddClicked()));
}

MultiSelectFolderView::~MultiSelectFolderView()
{
}

QStringList MultiSelectFolderView::getAllSelectedPaths()
{
	std::list<QString> selectedPaths;
	QModelIndexList indexList = ui.treeView_folderTree->getAllSelectedModel();
	auto it = indexList.begin();
	while (it != indexList.end())
	{
		selectedPaths.push_back(model.fileInfo(*it).absoluteFilePath());
		it++;
	}
	selectedPaths.sort();
	selectedPaths.unique();
	
	return QStringList(QList<QString>::fromStdList(selectedPaths));
}

void MultiSelectFolderView::onBtnAddClicked()
{
	emit emitBtnAddClicked();
	this->hide();
}