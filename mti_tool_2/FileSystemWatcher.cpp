#include "FileSystemWatcher.h"

#include <QDir>
#include <QSet>
#include <QFileInfo>
#include <QTextCodec>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QDatetime>

FileSystemWatcher::FileSystemWatcher(int islog, QString basepath, QObject *parent) :QObject(parent)
{
	m_plog = new CLog(islog,basepath,"filesys_watch.log");
	assert(m_plog);

	m_curr_path = "";
	m_picture_path = "";
	m_hospital_num = m_conf.Get("hospital","serial_num").toInt();
	m_db_code = m_conf.Get("db","code").toString();
}

FileSystemWatcher::~FileSystemWatcher()
{
	if (m_plog)
	{
		delete m_plog;
		m_plog = nullptr;
	}

	close_db();
}

void FileSystemWatcher::directoryUpdated(const QString& path)
{
	//比较最新的内容和保存的内容找出区别(变化)
	QStringList currEntryList = m_currentContentMap[path];
	
	const QDir dir(path);
	QStringList newEntryList = dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);

	QSet<QString> newDirSet = QSet<QString>::fromList(newEntryList);
	QSet<QString> currentDirSet = QSet<QString>::fromList(currEntryList);

	//添加了文件
	QSet<QString> newFiles = newDirSet - currentDirSet;
	QStringList newFile = newFiles.toList();

	//更新当前设置
	m_currentContentMap[path] = newEntryList;

	//文件减少，不关心
	if (newFile.size() <= 0)
		return;

	//遍历新增加的文件
	for (QStringList::iterator iter = newFile.begin(); iter != newFile.end();++iter)
	{
		QString sfile_name = *iter;
		switch (m_hospital_num)
		{
		case 10000:                  
			HUBEI10000_FILE(path,sfile_name);      //武汉大学人民医院
			break;
		case 11000:
			HEBEI11000_FILE(path, sfile_name);     //河北徐水宝石花医院
			break;
		case 12000:
			SHANXI12000_FILE(path,sfile_name);     //山西太原煤炭医院
			break;
		default:
			m_plog->Trace("hospital num = %d is error",m_hospital_num);
			break;
		}
	}//end for(QStringList::iterator iter = newFile.begin(); iter != newFile.end();++iter)
	
}//end directoryUpdated



//begin public functions
void FileSystemWatcher::addWatchPath(QString path)
{
	m_pSystemWatcher = new QFileSystemWatcher();
	assert(m_pSystemWatcher);
	connect(m_pSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryUpdated(QString)));

	m_pSystemWatcher->addPath(path);
	m_curr_path = path;

	//如果添加路径是一个目录，保存当前内容列表
	QFileInfo file(path);
	if (file.isDir())
	{
		const QDir dirw(path);
		m_currentContentMap[path] = dirw.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
	}

	m_plog->Trace("add watch path %s", path.toUtf8().data());
}

void FileSystemWatcher::modifyWatchPath(QString path)
{
	if (m_pSystemWatcher)
	{
		m_pSystemWatcher->removePath(m_curr_path);
		m_pSystemWatcher->addPath(path);
		m_curr_path = path;

		//如果添加路径是一个目录，保存当前内容列表
		QFileInfo file(path);
		if (file.isDir())
		{
			const QDir dirw(path);
			m_currentContentMap[path] = dirw.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
		}

		m_plog->Trace("modify watch path success,new path=%s", m_curr_path.toLocal8Bit().data());
	}
	else
		m_plog->Trace("modify watch path fail,path=%s,curr_path=%s", path.toLocal8Bit().data(), m_curr_path.toLocal8Bit().data());
}

void FileSystemWatcher::set_write_log(bool flag)
{
	m_plog->set_logflag(flag);
}

