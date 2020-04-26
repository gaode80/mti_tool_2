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
#include <QTextCodec>

void FileSystemWatcher::SHANXI12000_FILE(QString path, QString filename)
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

	struct patient_info pinfo;
	bool bret = get_patient_info(sfilepath, pinfo);
	if (!bret)
	{
		m_plog->Trace("get patient info failed!");
		return;
	}

	if (!conn_db())              //打开数据库连接
		return;

	QString sql = "select sequence_no from VIEW_PEIS_RXCX where PATIENTNO='";
	sql += pinfo.spatient_id;
	sql += "';";
	QSqlQuery query_view(m_db);
	query_view.prepare(sql);
	bret = query_view.exec();
	if (!bret)
	{
		m_plog->Trace("query sequence_no fail,sql=%s",sql.toLocal8Bit().data());
		close_db();
		return;
	}

	QString sequence_no = "";
	if (query_view.next())
		sequence_no = query_view.value("sequence_no").toString();
	else
	{
		m_plog->Trace("get sequence_no fail by patientno=%s", pinfo.spatient_id.toLocal8Bit().data());
		close_db();
		return;
	}

	//sql = QString("call PROC_PEIS_GETRXCXRESULTINFO(?,?,convert(?,'ZHS16GBK','utf8'),convert(?,'ZHS16GBK','utf8'),convert(?,'ZHS16GBK','utf8'),to_date(?,'YYYY/MM/DD HH24:mi:ss'),?,?)");  
	sql = "call PROC_PEIS_GETRXCXRESULTINFO(?,?,convert(?,'";
	sql += m_db_code;
	sql += "','utf8'),convert(?,'";
	sql += m_db_code;
	sql += "','utf8'),convert(?,'";
	sql += m_db_code;
	sql += "','utf8'),to_date(?,'YYYY / MM / DD HH24 : mi:ss'),?,?)";
	QSqlQuery query_mt(m_db);  
	query_mt.prepare(sql);
	query_mt.bindValue(0, pinfo.spatient_id);
	query_mt.bindValue(1, sequence_no); 
	query_mt.bindValue(2, QString::fromLocal8Bit(pinfo.sresult.toLocal8Bit())); 
	query_mt.bindValue(3, QString::fromLocal8Bit(pinfo.sresult.toLocal8Bit()));
	query_mt.bindValue(4, QString::fromLocal8Bit(pinfo.sdoctor_name.toLocal8Bit()));
	query_mt.bindValue(5, sdate);
	query_mt.bindValue(6, 0, QSql::Out);
	query_mt.bindValue(7, "0000000000000000000000000000000000000000000000000000000000000000", QSql::Out);
	
	bret = query_mt.exec();
	if (bret)
	{
		QString errc =  query_mt.boundValue(6).toString();
		QByteArray qb = query_mt.boundValue(7).toByteArray();
		QString s_des = QString::fromLocal8Bit(qb);
		m_plog->Trace("exec PROC_PEIS_GETRXCXRESULTINFO success,return_code=%s,return_des=%s,patient_id=%s,sequence_no=%s,result=%s,doctor_name=%s,sdate=%s", \
			           errc.toLocal8Bit().data(),s_des.toLocal8Bit().data(), pinfo.spatient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data(), \
			           pinfo.sresult.toLocal8Bit().data(),pinfo.sdoctor_name.toLocal8Bit().data(), sdate.toLocal8Bit().data());
	}
	else
	{
		m_plog->Trace("exec PROC_PEIS_GETRXCXRESULTINFO fail,patient_id=%s,sequence_no=%s,result=%s,doctor_name=%s,sdate=%s",\
		               pinfo.spatient_id.toLocal8Bit().data(),sequence_no.toLocal8Bit().data(),pinfo.sresult.toLocal8Bit().data(),\
		               pinfo.sdoctor_name.toLocal8Bit().data(),sdate.toLocal8Bit().data());
	}

	//每次都重新连接，用完关闭，是因为用户可能随时在配置窗口修改DSN
	close_db();     //关闭数据库连接  
}