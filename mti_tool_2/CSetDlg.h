#pragma once
/*******************************************************
author:       gaojy
create date:  2019-11-26
function:     ���û�����Ϣ������
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
	void initControl();              //���ô�����ʾʱ����ȡ�����ļ�������ݣ���䵽�ؼ�

public slots:
	void OKClicked();
	void CancelClicked();
	void SetLogPath();
	void Upload_Mode_Changed();
	void IsLogChanged();
	void SetPatientPath();
	void SetPDFPath();

private:
	void init_control();             //��ʼ�������ڿؼ��Ļ�����Ϣ

	void set_baseinfo();             //���û�����Ϣ
	void set_fieldinfo();            //���ô洢�������Ϣ�ı����ͼ���ֶ���Ϣ

private:
	QString m_confpath;              //�����ļ�·��                  
	Config m_conf;                   //���������ļ�ʵ��

	FileSystemWatcher *m_pfile_watch;
	CPdfFileWatcher *m_pdf_watch;
	mti_tool_2* m_pmti_tool;

private:
	Ui::CSetDlg ui; 
};
