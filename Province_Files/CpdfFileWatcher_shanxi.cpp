#include "CPdfFileWatcher.h"
#include <windows.h>
#include <io.h>
#include <direct.h>

#include <QTextCodec>
#include <QMessageBox>

void CPdfFileWatcher::SHANXI12000_PDF(QString path, QString filename)
{
	if (filename.contains("_", Qt::CaseSensitive))
	{
		return;
	}

	QString sfilepath = "";
	sfilepath = path + "/" + filename;

	//�Ӳ��������ļ���ȡ ����
	QStringList qlist = filename.split('.');
	QString report_name = qlist.value(0);      //�ļ���
	QString file_suffix = qlist.value(1);      //��׺

	//��������JPG ͼƬ�ļ�
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("yyyy-MM-dd");
	QString new_filepath = path + "/" + report_name + "_" + current_date + "." + file_suffix;
	int rret = rename(sfilepath.toLocal8Bit().data(), new_filepath.toLocal8Bit().data());
	m_plog->Trace("rename result=%d,new_name=%s,old_name=%s", rret, new_filepath.toLocal8Bit().data(), sfilepath.toLocal8Bit().data());

	if (rret != 0)
	{
		Sleep(3000);
		rret = rename(sfilepath.toLocal8Bit().data(), new_filepath.toLocal8Bit().data());
		m_plog->Trace("two rename result=%d,new_name=%s,old_name=%s", rret, new_filepath.toLocal8Bit().data(), sfilepath.toLocal8Bit().data());
	}

	if (!connect_db())              //�����ݿ�����
	{
		m_plog->Trace("connect db fail for %s",filename.toLocal8Bit().data());
		return;
	}
		
	//��ȡ�����
	QString sql = "select sequence_no from VIEW_PEIS_RXCX where PATIENTNO='";
	sql += report_name;
	sql += "';";
	QSqlQuery query_view(m_db);
	query_view.prepare(sql);
	bool bret = query_view.exec();
	if (!bret)
	{
		m_plog->Trace("query sequence_no fail,sql=%s", sql.toLocal8Bit().data());
		close_db();
		return;
	}

	QString sequence_no = "";
	if (query_view.next())
		sequence_no = query_view.value("sequence_no").toString();
	else
	{
		m_plog->Trace("get sequence_no fail by patientno=%s", report_name.toLocal8Bit().data());
		close_db();
		return;
	}

	//���ļ��������ϴ���·�������ϴ�ͼƬ�ļ�
	string file_path = SHANXI12000_Create_Folder(report_name,sequence_no);
	if (file_path != "")
	{
		string new_filename = SHANXI12000_Get_Filename(file_path);
		if (new_filename == "")
		{
			m_plog->Trace("get filename fail,no upload picture %s",filename.toLocal8Bit().data());
			close_db();
			return;
		}
		
		string s_newpath = new_filepath.toStdString();
		wstring wstr(s_newpath.length(),L' ');                       //��ʼ�����ֽ�
		copy(s_newpath.begin(),s_newpath.end(),wstr.begin());        //��str���Ƶ�
		LPCWSTR new_path = wstr.c_str();                             //��wstrת��ΪC�ַ�����ָ��,Ȼ��ֵ��path
		
        string s_new_uploadpath = file_path + "\\" + new_filename;
		wstring wstr_upload(s_new_uploadpath.length(), L' ');
		copy(s_new_uploadpath.begin(), s_new_uploadpath.end(), wstr_upload.begin());
		LPCWSTR new_upload_path = wstr_upload.c_str();

		bret = CopyFile(new_path, new_upload_path, true);
		if (bret)
			m_plog->Trace("upload jpg file %s to %s success", new_filename.c_str(), file_path.c_str());
		else
			m_plog->Trace("upload jpg file %s to %s fail", new_filename.c_str(), file_path.c_str());
	}
	else
	{
		m_plog->Trace("no upload picture file %s", new_filepath.toLocal8Bit().data());
		close_db();
		return;
	}

	//ÿ�ζ��������ӣ�����رգ�����Ϊ�û�������ʱ�����ô����޸�DSN
	close_db();     //�ر����ݿ�����  

}