void FileSystemWatcher::HUBEI10000_FILE(QString path,QString filename)
{
	QString ssuffix = filename.section('.', 1, 1).trimmed();
	if ("txt" != ssuffix)
	{
		m_plog->Trace("new filename=%s,suffix is not txt,ignore",filename.toLocal8Bit().data());
		return;
	}

	//获取日期
	QString sdate = get_date_byfilename(filename);
	QByteArray ba = sdate.toLatin1();
	char* cdate = ba.data();

	QString sfilepath = path + "/" + filename;

	//struct patient_info pinfo;
	//get_patient_info(sfilepath, pinfo);

	//获取医生姓名
	const char* chdoctor = nullptr;
	std::string str_doctor = "";
	QByteArray ba_doctor;
	QString sdoctor = getDoctor(sfilepath);
	if ("" == sdoctor)
	{
		sdoctor = "未知";
		str_doctor = sdoctor.toStdString();
		chdoctor = str_doctor.c_str();
		m_plog->Trace("get doctor from sfile_name=%s fail", filename.toUtf8().data());
	}
	else
	{
		ba_doctor = sdoctor.toLocal8Bit();
		chdoctor = ba_doctor.data();
	}

	//获取检查结果
	QString sdescription = getResult(sfilepath);
	QByteArray ba_des = sdescription.toLocal8Bit();
	char* ch_des = ba_des.data();

	//从检查结果文件中获取体检号
	QString cur_peid = getPatientID(sfilepath);

	if (!conn_db())              //打开数据库连接
		return;

	//从数据库中获取申请号
	QString sappno = "";               
	const char* ch_appno = nullptr;
	QByteArray ba_appno;
	QSqlQuery query_appno(m_db);              //
	QString sql_appno = "select a.apply_no from PHYEXAM.rxcz_info a where a.pe_id=" + cur_peid;
	query_appno.prepare(sql_appno);
	m_plog->Trace("begin select applyno by peid=%s", cur_peid.toUtf8().data());
	if (query_appno.exec())
	{
		if (query_appno.next())
		{
			QVariant qv = query_appno.value("apply_no");
			sappno = qv.toString().trimmed();
			ba_appno = sappno.toLocal8Bit();
			ch_appno = ba_appno.data();
		}
	}//end if (query_appno.exec())
	else
		m_plog->Trace("select apply_no fail by peid=%s", cur_peid.toUtf8().data());

	char chsql[512];
	memset(chsql, 0, 512);    //
	sprintf_s(chsql, "insert into PHYEXAM.RxczTestValue(apply_no,exam_date_time,report_date_time,technician,reporter,description) \
			        values(\'%s\',to_date(\'%s\',\'YYYY/MM/DD HH24:MI:SS\'),to_date(\'%s\',\'YYYY/MM/DD HH24:MI:SS\'),\'%s\',\'%s\',\'%s\')", \
		            ch_appno, cdate, cdate, chdoctor, chdoctor, ch_des);

	QString sql = QString::fromLocal8Bit(chsql);
	QSqlQuery query(m_db);
	query.prepare(sql);

	m_plog->Trace("begin insert sql,content=%s", sql.toUtf8().data());
	bool bret = query.exec();
	if (false == bret)    //入库失败
		m_plog->Trace("insert into RxczTestValue fail,content = %s", sql.toUtf8().data());
	else
		m_plog->Trace("insert into RxczTestValue success,content = %s", sql.toUtf8().data());

	//每次都重新连接，用完关闭，是因为用户可能随时在配置窗口修改DSN
	close_db();     //关闭数据库连接     
}
//end public functions



//begin private functions
//获取检查医生姓名
QString FileSystemWatcher::getDoctor(QString spath)
{
	QString sdoctor = "";
	QTextCodec* codec = QTextCodec::codecForName("GBK");

	QFile qfile(spath);
	bool bopen = false;
	bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	if (!bopen)
	{
		m_plog->Trace("open file fail in getDoctor,filepath=%s", spath.toUtf8().data());
		return sdoctor;
	}

	QByteArray t;
	t = qfile.readLine();
	QString scontent = codec->toUnicode(t);
	
	if ("##" == scontent.left(2))
	{
		t = qfile.readLine();
		scontent = codec->toUnicode(t);
		QStringList qlist = scontent.split('	');
		sdoctor = qlist.value(6);
	}

	qfile.close();

	return sdoctor;
}//end getDoctor

QString FileSystemWatcher::getSex(QString spath)
{
	QString sex = "";
	QTextCodec* codec = QTextCodec::codecForName("GBK");

	QFile qfile(spath);
	bool bopen = false;
	bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	if (!bopen)
	{
		m_plog->Trace("open file fail in getSex,filepath=%s", spath.toUtf8().data());
		return sex;
	}

	QByteArray t;
	t = qfile.readLine();
	QString scontent = codec->toUnicode(t);

	if ("##" == scontent.left(2))
	{
		t = qfile.readLine();
		scontent = codec->toUnicode(t);
		QStringList qlist = scontent.split('	');
		sex = qlist.value(3);
	}

	qfile.close();

	return sex;
}

QString FileSystemWatcher::getResult(QString spath)
{
	QString sdes = "";
	QTextCodec* codec = QTextCodec::codecForName("GBK");

	QFile qfile(spath);
	bool bopen = false;
	bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	if (!bopen)
	{
		m_plog->Trace("open file fail in getResult,filepath=%s", spath.toUtf8().data());
		return sdes;
	}

	QByteArray t;
	int itimes = 0;
	while (!qfile.atEnd())
	{
		t = qfile.readLine();
		QString scontent = codec->toUnicode(t);
		scontent = scontent.trimmed();

		if ("##" == scontent.left(2) && itimes < 2)
		{
			++itimes;
			continue;
		}
		else if (2 == itimes)
		{
			sdes += scontent;
			sdes += ";";
		}
	}
	sdes = sdes.left(sdes.length() - 1);
	qfile.close();

	return sdes;
}//end getResult

