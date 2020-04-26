#include "CPdfFileWatcher.h"
#include <windows.h>
//#include <stdio.h>
#include <io.h>

#include <QTextCodec>
#include <QMessageBox>

void CPdfFileWatcher::HEBEI11000_PDF(QString path, QString filename)
{
	QString sfilepath = "";
	sfilepath = path + "/" + filename;

	//从病例报告文件中取 检查号
	QStringList qlist = filename.split('.');
	QString report_name = qlist.value(0);    //病例文件名
	QString file_suffix = qlist.value(1);    //后缀

	//取文件名中的日期
	//QString sdate = get_date_byfilename(report_name);

	//从检查结果中获取 体检号
	//QString report_path = m_report_path + "/" + report_name + ".txt";
	//QString patientID = getPatientID(report_path);
	//if ("" == patientID)
	//{
	//	m_plog->Trace("get patientID from %s fail,patientID=%s", report_path.toLocal8Bit().data(), patientID.toLatin1().data());
	//	return;
	//}

	//QString new_filepath = path + "/" + patientID + "_" + sdate + "." + file_suffix;
	if (filename.contains("_", Qt::CaseSensitive))
		return;

	QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy-MM-dd");
	//QStringList date_list = current_date.split('-');
	//QString syear = date_list.value(0);
	//QString smonth = date_list.value(1);
	//QString sday = date_list.value(2);
	//if ("0" == smonth.left(1))
	//	smonth = smonth.right(1);
	//if ("0" == sday.left(1))
	//	sday = sday.right(1);
	//current_date = syear + "-" + smonth + "-" + sday;

	int rret = 0;
	QString new_filepath = path + "/" + report_name + "_" + current_date + "." + file_suffix;
	if (0 == _access(new_filepath.toLocal8Bit().data(), 0))
	{
		rret = remove(new_filepath.toLocal8Bit().data());
		if (0 == rret)
			m_plog->Trace("remove file %s success", new_filepath.toLocal8Bit().data());
		else
		{
			m_plog->Trace("remove file %s fail", new_filepath.toLocal8Bit().data());
			return;
		}
	}
	
	rret = rename(sfilepath.toLocal8Bit().data(), new_filepath.toLocal8Bit().data());
	m_plog->Trace("rename result=%d,new_name=%s,old_name=%s", rret, new_filepath.toLocal8Bit().data(), sfilepath.toLocal8Bit().data());
	
	if (rret != 0)
	{
		Sleep(3000);
		rret = rename(sfilepath.toLocal8Bit().data(), new_filepath.toLocal8Bit().data());
		m_plog->Trace("two rename result=%d,new_name=%s,old_name=%s", rret, new_filepath.toLocal8Bit().data(), sfilepath.toLocal8Bit().data());
	}
}