string CPdfFileWatcher::SHANXI12000_Create_Folder(QString patient_id, QString sequence_no)
{
	QString s_year = patient_id.left(2);
	QString s_month = patient_id.mid(2, 2);
	QString m_share_ip = m_conf.Get("share", "ip").toString();
	QString m_share_folder = m_conf.Get("share", "folder").toString();

	string s_share_path = "\\\\";
	s_share_path += string((const char*)m_share_ip.toLocal8Bit());
	s_share_path += "\\";
	s_share_path += string((const char*)m_share_folder.toLocal8Bit());
	s_share_path += "\\";
	s_share_path += string((const char*)s_year.toLocal8Bit());

	int iret;
	if (_access(s_share_path.c_str(), 0) == -1)        //����ļ��в�����
	{
		iret = _mkdir(s_share_path.c_str());
		if (!iret)
		{
			s_share_path += "\\";
			s_share_path += string((const char*)s_month.toLocal8Bit());
			iret = _mkdir(s_share_path.c_str());       //�����·��ļ���
			if (!iret)
			{
				s_share_path += "\\";
				s_share_path += string((const char*)patient_id.toLocal8Bit());    //���� ���� �ļ���
				iret = _mkdir(s_share_path.c_str());
				if (!iret)
				{
					s_share_path += "\\";
					s_share_path += string((const char*)sequence_no.toLocal8Bit());  //���� ����� �ļ���
					iret = _mkdir(s_share_path.c_str());
					if (iret)
					{
						m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
						return "";
					}
				}
				else
				{
					m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
					return "";
				}

			}
			else
			{
				m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
				return "";
			}
		}
		else
		{
			m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
			return "";
		}

	}//end if (_access(s_share_path.c_str(), 0) == -1)
	else                                                          //����ļ��в�����
	{
		s_share_path += "\\";
		s_share_path += string((const char*)s_month.toLocal8Bit());
		if (_access(s_share_path.c_str(), 0) == -1)      //�·��ļ��в�����
		{
			iret = _mkdir(s_share_path.c_str());         //�����·��ļ���
			if (!iret)
			{
				s_share_path += "\\";
				s_share_path += string((const char*)patient_id.toLocal8Bit());
				iret = _mkdir(s_share_path.c_str());      //���� ���� �ļ���
				if (!iret)
				{
					s_share_path += "\\";
					s_share_path += string((const char*)sequence_no.toLocal8Bit());
					iret = _mkdir(s_share_path.c_str());  //���� ����� �ļ���
					if (iret)
					{
						m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
						return "";
					}
				}
				else
				{
					m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
					return "";
				}
			}
			else
			{
				m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
				return "";
			}

		}
		else    //�·��ļ��д���
		{
			s_share_path += "\\";
			s_share_path += string((const char*)patient_id.toLocal8Bit());
			if (_access(s_share_path.c_str(), 0) == -1)   // ���� �ļ��в�����
			{
				iret = _mkdir(s_share_path.c_str());      //���� ���� �ļ���
				if (!iret)
				{
					s_share_path += "\\";
					s_share_path += string((const char*)sequence_no.toLocal8Bit());
					iret = _mkdir(s_share_path.c_str());  //���� ����� �ļ���
					if (iret)
					{
						m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
						return "";
					}
				}
				else
				{
					m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
					return "";
				}
			}
			else   // ���� �ļ��д���
			{
				s_share_path += "\\";
				s_share_path += string((const char*)sequence_no.toLocal8Bit());
				if (_access(s_share_path.c_str(), 0) == -1)    //����� �ļ��в�����
				{
					iret = _mkdir(s_share_path.c_str());  //���� ����� �ļ���
					if (iret)
					{
						m_plog->Trace("create path %s fail,patientid=%d,sequence_no=%s", s_share_path.c_str(), patient_id.toLocal8Bit().data(), sequence_no.toLocal8Bit().data());
						return "";
					}
				}
			}
		}
	}//end else

	return s_share_path;
}

string CPdfFileWatcher::SHANXI12000_Get_Filename(string path)
{  
	long hFile = 0;   //�ļ����
	struct _finddata_t fileinfo;    //�ļ���Ϣ 
	QString filename = "";
	
	string p = "";
	int imax = 0;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,����֮,�������,�����б�  
			if ((fileinfo.attrib & _A_SUBDIR))
				continue;
			else
			{
				filename = fileinfo.name;
				QStringList qlist = filename.split('.');
				QString report_name = qlist.value(0);      //�ļ���
				QString file_suffix = qlist.value(1);      //��׺
				if (imax < report_name.toInt())
					imax = report_name.toInt();
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

	char name[256];
	memset(name, 0, 256);
	sprintf_s(name, "%d.jpg", imax+1);
	string return_name = name;
	return return_name;
}