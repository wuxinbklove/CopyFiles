#include "OldFileItemTemplates.h"
#include <QFontMetrics>

OldFileItemTemplates::OldFileItemTemplates(QString filePath , QWidget *parent)
	: QWidget(parent) , contextMenu(this)
{
	ui.setupUi(this);
	fileInfo = QFileInfo(filePath);

	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	this->setToolTip(fileInfo.absoluteFilePath());
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	contextMenu.addAction(QString::fromLocal8Bit("É¾³ýËùÑ¡¼ÇÂ¼"));

	QString fileName = ui.label_fileName->fontMetrics().elidedText(fileInfo.fileName(), Qt::ElideRight, ui.label_fileName->width());
	ui.label_fileName->setText(fileName);
	QString fullPath = ui.label_filePath->fontMetrics().elidedText(fileInfo.absoluteFilePath(), Qt::ElideRight, ui.label_filePath->width());
	ui.label_filePath->setText(fullPath);

	connect(ui.checkBox, SIGNAL(clicked()), this, SLOT(onCheckClicked()));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContentMenuNeedShow(const QPoint&)));
	connect(&contextMenu, SIGNAL(triggered(QAction *)), this, SLOT(onContextMenuTriggered(QAction *)));
}

OldFileItemTemplates::~OldFileItemTemplates()
{
	
}

void OldFileItemTemplates::onCheckClicked()
{
	Qt::CheckState state = ui.checkBox->checkState();
	emit emitCheckHasChanged(state);
}

void OldFileItemTemplates::onContentMenuNeedShow(const QPoint& pos)
{
	contextMenu.move(cursor().pos());
	contextMenu.show();
}

void OldFileItemTemplates::onContextMenuTriggered(QAction *)
{
	emit emitItemNeedDelete();
}