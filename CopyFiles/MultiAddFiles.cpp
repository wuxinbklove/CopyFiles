#include "MultiAddFiles.h"
#include <QSettings>

void SearchFilesThread::run()
{
	m_returnList.clear();
	enumAllFilesInDir(m_searchPath, m_returnList);
	emit rearchFinished("");
}

void SearchFilesThread::enumAllFilesInDir(const QString& dirPath, QStringList &filePathList)
{
	QDir dir(dirPath);
	QFileInfoList fileInfoList = dir.entryInfoList(dir.filter() | QDir::NoDotAndDotDot);
	auto fileInfo = fileInfoList.begin();
	while (fileInfo != fileInfoList.end())
	{
		if (fileInfo->isDir())
		{
			if (m_isSearchSubPath)
				enumAllFilesInDir(fileInfo->absoluteFilePath(), filePathList);
		}
		else
		{
			QString filePath = fileInfo->absoluteFilePath();
			if (m_searchWay == SearchByFormat)
			{
				if (m_searchFormatList.contains(".*") || m_searchFormatList.contains(QString(".") + fileInfo->suffix(), Qt::CaseInsensitive))
					filePathList.append(filePath);
			}
			else if (m_searchWay == SearchByRegular)
			{
				QDir dir(m_searchPath);
				QString fileRelativePath = dir.relativeFilePath(filePath);
				if (m_regExp.exactMatch(fileRelativePath))
					filePathList.append(filePath);
			}
		}
		fileInfo++;
	}
}

void SearchFilesThread::setFormatSearch(const QString& searchPath, const QStringList& formatList, bool isSearchSubPath)
{
	m_searchWay = SearchByFormat;
	m_searchPath = searchPath;
	m_searchFormatList = formatList;
	m_isSearchSubPath = isSearchSubPath;
}

void SearchFilesThread::setRegularSearch(const QString& searchPath, const QRegExp& regExp)
{
	m_searchWay = SearchByRegular;
	m_regExp = regExp;
	m_isSearchSubPath = true;
}




MultiAddFiles::MultiAddFiles(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
	connect(ui.button_add, SIGNAL(clicked()), this, SLOT(onBtnAddClicked()));
	connect(ui.button_selectPath, SIGNAL(clicked()), this, SLOT(onBtnSelectPathClicked()));
	connect(ui.radioButton_adaptFormat, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
	connect(ui.radioButton_adaptRegular, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
	connect(&m_searchThread, SIGNAL(rearchFinished(const QString&)), this, SLOT(onThreadSearchFinished(const QString&)));
	
	QSettings histroy(QSettings::UserScope, "JinQu", "CopyFileTools", this);
	QString lastDir = histroy.value("LastSelectFolder").toString();
	if (lastDir.isEmpty())
		ui.lineEdit_path->setText(QFileInfo(qApp->applicationDirPath()).absoluteFilePath());
	else
		ui.lineEdit_path->setText(lastDir);
	ui.lineEdit_format->setText(".exe;.dll");
	ui.lineEdit_regular->setText("^Bin/.*\\.(exe|dll)$");

	onRadioButtonClicked();
}

MultiAddFiles::~MultiAddFiles()
{
}

void MultiAddFiles::onBtnSelectPathClicked()
{
	QSettings histroy(QSettings::UserScope, "JinQu", "CopyFileTools", this);
	QString lastDir = histroy.value("LastSelectFolder").toString();	
	QString dirPath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("请选择查找文件夹"), lastDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dirPath.isEmpty())
		return;
	ui.lineEdit_path->setText(dirPath);
	histroy.setValue("LastSelectFolder", dirPath);
}

void MultiAddFiles::onBtnAddClicked()
{
	QFileInfo pathInfo(ui.lineEdit_path->text());
	if (!pathInfo.isDir())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("批量添加文件"), QString::fromLocal8Bit("请选择有效的文件夹路径\t\n"));
		return;
	}

	if (ui.radioButton_adaptFormat->isChecked())
	{
		QString formatStr = ui.lineEdit_format->text();
		formatStr.replace(" ", "");
		QStringList allFormats = formatStr.split(";");
		m_searchThread.setFormatSearch(ui.lineEdit_path->text(), allFormats, ui.checkBox_includeSubPath->isChecked());
	}
	else if (ui.radioButton_adaptRegular->isChecked())
	{
		QRegExp regExp(ui.lineEdit_regular->text());
		m_searchThread.setRegularSearch(ui.lineEdit_path->text(), regExp);
	}
	m_searchThread.start();
	emitBtnAddClicked();
	this->hide();
}

void MultiAddFiles::onRadioButtonClicked()
{
	if (ui.radioButton_adaptFormat->isChecked())
	{
		ui.lineEdit_format->setEnabled(true);
		ui.checkBox_includeSubPath->setEnabled(true);
		ui.lineEdit_regular->setEnabled(false);
	}
	else if (ui.radioButton_adaptRegular->isChecked())
	{
		ui.lineEdit_format->setEnabled(false);
		ui.checkBox_includeSubPath->setEnabled(false);
		ui.lineEdit_regular->setEnabled(true);
	}
}

void MultiAddFiles::onThreadSearchFinished(const QString&)
{
	allPaths = m_searchThread.getReturnList();
	emit emitSearchFinished();
}