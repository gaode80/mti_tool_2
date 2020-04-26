#include "FileSystemWatcher.h"
#include <windows.h>

#include <QDir>
#include <QSet>
#include <QFileInfo>
#include <QTextCodec>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QDatetime>

//�ӱ���ˮ��ʯ��ҽԺ
void FileSystemWatcher::HEBEI11000_FILE(QString path, QString filename)
{
	int ipos = filename.indexOf("_", Qt::CaseInsensitive);   //����Сʱ,�֣��� ���ļ����������������ģ��Թ�
	if (ipos >= 0)
		return;

	QString sfilepath = path + "/" + filename;

	QString ssuffix = filename.section('.', 1, 1).trimmed();
	if ("pat" == ssuffix)
	{
		Sleep(2000);
		QDateTime current_time = QDateTime::currentDateTime();
		QStringList list_name = filename.split('.');
		QString new_name = list_name.value(0) + "_" + current_time.toString("hh-mm") + "."+list_name.value(1);
		QString old_path = path + "/" + filename;
		QString new_path = path + "/" + new_name;
		int iret = rename(old_path.toLocal8Bit().data(), new_path.toLocal8Bit().data());
		if (!iret)
			m_plog->Trace("rename success,oldname = %s,newname=%s", filename.toLocal8Bit().data(), new_name.toLocal8Bit().data());
		else
			m_plog->Trace("rename fail,oldname = %s,newname=%s", filename.toLocal8Bit().data(), new_name.toLocal8Bit().data());

		return;
	}

	if ("txt" != ssuffix)
	{
		m_plog->Trace("new filename=%s,suffix is not txt,ignore", filename.toLocal8Bit().data());
		return;
	}

	//��ȡ����
	QString sdate = get_date_byfilename(filename);

	//���������
    //QString spatient_name = filename.section(' ', 0, 1).trimmed();
	QString spatient_name = getPatientName(sfilepath);

	//��ȡҽ������
	QString sdoctor = getDoctor(sfilepath);
	if ("" == sdoctor)
	{
		sdoctor = QString::fromLocal8Bit("δ֪");
		m_plog->Trace("get doctor from sfile_name=%s fail", filename.toUtf8().data());
	}

	//��ȡ�����Ա�
	QString spatient_sex = getSex(sfilepath);
	if ("" == spatient_sex)
		spatient_sex = QString::fromLocal8Bit("Ů");
	else
		spatient_sex = spatient_sex.left(1);

	//��ȡ�����
	QString sdescription = getResult_11000(sfilepath);

	//�Ӽ�����ļ��л�ȡ����
	QString CISID = getPatientID(sfilepath);
	//ͼƬ����·��
	//QString picture_path = m_picture_path + "/" + CISID + "_" + sdate.section(' ',0,0).trimmed()+".jpg";
	QString picture_name = CISID + "_" + sdate.section(' ', 0, 0).trimmed() + ".jpg";

	if (!conn_db())              //�����ݿ�����
		return;

	//�����ݿ��л�ȡ�����
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
    */    //����Ҫ��ѯ����ţ���CISID����

	//�����ݿ����ͼ�л�ȡ���߳�������
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

	m_plog->Trace("begin insert sql,content=%s", sql.toLocal8Bit().data());
	bool bret = query.exec();
	if (false == bret)    //���ʧ��
		m_plog->Trace("insert into T_SYN_ZK_CHECK fail,content = %s", sql.toLocal8Bit().data());
	else
		m_plog->Trace("insert into T_SYN_ZK_CHECK success,content = %s", sql.toLocal8Bit().data());

	QDateTime current_time = QDateTime::currentDateTime();
	QStringList list_name = filename.split('.');
	QString new_name = list_name.value(0)+"_"+current_time.toString("hh-mm")+"."+list_name.value(1);
	QString old_path = path + "/" + filename;
	QString new_path = path + "/" + new_name;
	int iret = rename(old_path.toLocal8Bit().data(), new_path.toLocal8Bit().data());
	if (!iret)
		m_plog->Trace("rename success,oldname = %s,newname=%s", filename.toLocal8Bit().data(), new_name.toLocal8Bit().data());
	else
		m_plog->Trace("rename fail,oldname = %s,newname=%s", filename.toLocal8Bit().data(), new_name.toLocal8Bit().data());

	//ÿ�ζ��������ӣ�����رգ�����Ϊ�û�������ʱ�����ô����޸�DSN
	close_db();     //�ر����ݿ�����  
}

QString FileSystemWatcher::getResult_11000(QString spath)
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
			QStringList qlist = scontent.split('	');
			QString sdes_temp = qlist.value(8);
			if (sdes_temp != "")
			{
				sdes += qlist.value(8);	
				sdes += ";";
			}
		}
	}
	sdes = sdes.left(sdes.length() - 1);
	qfile.close();

	return sdes;
}