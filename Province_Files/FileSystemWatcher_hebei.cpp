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

//河北徐水宝石花医院
void FileSystemWatcher::HEBEI11000_FILE(QString path, QString filename)
{
	QString ssuffix = filename.section('.', 1, 1).trimmed();
	if ("txt" != ssuffix)
	{
		m_plog->Trace("new filename=%s,suffix is not txt,ignore", filename.toLocal8Bit().data());
		return;
	}

	//获取日期
	QString sdate = get_date_byfilename(filename);

	QString sfilepath = path + "/" + filename;

	//检查者姓名
    //QString spatient_name = filename.section(' ', 0, 1).trimmed();
	QString spatient_name = getPatientName(sfilepath);

	//获取医生姓名
	QString sdoctor = getDoctor(sfilepath);
	if ("" == sdoctor)
	{
		sdoctor = QString::fromLocal8Bit("未知");
		m_plog->Trace("get doctor from sfile_name=%s fail", filename.toUtf8().data());
	}

	//获取患者性别
	QString spatient_sex = getSex(sfilepath);
	if ("" == spatient_sex)
		spatient_sex = QString::fromLocal8Bit("女");
	else
		spatient_sex = spatient_sex.left(1);

	//获取检查结果
	QString sdescription = getResult(sfilepath);

	//从检查结果文件中获取体检号
	QString CISID = getPatientID(sfilepath);
	//图片报告路径
	//QString picture_path = m_picture_path + "/" + CISID + "_" + sdate.section(' ',0,0).trimmed()+".jpg";
	QString picture_name = CISID + "_" + sdate.section(' ', 0, 0).trimmed() + ".jpg";

	if (!conn_db())              //打开数据库连接
		return;

	//从数据库中获取申请号
	/*QString sappno = "";
	QSqlQuery query_appno(m_db);              
	QString sql_appno = "select a.Exam_No from V_Pacs_InterFace a where a.CISID=" + cur_peid;
	query_appno.prepare(sql_appno);
	m_plog->Trace("begin select Exam_No by CISID=%s", cur_peid.toUtf8().data());

	if (query_appno.exec())
	{
		if (query_appno.next())
			sappno = query_appno.value("Exam_No").toString().trimmed();

	}//end if (query_appno.exec())
	else
		m_plog->Trace("select Exam_No fail by CISID=%s", cur_peid.toUtf8().data());
    */    //不需要查询申请号，用CISID代替

	//从数据库的视图中获取患者出生日期
	QString sbirthday = "";
	QSqlQuery query_birthday(m_db);
	QString sql_birthday = "select a.BirthDay from V_Pacs_InterFace a where a.CISID=" + CISID;
	query_birthday.prepare(sql_birthday);
	m_plog->Trace("begin select BirthDay by CISID=%s", CISID.toUtf8().data());

	if (query_birthday.exec())
	{
		if (query_birthday.next())
			sbirthday = query_birthday.value("BirthDay").toString().trimmed();

	}//end if (query_appno.exec())
	else
		m_plog->Trace("select BirthDay fail by CISID=%s", CISID.toUtf8().data());

	QString spatient_id = "T" + CISID;

	char chsql[1024];
	memset(chsql, 0, 1024);    
	sprintf_s(chsql, "insert into T_SYN_ZK_CHECK(PatientID,PACS_CheckID,CISID,Exam_No,PatientNameChinese,PatientSex,PatientBirthday,StudyType,ClinicDiagnose,IMGStrings,StudyState,Check_Doc,Check_Date,Report_Doc,Report_Date,Audit_Doc,Audit_Date,Status_To_Cis) \
			        values(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',convert(datetime,\'%s\'),\'%s\',\'%s\',\'%s\',%d,\'%s\',convert(datetime,\'%s\'),\'%s\',convert(datetime,\'%s\'),\'%s\',convert(datetime,\'%s\'),%d)", \
		            spatient_id.toUtf8().data(),CISID.toUtf8().data(),CISID.toUtf8().data(),CISID.toUtf8().data(),spatient_name.toLocal8Bit().data(),spatient_sex.toLocal8Bit().data(),\
		            sbirthday.toLocal8Bit().data(),"EH",sdescription.toLocal8Bit().data(), picture_name.toLocal8Bit().data(),5,sdoctor.toLocal8Bit().data(), sdate.toLatin1().data(), \
		            sdoctor.toLocal8Bit().data(),sdate.toLatin1().data(),sdoctor.toLocal8Bit().data(),sdate.toLatin1().data(),0);

	QString sql = QString::fromLocal8Bit(chsql);
	QSqlQuery query(m_db);
	query.prepare(sql);

	m_plog->Trace("begin insert sql,content=%s", sql.toUtf8().data());
	bool bret = query.exec();
	if (false == bret)    //入库失败
		m_plog->Trace("insert into T_SYN_ZK_CHECK fail,content = %s", sql.toUtf8().data());
	else
		m_plog->Trace("insert into T_SYN_ZK_CHECK success,content = %s", sql.toUtf8().data());

	//每次都重新连接，用完关闭，是因为用户可能随时在配置窗口修改DSN
	close_db();     //关闭数据库连接  
}