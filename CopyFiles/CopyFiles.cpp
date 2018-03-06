#include "CopyFiles.h"
#include <QProcess>
#include <QMutex>
#include <QSettings>

#ifdef Q_OS_OSX
#import <AppKit/AppKit.h>
#endif

CopyFilesThread::CopyFilesThread(CopyFiles *caller)
{
	m_caller = caller;
	m_questionAnswer = Agree;
}

bool CopyFilesThread::isShouldCopyFile(QString targetFileName)
{
	Ui::CopyFilesClass &ui = m_caller->ui;
	if (QFile::exists(targetFileName))
	{
		if (ui.check_forceCopy->isChecked())
		{
			if (ui.radio_forceCopy->isChecked())
			{
				QFile::remove(targetFileName);
				_emitProcessNeedAddText(QString::fromLocal8Bit("检测到目标文件已存在，正在替换"), QColor(0, 255, 0));
			}
			else
			{
				_emitProcessNeedAddText(QString::fromLocal8Bit("检测到目标文件已存在，已跳过"), QColor(200, 150, 40));
				_emitProcessNeedTrigger();
				return false;
			}

		}
		else
		{
			m_questionAnswer = NoAnswer;
			emitNeedAskAQuestion(QString::fromLocal8Bit("检测到文件\"") + targetFileName + QString::fromLocal8Bit("已存在 , 是否覆盖？"));
			exec();
			if (m_questionAnswer == Agree)
			{
				QFile::remove(targetFileName);
				_emitProcessNeedAddText(QString::fromLocal8Bit("正在执行替换"), QColor(0, 255, 0));
			}
			else
			{
				_emitProcessNeedAddText(QString::fromLocal8Bit("已跳过"), QColor(200, 150, 40));
				_emitProcessNeedTrigger();
				return false;
			}
		}
	}
	int index = targetFileName.lastIndexOf('/') > targetFileName.lastIndexOf('\\') ? targetFileName.lastIndexOf('/') : targetFileName.lastIndexOf('\\');
	QString dirPath = targetFileName.left(index);
	QDir dir(dirPath);
	if (!dir.exists())
	{
		bool ret = dir.mkpath(dirPath);
		if (!ret)
		{
			_emitProcessNeedAddText(QString::fromLocal8Bit("创建所需文件夹失败") , QColor(255, 0, 0));
			_emitProcessNeedTrigger();
			return false;
		}
	}
	return true;
}

