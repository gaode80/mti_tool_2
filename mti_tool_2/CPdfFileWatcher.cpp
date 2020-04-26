#include "CPdfFileWatcher.h"

#include <QTextCodec>
#include <QMessageBox>

CPdfFileWatcher::CPdfFileWatcher(int islog, QString basepath, QObject* parent) :QObject(parent)
{
	m_sftpIp="";
	m_ftpuser="";
	m_ftppsw="";

	m_ftpport=21;

	m_plog = new CLog(islog,basepath,"pdffile_watcher.log");
	assert(m_plog);

	m_curr_path = "";
	m_hospital_num = m_conf.Get("hospital", "serial_num").toInt();
}

CPdfFileWatcher::~CPdfFileWatcher()
{
	if (m_plog)
	{
		delete m_plog;
		m_plog = nullptr;
	}

}

//begin slots functions
void CPdfFileWatcher::directoryUpdated(const QString& path)
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

	//bool bconnect  = connect_db();
	//遍历新增加的文件
	for (QStringList::iterator iter = newFile.begin(); iter != newFile.end(); ++iter)
	{	
		QString sfile_name = *iter;
		switch (m_hospital_num)
		{
		case 10000:                  
			HUBEI10000_PDF(path,sfile_name);       //武汉大学人民医院
			break;
		case 11000:
			HEBEI11000_PDF(path, sfile_name);      //河北徐水宝石花医院
			break;
		case 12000:
			SHANXI12000_PDF(path, sfile_name);     //山西太原煤炭医院
			break;
		default:
			m_plog->Trace("hospital num = %d is error", m_hospital_num);
			break;
		}
	}//end for(...
}//end directoryUpdated

void CPdfFileWatcher::OnFinishRelay()
{
	if (m_replyOrg->error() == QNetworkReply::NoError)
	{
		//QMessageBox::critical(0, "alarm", "pat file FTP upload fail!", QMessageBox::Ok, 0);
		m_plog->Trace("upload file FTP upload fail!");
	}
	else
		m_plog->Trace("upload file FTP upload success!");

	m_sourceFile->close();
	m_replyOrg->abort();
	m_replyOrg->deleteLater();
}
//end slots functions




//begin public functions
void CPdfFileWatcher::addWatchPath(QString spath)
{
	m_pPDFWatcher = new QFileSystemWatcher();
	connect(m_pPDFWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryUpdated(QString)));
	m_pPDFWatcher->addPath(spath);
	m_curr_path = spath;

	//如果添加路径是一个目录，保存当前内容列表
	QFileInfo file(spath);
	if (file.isDir())
	{
		const QDir dirw(spath);
		m_currentContentMap[spath] = dirw.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
	}

	m_plog->Trace("add watch path %s", spath.toLatin1().data());
}//end addWatchPath

void CPdfFileWatcher::modifyWatchPath(QString path)
{
	if (m_pPDFWatcher)
	{
		m_pPDFWatcher->removePath(m_curr_path);
		m_pPDFWatcher->addPath(path);
		m_curr_path = path;

		//如果添加路径是一个目录，保存当前内容列表
		QFileInfo file(path);
		if (file.isDir())
		{
			const QDir dirw(path);
			m_currentContentMap[path] = dirw.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
		}

		m_plog->Trace("modify watch pdf path success,new path=%s", m_curr_path.toLocal8Bit().data());
	}
	else
		m_plog->Trace("modify watch pdf path fail,path=%s,curr_path=%s", path.toLocal8Bit().data(), m_curr_path.toLocal8Bit().data());
}

void CPdfFileWatcher::set_write_log(bool flag)
{
	m_plog->set_logflag(flag);
}
//end public functions