QString FileSystemWatcher::getPatientID(QString spath)
{
	QString spaintent_id = "";
	QTextCodec* codec = QTextCodec::codecForName("GBK");

	QFile qfile(spath);
	bool bopen = false;
	bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	if (!bopen)
	{
		m_plog->Trace("open file fail in getPatientID,filepath=%s", spath.toUtf8().data());
		return spaintent_id;
	}

	QByteArray t;
	t = qfile.readLine();
	QString scontent = codec->toUnicode(t);

	if ("##" == scontent.left(2))
	{
		t = qfile.readLine();
		scontent = codec->toUnicode(t);
		QStringList qlist = scontent.split('	');
		spaintent_id = qlist.value(2);
	}

	qfile.close();

	return spaintent_id;
}//end func getPaintentID

QString FileSystemWatcher::getPatientName(QString spath)
{
	QString spaintent_name = "";
	QTextCodec* codec = QTextCodec::codecForName("GBK");

	QFile qfile(spath);
	bool bopen = false;
	bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	if (!bopen)
	{
		m_plog->Trace("open file fail in getPatientNane,filepath=%s", spath.toUtf8().data());
		return spaintent_name;
	}

	QByteArray t;
	t = qfile.readLine();
	QString scontent = codec->toUnicode(t);

	if ("##" == scontent.left(2))
	{
		t = qfile.readLine();
		scontent = codec->toUnicode(t);
		QStringList qlist = scontent.split('	');
		spaintent_name = qlist.value(1);
	}

	qfile.close();

	return spaintent_name;
}

bool FileSystemWatcher::get_patient_info(QString spath, struct patient_info& patientinfo)
{
	 QTextCodec* codec = QTextCodec::codecForName("GBK");

	 QFile qfile(spath);
	 bool bopen = false;
	 bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	 if (!bopen)
	 {
		 m_plog->Trace("open file fail in get_patient_info,filepath=%s", spath.toUtf8().data());
		 return false;
	 }

	 QByteArray t;
	 int itimes = 0;
	 QString sdes = "";
	 while (!qfile.atEnd())
	 {
		 t = qfile.readLine();
		 QString scontent = codec->toUnicode(t);
		 scontent = scontent.trimmed();

		 if ("##" == scontent.left(2) && 0 == itimes)
		 {
		     t = qfile.readLine();
			 scontent = codec->toUnicode(t);
			 QStringList qlist = scontent.split('	');
			 patientinfo.spatient_name = qlist.value(1);
			 patientinfo.spatient_id = qlist.value(2);
			 patientinfo.sSex = qlist.value(3);
			 patientinfo.sdoctor_name = qlist.value(6);

			 ++itimes;
		 }
		 else if ("##" == scontent.left(2) && itimes < 2)
		 {
			 ++itimes;
			 continue;
		 }
		 else if (2 == itimes)
		 {
			 sdes += scontent;
			 sdes += ";";
		 }
	 } 

	 patientinfo.sresult = sdes;
	 qfile.close();

	 return true;
}

bool FileSystemWatcher::conn_db()
{
	if (!m_db.isOpen())
	{
		m_dsn = m_conf.Get("dsninfo", "dsn").toString();
		m_user = m_conf.Get("dsninfo", "db_user").toString();
		m_psw = m_conf.Get("dsninfo", "db_psw").toString();

		m_db = QSqlDatabase::addDatabase("QODBC");
		//db.setPort(m_port.toInt());
		//db.setHostName(m_ip);

		//***数据源配置时填写的那个DataSourceName
		m_db.setDatabaseName(m_dsn);
		m_db.setUserName(m_user);
		m_db.setPassword(m_psw);

		if (!m_db.open())
		{
			m_plog->Trace("open DB fail,dsn=%s,detail=%s", m_dsn.toUtf8().data(), m_db.lastError().text().toUtf8().data());
			return false;
		}

		return true;
	}
	return true;
}

void FileSystemWatcher::close_db()
{
	if (m_db.isOpen())
		m_db.close();
}

QString FileSystemWatcher::get_date_byfilename(QString filename)
{
	QString sdate = "";         //报告日期

	QStringList qlist_date = filename.split(' ');
	sdate = qlist_date.value(qlist_date.count() - 1);
	sdate = sdate.section('.', 0, 0).trimmed();
	if ("" == sdate)   //取不到日期，就用当前日期
	{
		QDateTime current_date_time = QDateTime::currentDateTime();
		sdate = current_date_time.toString("yyyy/MM/dd hh:mm:ss");
		m_plog->Trace("get date from filename=%s fail", filename.toLocal8Bit().data());
	}
	else
	{
		QDateTime current_time = QDateTime::currentDateTime();
		QStringList qlist_data2 = sdate.split('-');
		QString syear = qlist_data2.value(0);
		QString smonth = qlist_data2.value(1);
		QString sday = qlist_data2.value(2);

		//if ("0" == smonth.left(1))
		//	smonth = smonth.right(1);
		//if ("0" == sday.left(1))
		//	sday = sday.right(1);

		char ch_date[64];
		memset(ch_date, 0, 64);
		sprintf_s(ch_date, "%s-%02s-%02s", syear.toLocal8Bit().data(), smonth.toLocal8Bit().data(), sday.toLocal8Bit().data());
		sdate = ch_date;
		sdate += current_time.toString(" hh:mm:ss");
	}
	
	return sdate;
}
//end private functions