void CopyFilesThread::run()
{
	Ui::CopyFilesClass &ui = m_caller->ui;
	for (int i = 0; i < ui.listWidget->count(); i++)
	{
		OldFileItemTemplates *cellWidget = (OldFileItemTemplates *)ui.listWidget->itemWidget(ui.listWidget->item(i));
		QString targetFile;
		if (ui.check_retainWorkSpace->isChecked())
			targetFile = m_caller->getRelativePathToWorkSpace(cellWidget->getAbsolutePath());
		else
			targetFile = QFileInfo(cellWidget->getAbsolutePath()).fileName();

		for (int j = 0; j < ui.listWidget_2->count(); j++)
		{
			QString targetPath = ui.listWidget_2->item(j)->text();

			if (cellWidget->isNeedCreateLink())
			{
#ifdef Q_OS_WIN
				if (ui.radio_createWinLink->isChecked())
				{
					_emitProcessNeedAddText(QString::fromLocal8Bit("正在创建快捷方式 ") + targetPath + "/" + targetFile + ".lnk => " + cellWidget->getAbsolutePath(), QColor(10, 10, 10));
					if (!isShouldCopyFile(targetPath + "/" + targetFile + ".lnk"))
					{
						continue;
					}

					QFile::link(cellWidget->getAbsolutePath(), targetPath + "/" + targetFile + ".lnk");
				}
				else
				{
					_emitProcessNeedAddText(QString::fromLocal8Bit("正在创建链接 ") + targetPath + "/" + targetFile + " => " + cellWidget->getAbsolutePath(), QColor(10, 10, 10));
					if (!isShouldCopyFile(targetPath + "/" + targetFile))
						continue;
					STARTUPINFO si;
					memset(&si, 0, sizeof(STARTUPINFO));
					si.cb = sizeof(STARTUPINFO);
					si.dwFlags = STARTF_USESHOWWINDOW;
					si.wShowWindow = SW_HIDE;
					PROCESS_INFORMATION pi;
					if (ui.radio_createSymLink->isChecked())
					{
						QString mklinkStr = QString("\"mklink  \"\"\"") + QString(targetPath + "/" + targetFile) + QString("\"\"\" \"\"\"") + cellWidget->getAbsolutePath() + QString("\"\"\"\"");
						mklinkStr = mklinkStr.replace('/', '\\');
						if (!CreateProcess(NULL, (LPWSTR)(QString("C:\\Windows\\system32\\cmd.exe") + " /c " + mklinkStr).toStdWString().c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
						{
							continue;
						}
						else
						{
							WaitForSingleObject(pi.hProcess, INFINITE);
						}
					}
					else if (ui.radio_createHardLink->isChecked())
					{
						QString mklinkStr = QString("\"mklink  /H \"\"\"") + QString(targetPath + "/" + targetFile) + QString("\"\"\" \"\"\"") + cellWidget->getAbsolutePath() + QString("\"\"\"\"");
						mklinkStr = mklinkStr.replace('/', '\\');
						if (!CreateProcess(NULL, (LPWSTR)(QString("C:\\Windows\\system32\\cmd.exe") + " /c " + mklinkStr).toStdWString().c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
						{
							continue;
						}
						else
						{
							WaitForSingleObject(pi.hProcess, INFINITE);
						}
					}

					DWORD returnCode = 0;
					GetExitCodeProcess(pi.hProcess, &returnCode);
					if (returnCode != 0)
					{
						_emitProcessNeedAddText(QString::fromLocal8Bit("创建失败"), QColor(255, 0, 0));
					}
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}

#else defined(Q_OS_LINUX) || defined(Q_OS_OSX)
				_emitProcessNeedAddText(QString::fromLocal8Bit("正在创建链接 ") + targetPath + "/" + targetFile + " => " + cellWidget->getAbsolutePath());
				if (!isShouldCopyFile(targetPath + "/" + targetFile + ".lnk"))
					continue;

				if (ui.radio_createSymLink->isChecked())
				{
					QProcess::execute("ln", QStringList() << "-s" << cellWidget->getAbsolutePath() << QString(targetPath + "/" + targetFile));
				}
				else if (ui.radio_createHardLink->isChecked())
				{
					QProcess::execute("ln", QStringList() << cellWidget->getAbsolutePath() << QString(targetPath + "/" + targetFile));
				}
#endif
			}
			else
			{
				_emitProcessNeedAddText(QString::fromLocal8Bit("正在拷贝文件 ") + cellWidget->getAbsolutePath() + " -> " + targetPath + "/" + targetFile , QColor(10, 10, 10));
				if (!isShouldCopyFile(targetPath + "/" + targetFile))
					continue;
				QFile::copy(cellWidget->getAbsolutePath(), targetPath + "/" + targetFile);
			}
			_emitProcessNeedTrigger();
		}
	}
	_emitProcessNeedAddText(QString::fromLocal8Bit("操作完成") , QColor(10, 10, 10));
}

void CopyFilesThread::_emitProcessNeedAddText(const QString &text, const QColor &color)
{
	emit emitProcessNeedAddText(text, color);
	exec();
}

void CopyFilesThread::_emitProcessNeedTrigger()
{
	emit emitProcessNeedTrigger();
	exec();
}

CopyFiles::CopyFiles(QWidget *parent)
	: QMainWindow(parent), addMenu(this),multiAddSrcFiles(this), list2ContentMenu(this),multiSelectFolderView(this), copyThread(this)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
	this->setWindowIcon(QIcon(":/icons/logo.png"));
	ui.button_delete->hide();

	processView = new OperateProcess(this);
	processView->setWindowFlags(Qt::Window);
	processView->hide();

#ifndef Q_OS_WIN
	ui.radio_createWinLink->setEnabled(false);
#else
	if (QSysInfo::windowsVersion() <= QSysInfo::WV_2003) 
	{
		ui.radio_createSymLink->setEnabled(false);
		ui.radio_createHardLink->setEnabled(false);
		ui.radio_createWinLink->setChecked(true);
	}
#endif
	ui.listWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);

	isAddSrcFilesRunning = false;

	addMenu.addAction(QString::fromLocal8Bit("选择文件"));
	addMenu.addAction(QString::fromLocal8Bit("按条件批量选择文件"));
	connect(ui.button_add, SIGNAL(clicked()), this, SLOT(onBtnAddClicked()));
	connect(ui.button_delete, SIGNAL(clicked()), this, SLOT(onBtnDelClicked()));
	connect(&addMenu, SIGNAL(triggered(QAction *)), this, SLOT(onAddMenuTriggered(QAction *)));
	connect(&multiAddSrcFiles, SIGNAL(emitSearchFinished()), this, SLOT(onMultiAddSrcFilesFinished()));
	connect(ui.check_retainWorkSpace, SIGNAL(clicked()), this, SLOT(onCheckRetainWorkSpaceClicked()));
	connect(ui.button_selectSrcWorkSpace, SIGNAL(clicked()), this, SLOT(onBtnSelectWorkSpaceClicked()));
	connect(ui.check_forceCopy, SIGNAL(clicked()), this, SLOT(onCheckForceCopyClicked()));
	connect(ui.listWidget_2, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onList2ContentMenuNeedShow(const QPoint&)));
	connect(&list2ContentMenu, SIGNAL(triggered(QAction *)), this, SLOT(onList2ContentMenuTriggered(QAction *)));
	connect(&multiSelectFolderView, SIGNAL(emitBtnAddClicked()), this, SLOT(onMultiSelectTargetDirsClicked()));
	connect(ui.button_apply, SIGNAL(clicked()), this, SLOT(onBtnApplyClicked()));
	
	connect(&copyThread, SIGNAL(emitProcessNeedAddText(const QString&, const QColor&)), this, SLOT(onProcessViewNeedAddText(const QString&, const QColor&)));
	connect(&copyThread, SIGNAL(emitProcessNeedTrigger()), this, SLOT(onProcessViewNeedTrigger()));
	connect(&copyThread, SIGNAL(emitNeedAskAQuestion(const QString&)), this, SLOT(onThreadNeedAskAQuestion(const QString&)));
	
	onCheckRetainWorkSpaceClicked();
	onCheckForceCopyClicked();
}

void CopyFiles::addNewListItem(QString path)
{
	if (curListItems.contains(path))
	{
		return;
	}
	QListWidgetItem *listItem = new QListWidgetItem();
	listItem->setSizeHint(QSize(100, 45));
	ui.listWidget->insertItem(ui.listWidget->count(), listItem);
	OldFileItemTemplates *newItemWidget = new OldFileItemTemplates(path, this);
	ui.listWidget->setItemWidget(listItem, newItemWidget);
	ui.listWidget->scrollToBottom();
	QFileInfo fileInfo = QFileInfo(path);
	curListItems.insert(fileInfo.absoluteFilePath(), listItem);
	connect(newItemWidget, SIGNAL(emitCheckHasChanged(Qt::CheckState)), this, SLOT(onListItemCheckStateChanged(Qt::CheckState)));
	connect(newItemWidget, SIGNAL(emitItemNeedDelete()), this, SLOT(onListItemNeedDeleted()));
}

QString CopyFiles::getRelativePathToWorkSpace(QString srcFilePath)
{
	QDir dir(ui.lineEdit_srcWorkSpacePath->text());
	if (!dir.exists())
		return QString();
	return dir.relativeFilePath(srcFilePath);
}

bool CopyFiles::nativeEvent(const QByteArray & eventType, void * message, long * result)
{
	return false;
}

void CopyFiles::closeEvent(QCloseEvent * event)
{
	if (isWindowClosable())
	{
		multiAddSrcFiles.quitSearchThread();
		isAddSrcFilesRunning = false;

		if (copyThread.isRunning())
			copyThread.quit();

		event->accept();
		qApp->quit();
	}
	else
	{
		event->ignore();
	}
}

bool CopyFiles::isWindowClosable()
{
#if defined(Q_OS_WIN)
	return (::GetMenuState(::GetSystemMenu((HWND)this->winId(), false), SC_CLOSE, MF_BYCOMMAND) & MF_GRAYED) != MF_GRAYED;
#elif defined(Q_OS_OSX)
	return ([[(NSView *)(this->winId()) window] styleMask] & NSWindowStyleMaskClosable) == NSWindowStyleMaskClosable;
#else
	return (this->windowFlags() & Qt::WindowCloseButtonHint) == Qt::WindowCloseButtonHint;
#endif
}

void CopyFiles::setWindowClosable(bool isClosable)
{
	if (!isClosable)
	{
#if defined(Q_OS_WIN)
		::EnableMenuItem(::GetSystemMenu((HWND)this->winId(), false), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
#elif defined(Q_OS_OSX)
		[[(NSView *)(this->winId()) window] setStyleMask:([[(NSView *)(this->winId()) window] styleMask] & (~NSWindowStyleMaskClosable))];
#else
		this->setWindowFlags(this->windowFlags() & ~(Qt::WindowCloseButtonHint));
		this->show();
#endif
	}
	else
	{
#if defined(Q_OS_WIN)
		::EnableMenuItem(::GetSystemMenu((HWND)this->winId(), false), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
#elif defined(Q_OS_MAC)
		[[(NSView *)(this->winId()) window] setStyleMask:([[(NSView *)(this->winId()) window] styleMask] | NSWindowStyleMaskClosable)];
#else
		this->setWindowFlags(this->windowFlags() | Qt::WindowCloseButtonHint);
		this->show();
#endif
	}
}

void CopyFiles::onBtnAddClicked()
{
	addMenu.move(cursor().pos());
	addMenu.show();
}

void CopyFiles::onBtnDelClicked()
{
	onListItemNeedDeleted();
}

void CopyFiles::onListItemCheckStateChanged(Qt::CheckState state)
{
	QList<QListWidgetItem *> selectedItems = ui.listWidget->selectedItems();
	auto iter = selectedItems.begin();
	while (iter != selectedItems.end())
	{
		OldFileItemTemplates *itemWidget = (OldFileItemTemplates *)ui.listWidget->itemWidget(*iter);
		itemWidget->ui.checkBox->setCheckState(state);
		iter++;
	}
}

void CopyFiles::onListItemNeedDeleted()
{
	QList<QListWidgetItem *> selectedItems = ui.listWidget->selectedItems();
	auto iter = selectedItems.begin();
	while (iter != selectedItems.end())
	{
		OldFileItemTemplates *itemWidget = (OldFileItemTemplates *)ui.listWidget->itemWidget(*iter);
		ui.listWidget->takeItem(ui.listWidget->row(*iter));
		curListItems.remove(itemWidget->getAbsolutePath());
		disconnect(itemWidget, SIGNAL(emitCheckHasChanged(Qt::CheckState)), this, SLOT(onListItemCheckStateChanged(Qt::CheckState)));
		disconnect(itemWidget, SIGNAL(emitItemNeedDelete()), this, SLOT(onListItemNeedDeleted()));
		itemWidget->close();
		delete *iter;
		iter++;
	}
}

void CopyFiles::onAddMenuTriggered(QAction *action)
{
	if (action->text() == QString::fromLocal8Bit("选择文件"))
	{
		QString fileFilterStr(QString::fromLocal8Bit("所有文件类型 (*.*)"));
		QSettings histroy(QSettings::UserScope, "JinQu", "CopyFileTools", this);
		QString lastDir = histroy.value("LastSelectFolder").toString();
		QStringList newAddPaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("添加文件"), lastDir, fileFilterStr, &fileFilterStr, QFileDialog::DontResolveSymlinks);
		QStringList::Iterator it = newAddPaths.begin();
		while (it != newAddPaths.end()) {
			addNewListItem(*it);
			lastDir = QFileInfo(*it).absolutePath();
			++it;
		}
		histroy.setValue("LastSelectFolder", lastDir);
	}
	else
	{
		multiAddSrcFiles.show();
		multiAddSrcFiles.activateWindow();
	}
}

void CopyFiles::onMultiAddSrcFilesFinished()
{
	isAddSrcFilesRunning = true;
	//setWindowClosable(false);
	QStringList newAddPaths = multiAddSrcFiles.getAllPaths();
	QStringList::Iterator it = newAddPaths.begin();
	while (it != newAddPaths.end() && isAddSrcFilesRunning) {
		addNewListItem(*it);
		++it;
		qApp->processEvents();
	}
	ui.listWidget->setUpdatesEnabled(true);
	//setWindowClosable(true);
	isAddSrcFilesRunning = false;
}

void CopyFiles::onCheckRetainWorkSpaceClicked()
{
	bool isChecked = ui.check_retainWorkSpace->isChecked();
	ui.lineEdit_srcWorkSpacePath->setEnabled(isChecked);
	ui.button_selectSrcWorkSpace->setEnabled(isChecked);
	ui.button_selectSrcWorkSpace->setStyleSheet(isChecked ? "background:transparent ; color:blue;" : "background:transparent ; color:grey;");
}

void CopyFiles::onBtnSelectWorkSpaceClicked()
{
	QString dirPath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("请选择工作文件夹"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dirPath.isEmpty())
		return;
	ui.lineEdit_srcWorkSpacePath->setText(dirPath);
	bool isNeedWarning = false;
	for (int i = 0; i < ui.listWidget->count(); i++)
	{
		OldFileItemTemplates *cellWidget = (OldFileItemTemplates *)ui.listWidget->itemWidget(ui.listWidget->item(i));
		QString relativePath = getRelativePathToWorkSpace(cellWidget->getAbsolutePath());
		if (relativePath.left(2) == "..")
		{
			isNeedWarning = true;
			ui.listWidget->item(i)->setBackgroundColor(QColor(255, 0, 0, 80));
		}
		else
		{
			ui.listWidget->item(i)->setBackgroundColor(QColor(255, 0, 0, 0));
		}
	}
	if (isNeedWarning)
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("检测到有选择的源文件在工作路径外，已在条目上用浅红色背景注明，请确认！"));
}

void CopyFiles::onCheckForceCopyClicked()
{
	ui.widget_forceCopy->setEnabled(ui.check_forceCopy->isChecked());
}

void CopyFiles::onList2ContentMenuNeedShow(const QPoint&pos)
{
	list2ContentMenu.clear();
	list2ContentMenu.addAction(QString::fromLocal8Bit("添加"));
	if (ui.listWidget_2->itemAt(pos) != NULL)
	{
		list2ContentMenu.addAction(QString::fromLocal8Bit("移除"));
	}
	list2ContentMenu.move(cursor().pos());
	list2ContentMenu.show();
}

void CopyFiles::onList2ContentMenuTriggered(QAction *action)
{
	if (action->text() == QString::fromLocal8Bit("添加"))
	{
		multiSelectFolderView.setWindowModality(Qt::WindowModal);
		multiSelectFolderView.show();
		multiSelectFolderView.activateWindow();
	}
	else if (action->text() == QString::fromLocal8Bit("移除"))
	{
		QList<QListWidgetItem *>selectedItems = ui.listWidget_2->selectedItems();
		auto iter = selectedItems.begin();
		while (iter != selectedItems.end())
		{
			QString path = (*iter)->text();
			curList2Item.remove(path);
			ui.listWidget_2->takeItem(ui.listWidget_2->row(*iter));
			delete *iter;
			iter++;
		}
	}
}

void CopyFiles::onMultiSelectTargetDirsClicked()
{
	QStringList newAddPaths = multiSelectFolderView.getAllSelectedPaths();
	QStringList::Iterator it = newAddPaths.begin();
	while (it != newAddPaths.end())
	{
		QString newPath = QFileInfo(*it).absoluteFilePath();
		if (!curList2Item.contains(newPath))
		{
			QListWidgetItem *item = new QListWidgetItem(newPath);
			ui.listWidget_2->insertItem(ui.listWidget_2->count(), item);
			curList2Item.insert(newPath, item);
		}
		++it;
	}
}

bool CopyFiles::isShouldCopyFile(QString targetFileName)
{
	if (QFile::exists(targetFileName))
	{
		if (ui.check_forceCopy->isChecked())
		{
			if (ui.radio_forceCopy->isChecked())
			{
				QFile::remove(targetFileName);
				processView->addText(QString::fromLocal8Bit("检测到目标文件已存在，正在替换") , QColor(0 , 255 , 0));
			}
			else
			{
				processView->addText(QString::fromLocal8Bit("检测到目标文件已存在，已跳过"), QColor(200, 150, 40));
				processView->trigger();
				return false;
			}

		}
		else
		{
			if (QMessageBox::Yes == QMessageBox::question(this, QString(), QString::fromLocal8Bit("检测到文件\"") + targetFileName + QString::fromLocal8Bit("已存在 , 是否覆盖？")))
			{
				QFile::remove(targetFileName);
				processView->addText(QString::fromLocal8Bit("正在执行替换"), QColor(0, 255, 0));
			}
			else
			{
				processView->addText(QString::fromLocal8Bit("已跳过"), QColor(200, 150, 40));
				processView->trigger();
				return false;
			}
		}
	}
	int index = targetFileName.lastIndexOf('/') > targetFileName.lastIndexOf('\\') ? targetFileName.lastIndexOf('/') : targetFileName.lastIndexOf('\\');
	QString dirPath = targetFileName.left(index);
	QDir dir(dirPath);
	if (!dir.exists())
	{
		bool ret = dir.mkpath(dirPath);
		if (!ret)
		{
			processView->addText(QString::fromLocal8Bit("创建所需文件夹失败"), QColor(255, 0, 0));
			processView->trigger();
			return false;
		}
	}
	return true;
}

void CopyFiles::onBtnApplyClicked()
{
	if (isAddSrcFilesRunning)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("拷贝失败"), QString::fromLocal8Bit("程序正忙， 程序正在读取源文件列表\t\n"));
		return;
	}
	
	if (ui.check_retainWorkSpace->isChecked())
	{
		QFileInfo pathInfo(ui.lineEdit_srcWorkSpacePath->text());
		if (!pathInfo.isDir())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("指定源文件工作路径"), QString::fromLocal8Bit("请选择有效的文件夹路径\t\n"));
			return;
		}
	}
	processView->start(0, ui.listWidget->count() * ui.listWidget_2->count());
	processView->setWindowModality(Qt::WindowModal);
	processView->show();
	copyThread.start();
}

void CopyFiles::onProcessViewNeedAddText(const QString &text, const QColor &color)
{
	processView->addText(text , color);
	copyThread.exit();
}

void CopyFiles::onProcessViewNeedTrigger()
{
	processView->trigger();
	copyThread.exit();
}

void CopyFiles::onThreadNeedAskAQuestion(const QString& question)
{
	if (QMessageBox::Yes == QMessageBox::question(0, QString(), question))
		copyThread.setQuestionAnswer(CopyFilesThread::Agree);
	else
		copyThread.setQuestionAnswer(CopyFilesThread::Refuse);
	copyThread.exit();
}