//begin private function
void CPdfFileWatcher::HUBEI10000_PDF(QString path, QString filename)
{
	QString sfilepath = "";
	sfilepath = path + "/" + filename;

	//从病例报告文件中取 检查号
	QStringList qlist = filename.split('.');
	QString report_name = qlist.value(0);    //病例文件名
	QString file_suffix = qlist.value(1);    //后缀

	//取文件名中的日期
	QString sdate = get_date_byfilename(report_name);

	//从检查结果中获取 体检号
	QString report_path = m_report_path + "/" + report_name + ".txt";
	QString patientID = getPatientID(report_path);
	if ("" == patientID)
	{
		m_plog->Trace("get patientID from %s fail,patientID=%s", report_path.toLocal8Bit().data(),patientID.toLatin1().data());
		return;
	}
	
	QString new_filepath = "";
	QString qapplyno = "";
	if (connect_db())
	{
		//根据体检号获取 申请号
		                                    //
		QString sql = "select a.apply_no from PHYEXAM.rxcz_info a where a.pe_id=" + patientID;
		QSqlQuery query(m_db);
		query.prepare(sql);
		if (query.exec())
		{
			if (query.next())
				qapplyno = query.value("apply_no").toString().trimmed();
		}

		m_plog->Trace("appno=%s,sql=%s", qapplyno.toLatin1().data(), sql.toLatin1().data());
		if ("" == qapplyno)
			new_filepath = path + "/" + patientID + "_" + sdate + "." + file_suffix;
		else
			new_filepath = path + "/" + qapplyno + "." + file_suffix;
	}
	else
	{
		m_plog->Trace("connect db fail");
		new_filepath = path + "/" + patientID + "_" + sdate + "." + file_suffix;
	}
		//new_filepath = path + "/" + patientID + "_" + sdate + "." + file_suffix;

	/*QTextCodec* codec = */QTextCodec::codecForName("GBK");
	QByteArray ba_old;
	ba_old = sfilepath.toLocal8Bit();
	char* old_name = ba_old.data();

	QByteArray ba_new;
	ba_new = new_filepath.toLocal8Bit();
	char* new_name = ba_new.data();

	int rret = rename(old_name, new_name);
	m_plog->Trace("rename result=%d", rret);

	m_sourceFile = new QFile(new_filepath);             //设置文件路径   
	m_sourceFile->open(QIODevice::ReadOnly);            //读取模式        
	//by_txt=sourceFile->readAll();   //全部读取,遇到大文件时会内存溢出        
	//sourceFile->close();            //关闭文件         

	m_sftpIp = m_conf.Get("ftp", "ip").toString();
	m_ftpport = m_conf.Get("ftp", "port").toInt();
	m_ftpuser = m_conf.Get("ftp", "user").toString();
	m_ftppsw = m_conf.Get("ftp", "psw").toString();

	QNetworkAccessManager* manager = new QNetworkAccessManager();
	QUrl* ftpUrl = new QUrl();             //设置QUrl数据         
	ftpUrl->setScheme("ftp");              //设置URL的类型(或协议)        
	ftpUrl->setHost(m_sftpIp);             //设置主机地址        
	ftpUrl->setPort(m_ftpport);            //端口        
	ftpUrl->setUserName(m_ftpuser);        //ftp用户名        
	ftpUrl->setPassword(m_ftppsw);         //ftp密码 
	if ("" == qapplyno)
		ftpUrl->setPath(patientID + "_" + sdate + "." + file_suffix);    //设置路径+文件名,这里为ftp根目录 
	else
		ftpUrl->setPath(qapplyno + "." + file_suffix);

	m_plog->Trace("begin manager->put");
	m_replyOrg = manager->put(QNetworkRequest(*ftpUrl), m_sourceFile);   //上传
	m_plog->Trace("end manager->put");

	connect(m_replyOrg, SIGNAL(finished()), this, SLOT(OnFinishRelay()));

	close_db();           //关闭数据库连接
}

//获取患者ID
QString CPdfFileWatcher::getPatientID(QString spath)
{
	QString spaintent_id = "";
	QTextCodec* codec = QTextCodec::codecForName("GBK");

	QFile qfile(spath);
	bool bopen = false;
	bopen = qfile.open(QIODevice::ReadOnly); //| QIODevice::Text
	if (!bopen)
	{
		m_plog->Trace("open file fail filepath=%s", spath.toLocal8Bit().data());
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

bool CPdfFileWatcher::connect_db()
{
	if (!m_db.isOpen())
	{
		m_dsn = m_conf.Get("dsninfo", "dsn").toString();
		m_user = m_conf.Get("dsninfo", "db_user").toString();
		m_psw = m_conf.Get("dsninfo", "db_psw").toString();

		m_db = QSqlDatabase::addDatabase("QODBC");   //, "whhis"

		//数据源配置时填写的那个DataSourceName
		m_db.setDatabaseName(m_dsn);
		m_db.setUserName(m_user);
		m_db.setPassword(m_psw);

		if (!m_db.open())
		{
			QMessageBox::critical(0, QObject::tr("Database Error"), m_db.lastError().text());
			m_plog->Trace("connect DB fail,user=%s,psw=%s,error_content=%s", \
				           m_user.toUtf8().data(), m_psw.toLatin1().data(), m_db.lastError().text().toUtf8().data());

			return false;
		}
		return true;
	}
	return true;
}

void CPdfFileWatcher::close_db()
{
	if (m_db.isOpen())
		m_db.close();
}

QString CPdfFileWatcher::get_date_byfilename(QString filename)
{
	QStringList qlist_date = filename.split(' ');
	QString sdate = qlist_date.value(qlist_date.count() - 1).trimmed();
	if ("" == sdate)   //取不到日期，就用当前日期
	{
		QDateTime current_date_time = QDateTime::currentDateTime();
		sdate = current_date_time.toString("yyyy-MM-dd");
		m_plog->Trace("get date from report_name=%s fail", filename.toUtf8().data());
	}

	return sdate;
}
//end private functions