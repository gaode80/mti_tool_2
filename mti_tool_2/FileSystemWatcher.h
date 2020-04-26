#pragma once
#include "CLog.h"
#include "Config.h"

#include <QObject>
#include <QFileSystemWatcher>
#include <QMap>
#include <QSqlDatabase>

#include <string>
using namespace std;

//
/*******************************************************
author:       gaojy
create date:  2019-10-27
function:     ��ش�ż�����ļ�·�� ���ݱ�����������������
********************************************************/

struct patient_info
{
	QString sdoctor_name;
	QString sresult;
	QString sSex;
	QString spatient_id;
	QString spatient_name;
};

class FileSystemWatcher :public QObject
{
	Q_OBJECT

//private:
public:
	explicit FileSystemWatcher(int islog,QString basepath,QObject *parent=0);
	~FileSystemWatcher();

public:
	void addWatchPath(QString path);
	void modifyWatchPath(QString path);
	void set_write_log(bool flag);

	void HUBEI10000_FILE(QString path,QString filename);    //�人��ѧ����ҽԺ
	void HEBEI11000_FILE(QString path, QString filename);   //�ӱ���ˮ��ʯ��ҽԺ
	void SHANXI12000_FILE(QString path, QString filename);  //ɽ��̫ԭú̿ҽԺ

	void setPicture_path(QString spath) { m_picture_path = spath; }

public slots:
	void directoryUpdated(const QString& path);

private:
	QString getDoctor(QString spath);                     //���ļ��ж�ȡҽ������
	QString getResult(QString spath);                     //���ļ��ж�ȡ�����
	QString getSex(QString spath);                        //���ļ��ж�ȡ�����Ա�
	QString getPatientID(QString spath);                  //�Ӳ���������ȡ�����ߵļ���
	QString getPatientName(QString spath);                //�Ӳ���������ȡ�����ߵ�����

	bool get_patient_info(QString spath,struct patient_info& patientinfo);          //����챨�����ȡ������Ϣ(���Դ���������������)

	bool conn_db();                                       //�������ݿ�
	void close_db();                                      //�ر����ݿ�����
	QString get_date_byfilename(QString filename);        //���ļ�����ȡ������

private:
	//static FileSystemWatcher* m_pInstance;
	QFileSystemWatcher* m_pSystemWatcher;
	QMap<QString,QStringList> m_currentContentMap;

	QString m_dsn;
	QString m_user;
	QString m_psw;
	//QString m_ip;
	//QString m_port;

	CLog* m_plog;
	Config m_conf;
	QSqlDatabase m_db;                        //���ݿ�����ʵ��
	int m_hospital_num;                       //ҽԺ���
	QString m_curr_path;                      //��ǰ���·��   
	QString m_db_code;                        //���ݿ����

	QString m_picture_path;
};  
//end class

