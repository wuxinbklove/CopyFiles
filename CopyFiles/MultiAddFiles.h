#pragma once

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QApplication>
#include <QThread>
#include "ui_MultiAddFiles.h"

class SearchFilesThread : public QThread
{
	Q_OBJECT

private:
	enum SearchWay { SearchByFormat, SearchByRegular };

	SearchWay m_searchWay;
	QString m_searchPath;
	QStringList m_searchFormatList;
	bool m_isSearchSubPath;
	QRegExp m_regExp;
	QStringList m_returnList;

	void run();
	void enumAllFilesInDir(const QString& dirPath, QStringList &filePathList);

public:
	void setFormatSearch(const QString& searchPath, const QStringList& formatList, bool isSearchSubPath);
	void setRegularSearch(const QString& searchPath, const QRegExp& regExp);
	inline QStringList getReturnList() { return m_returnList; }

signals:
	void rearchFinished(const QString &s);
};


class MultiAddFiles : public QWidget
{
	Q_OBJECT

public:
	MultiAddFiles(QWidget *parent = Q_NULLPTR);
	~MultiAddFiles();
	inline QStringList getAllPaths() { return allPaths; }
	inline void quitSearchThread() { if (m_searchThread.isRunning()) { m_searchThread.quit(); } };

private:
	Ui::MultiAddFiles ui;
	SearchFilesThread m_searchThread;
	QStringList allPaths;

private slots:
	void onBtnSelectPathClicked();
	void onBtnAddClicked();
	void onRadioButtonClicked();
	void onThreadSearchFinished(const QString&);

signals:
	void emitBtnAddClicked();
	void emitSearchFinished();
};