#pragma once

#include <QtWidgets/QMainWindow>
#ifdef Q_OS_WIN
#include <QtWin>
#endif
#include <QSysInfo>
#include <QMenu>
#include <QFileDialog>
#include <QMap>
#include <QTreeView>
#include <QCloseEvent>
#include "ui_CopyFiles.h"
#include "OldFileItemTemplates.h"
#include "MultiAddFiles.h"
#include "MultiSelectFolderView.h"
#include "OperateProcess.h"

class CopyFiles;

class CopyFilesThread : public QThread
{
	Q_OBJECT

public:
	CopyFilesThread(CopyFiles *caller);
	enum QuestionAnswer { NoAnswer, Agree, Refuse };

private:
	CopyFiles* m_caller;
	QuestionAnswer m_questionAnswer;

	bool isShouldCopyFile(QString targetFileName);
	void run();
	void _emitProcessNeedAddText(const QString &, const QColor &color);
	void _emitProcessNeedTrigger();

signals:
	void emitProcessNeedAddText(const QString &,const QColor &color);
	void emitProcessNeedTrigger();
	void emitNeedAskAQuestion(const QString&);

public :
	inline void setQuestionAnswer(QuestionAnswer answer) { m_questionAnswer = answer; };
};


class CopyFiles : public QMainWindow
{
	Q_OBJECT

	friend class CopyFilesThread;

public:
    CopyFiles(QWidget *parent = Q_NULLPTR);

private:
    Ui::CopyFilesClass ui;
	QMenu addMenu;
	QMenu list2ContentMenu;
	MultiAddFiles multiAddSrcFiles;
	MultiSelectFolderView multiSelectFolderView;
	QMap<QString, QListWidgetItem *> curListItems;
	QMap<QString, QListWidgetItem *> curList2Item;
	OperateProcess *processView;
	CopyFilesThread copyThread;

	bool isAddSrcFilesRunning;

	void addNewListItem(QString path);
	QString getRelativePathToWorkSpace(QString srcFilePath);
	bool isShouldCopyFile(QString targetFileName);
	bool nativeEvent(const QByteArray & eventType, void * message, long * result);
	void closeEvent(QCloseEvent * event);
	bool isWindowClosable();
	void setWindowClosable(bool isClosable);

private slots:
	void onBtnAddClicked();
	void onBtnDelClicked();
	void onListItemCheckStateChanged(Qt::CheckState state);
	void onListItemNeedDeleted();
	void onAddMenuTriggered(QAction *action);
	void onMultiAddSrcFilesFinished();
	void onCheckRetainWorkSpaceClicked();
	void onBtnSelectWorkSpaceClicked();
	void onCheckForceCopyClicked();
	void onList2ContentMenuNeedShow(const QPoint&);
	void onList2ContentMenuTriggered(QAction *action);
	void onMultiSelectTargetDirsClicked();
	void onBtnApplyClicked();
	void onProcessViewNeedAddText(const QString&, const QColor&);
	void onProcessViewNeedTrigger();
	void onThreadNeedAskAQuestion(const QString&);
	
};