#pragma once
#include "CLog.h"
#include "Config.h"

#include <QObject>
#include <QFileSystemWatcher>
#include <QMap>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include <QDir>
#include <QSet>
#include <QFileInfo>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <string>
using namespace std;
/*******************************************************
author:       gaojy
create date:  2019-10-28
function:     ���ָ��·���µ�PDF�ļ������ϴ���FTP������
********************************************************/
class CPdfFileWatcher : public QObject
{
	Q_OBJECT
		
//private:
public:
	explicit CPdfFileWatcher(int islog,QString basepath,QObject* parent = 0);
	~CPdfFileWatcher();

public:
	void addWatchPath(QString spath);
	void modifyWatchPath(QString path);
	void set_write_log(bool flag);

public:
	void setReport_path(QString spath) { m_report_path = spath; }

public slots:
	void directoryUpdated(const QString& path);
	void OnFinishRelay();

private:
	QString getPatientID(QString spath);                  //�Ӳ���������ȡ�����˵ļ���
	
	void HUBEI10000_PDF(QString path,QString filename);
	void HEBEI11000_PDF(QString path, QString filename);
	void SHANXI12000_PDF(QString path, QString filename);
	string SHANXI12000_Create_Folder(QString patient_id,QString sequence_no);
	string SHANXI12000_Get_Filename(string path);

	bool connect_db();
	void close_db();
	QString get_date_byfilename(QString filename);        //���ļ�����ȡ������

private:
	QFileSystemWatcher* m_pPDFWatcher;
	QMap<QString, QStringList> m_currentContentMap;

	//ftp�����Ϣ
	QString m_sftpIp;
	QString m_ftpuser;
	QString m_ftppsw;

	int m_ftpport;
	//end ftp�����Ϣ
    
	QString m_report_path;            //������Ϣ�ļ�����·��
	QFile* m_sourceFile;
	QNetworkReply* m_replyOrg;

	//DB�����Ϣ
	QString m_dsn;
	QString m_user;
	QString m_psw;

	QSqlDatabase m_db;
	//end DB�����Ϣ

	CLog* m_plog;
	Config m_conf;

	QString m_curr_path;              //��ǰ���·��
	int m_hospital_num;                       //ҽԺ���
};