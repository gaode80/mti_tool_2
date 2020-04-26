#pragma once
/*******************************************************
author:       gaojy
create date:  2019-11-26
function:     设置基本信息窗体类
********************************************************/

#include <QWidget>
#include "ui_CSetDlg.h"

#include "Config.h"
#include "FileSystemWatcher.h"
#include "CPdfFileWatcher.h"

class mti_tool_2;

class CSetDlg : public QWidget
{
	Q_OBJECT

public:
	CSetDlg(mti_tool_2* ptool,QString path,QWidget *parent = Q_NULLPTR);
	~CSetDlg();

	void setWatchPointer(FileSystemWatcher* pfile, CPdfFileWatcher* ppdf);
	void initControl();              //配置窗口显示时，读取配置文件里的内容，填充到控件

public slots:
	void OKClicked();
	void CancelClicked();
	void SetLogPath();
	void Upload_Mode_Changed();
	void IsLogChanged();
	void SetPatientPath();
	void SetPDFPath();

private:
	void init_control();             //初始化窗体内控件的基本信息

	void set_baseinfo();             //设置基本信息
	void set_fieldinfo();            //设置存储体检者信息的表或试图的字段信息

private:
	QString m_confpath;              //配置文件路径                  
	Config m_conf;                   //操作配置文件实例

	FileSystemWatcher *m_pfile_watch;
	CPdfFileWatcher *m_pdf_watch;
	mti_tool_2* m_pmti_tool;

private:
	Ui::CSetDlg ui; 
};
