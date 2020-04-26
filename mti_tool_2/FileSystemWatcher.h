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
function:     监控存放检查结果文件路径 内容变更，将部分内容入库
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

	void HUBEI10000_FILE(QString path,QString filename);    //武汉大学人民医院
	void HEBEI11000_FILE(QString path, QString filename);   //河北徐水宝石花医院
	void SHANXI12000_FILE(QString path, QString filename);  //山西太原煤炭医院

	void setPicture_path(QString spath) { m_picture_path = spath; }

public slots:
	void directoryUpdated(const QString& path);

private:
	QString getDoctor(QString spath);                     //从文件中读取医生名字
	QString getResult(QString spath);                     //从文件中读取检查结果
	QString getSex(QString spath);                        //从文件中读取患者性别
	QString getPatientID(QString spath);                  //从病例报告中取出患者的检查号
	QString getPatientName(QString spath);                //从病例报告中取出患者的姓名

	bool get_patient_info(QString spath,struct patient_info& patientinfo);          //从体检报告里获取患者信息(可以代替上面的五个函数)

	bool conn_db();                                       //连接数据库
	void close_db();                                      //关闭数据库连接
	QString get_date_byfilename(QString filename);        //从文件名中取得日期

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
	QSqlDatabase m_db;                        //数据库连接实例
	int m_hospital_num;                       //医院编号
	QString m_curr_path;                      //当前监控路径   
	QString m_db_code;                        //数据库编码

	QString m_picture_path;
};  
//end class

