#include "CSetDlg.h"

#include <assert.h>
#include <QFileDialog>
#include "mti_tool_2.h"

CSetDlg::CSetDlg(mti_tool_2* ptool, QString path, QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_pmti_tool = ptool;

	assert(path!="");
	m_confpath = path;

	init_control();

	connect(ui.m_bok, SIGNAL(clicked()), this, SLOT(OKClicked()));                           //确定按钮
	connect(ui.m_bcancel, SIGNAL(clicked()), this, SLOT(CancelClicked()));                   //取消按钮
	connect(ui.m_set_logpath, SIGNAL(clicked()), this, SLOT(SetLogPath()));                  //选择日志存放路径按钮
	connect(ui.m_pdf_upload, SIGNAL(activated(int)),this,SLOT(Upload_Mode_Changed()));       //图片上传方式改变
	connect(ui.m_cblog, SIGNAL(activated(int)), this, SLOT(IsLogChanged()));                 //是否写日志控件内容变化 
	connect(ui.m_set_patient_path, SIGNAL(clicked()), this, SLOT(SetPatientPath()));         //设置检查结果存放路径按钮
	connect(ui.m_set_pdf_path, SIGNAL(clicked()), this, SLOT(SetPDFPath()));                 //设置PDF存放路径按钮
}

CSetDlg::~CSetDlg()
{
	//
}

void CSetDlg::setWatchPointer(FileSystemWatcher* pfile, CPdfFileWatcher* ppdf)
{
	assert(pfile);
	assert(ppdf);

	m_pfile_watch = pfile;
	m_pdf_watch = ppdf;
}

void CSetDlg::initControl()
{
	Config conf;
	ui.m_id_len->setText(conf.Get("userinfo","id_len").toString());
	//ui.m_bt_version->setText(conf.Get("userinfo", "version").toString());
	QString s_bt_version = conf.Get("userinfo", "version").toString();
	if ("2.0" == s_bt_version)
		ui.m_bt_version->setCurrentIndex(0);
	else
		ui.m_bt_version->setCurrentIndex(1);

	QString s_have_enter = conf.Get("userinfo", "enter").toString();
	if ("1" == s_have_enter)
		ui.m_have_enter->setCurrentIndex(0);
	else
		ui.m_have_enter->setCurrentIndex(1);

	QString s_pdf_upload = conf.Get("pdf","upload").toString();
	if ("ftp" == s_pdf_upload)
		ui.m_pdf_upload->setCurrentIndex(0);
	else
		ui.m_pdf_upload->setCurrentIndex(1);

	QString s_is_log = conf.Get("log", "tracelog").toString();
	if ("1" == s_is_log)
		ui.m_cblog->setCurrentIndex(0);
	else
		ui.m_cblog->setCurrentIndex(1);

	ui.m_logpath->setText(conf.Get("log", "logpath").toString());
	ui.m_patient_path->setText(conf.Get("report", "path").toString());
	ui.m_pdf_path->setText(conf.Get("pdf", "pdfpath").toString());

	ui.m_ftp_ip->setText(conf.Get("ftp", "ip").toString());
	ui.m_ftp_port->setText(conf.Get("ftp", "port").toString());
	ui.m_ftp_user->setText(conf.Get("ftp", "user").toString());
	ui.m_ftp_psw->setText(conf.Get("ftp", "psw").toString());

	ui.m_share_ip->setText(conf.Get("share","ip").toString());
	ui.m_share_folder->setText(conf.Get("share","folder").toString());

	ui.m_dsn->setText(conf.Get("dsninfo", "dsn").toString());
	ui.m_db_user->setText(conf.Get("dsninfo", "db_user").toString());
	ui.m_db_psw->setText(conf.Get("dsninfo", "db_psw").toString());

	ui.m_tablename->setText(conf.Get("db", "info_tablename").toString());

	QString s_man_store = conf.Get("db","man_store").toString();
	if ("true" == s_man_store)
		ui.m_man_store->setCurrentIndex(1);
	else if("false" == s_man_store)
		ui.m_man_store->setCurrentIndex(2);
	else if("1" == s_man_store)
		ui.m_man_store->setCurrentIndex(3);
	else if("0" == s_man_store)
		ui.m_man_store->setCurrentIndex(4);
	else
		ui.m_man_store->setCurrentIndex(0);

	ui.m_patient_name->setText(conf.Get("db", "patient_name").toString());
	ui.m_patient_age->setText(conf.Get("db", "patient_age").toString());

	QString s_age_convert = conf.Get("db","age_convert").toString();
	if ("1" == s_age_convert)
		ui.m_age_convert->setCurrentIndex(0);
	else
		ui.m_age_convert->setCurrentIndex(1);

	ui.m_patient_id->setText(conf.Get("db", "patient_id").toString());
	ui.m_patient_sex->setText(conf.Get("db", "patient_sex").toString());
	ui.m_db_code->setText(conf.Get("db", "code").toString());

	QString s_db_type = conf.Get("db","type").toString();
	if ("oracle" == s_db_type)
		ui.m_db_type->setCurrentIndex(0);
	else if("sqlserver" == s_db_type)
		ui.m_db_type->setCurrentIndex(1);
	else if("mysql" == s_db_type)
		ui.m_db_type->setCurrentIndex(2);

	Upload_Mode_Changed();
	IsLogChanged();
}

//begin slots functions
void CSetDlg::OKClicked()
{
	set_baseinfo();
	set_fieldinfo();

	close();
}

void CSetDlg::CancelClicked(){ close(); }

void CSetDlg::SetLogPath()
{
	QString dirpath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择目录"), "./", QFileDialog::ShowDirsOnly);
	if (dirpath != "")
	    ui.m_logpath->setText(dirpath);
}

void CSetDlg::Upload_Mode_Changed()
{
	QString s_upload_mode = ui.m_pdf_upload->currentText();
	if ("share" == s_upload_mode)
	{
		ui.groupBox->setEnabled(false);
		ui.label_9->setEnabled(false);
		ui.label_10->setEnabled(false);
		ui.label_11->setEnabled(false);
		ui.label_12->setEnabled(false);
		ui.m_ftp_ip->setEnabled(false);
		ui.m_ftp_port->setEnabled(false);
		ui.m_ftp_user->setEnabled(false);
		ui.m_ftp_psw->setEnabled(false);

		ui.groupBox_6->setEnabled(true);
		ui.label_5->setEnabled(true);
		ui.label_6->setEnabled(true);
		ui.m_share_ip->setEnabled(true);
		ui.m_share_folder->setEnabled(true);

	}
	else if ("ftp" == s_upload_mode)
	{
		ui.groupBox->setEnabled(true);
		ui.label_9->setEnabled(true);
		ui.label_10->setEnabled(true);
		ui.label_11->setEnabled(true);
		ui.label_12->setEnabled(true);
		ui.m_ftp_ip->setEnabled(true);
		ui.m_ftp_port->setEnabled(true);
		ui.m_ftp_user->setEnabled(true);
		ui.m_ftp_psw->setEnabled(true);

		ui.groupBox_6->setEnabled(false);
		ui.label_5->setEnabled(false);
		ui.label_6->setEnabled(false);
		ui.m_share_ip->setEnabled(false);
		ui.m_share_folder->setEnabled(false);
	}
}

void CSetDlg::IsLogChanged()
{
	QString sblog = ui.m_cblog->currentText();
	QString swrite = QString::fromLocal8Bit("否");
	if (!QString::compare(sblog, swrite))
	{
		ui.m_logpath->setEnabled(false);
		ui.m_set_logpath->setEnabled(false);
	}
	else
	{
		ui.m_logpath->setEnabled(true);
		ui.m_set_logpath->setEnabled(true);
	}
}

void CSetDlg::SetPatientPath()
{
	QString dirpath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择目录"), "./", QFileDialog::ShowDirsOnly);
	if(dirpath!="")
	    ui.m_patient_path->setText(dirpath);
}

void CSetDlg::SetPDFPath()
{
	QString dirpath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择目录"), "./", QFileDialog::ShowDirsOnly);
	if (dirpath != "")
	    ui.m_pdf_path->setText(dirpath);
}
//end slots functions

//begin private functions
void CSetDlg::init_control()
{
	ui.m_cblog->addItem(QString::fromLocal8Bit("是"));
	ui.m_cblog->addItem(QString::fromLocal8Bit("否"));
	
	ui.m_have_enter->addItem(QString::fromLocal8Bit("是"));
	ui.m_have_enter->addItem(QString::fromLocal8Bit("否"));

	ui.m_bt_version->addItem(QString::fromLocal8Bit("2.0"));
	ui.m_bt_version->addItem(QString::fromLocal8Bit("1.8.1"));

	ui.m_man_store->addItem(QString::fromLocal8Bit("男"));
	ui.m_man_store->addItem("true");
	ui.m_man_store->addItem("false");
	ui.m_man_store->addItem("1");
	ui.m_man_store->addItem("0");

	ui.m_age_convert->addItem(QString::fromLocal8Bit("是"));
	ui.m_age_convert->addItem(QString::fromLocal8Bit("否"));

	ui.m_pdf_upload->addItem("ftp");
	ui.m_pdf_upload->addItem("share");

	ui.m_db_type->addItem("oracle");
	ui.m_db_type->addItem("sqlserver");
	ui.m_db_type->addItem("mysql");

}

void CSetDlg::set_baseinfo()
{
	//begin 基本信息
	QString s_id_len = ui.m_id_len->text();
	if (s_id_len != "")
	{
		QVariant qv_id_len(s_id_len);
		m_conf.Set("userinfo","id_len",qv_id_len);
	}

	QString s_bt_version = ui.m_bt_version->currentText();
	if (s_bt_version != "")
	{
		QVariant qv_bt_version(s_bt_version);
		m_conf.Set("userinfo","version",qv_bt_version);
	}
	
	QString s_have_enter = ui.m_have_enter->currentText();
	QString s_write_enter = QString::fromLocal8Bit("是");
	if (!QString::compare(s_have_enter, s_write_enter))
	{
		m_pmti_tool->change_has_enter(true);
		s_have_enter = "1";
	}	
	else
	{
		m_pmti_tool->change_has_enter(false);
		s_have_enter = "0";
	}	

	QVariant qv_have_enter(s_have_enter);
	m_conf.Set("userinfo","enter",qv_have_enter);

	QString s_pdf_upload = ui.m_pdf_upload->currentText();
	if (s_pdf_upload != "")
	{
		QVariant qv_pdf_upload(s_pdf_upload);
		m_conf.Set("pdf","upload",qv_pdf_upload);
	}
	//end 基本信息

	//begin 日志相关
	QString sblog = ui.m_cblog->currentText();
	QString swrite = QString::fromLocal8Bit("是");
	if (!QString::compare(sblog, swrite))
		sblog = "1";
	else
		sblog = "0";

	QVariant qv(sblog);
	m_conf.Set("log", "tracelog",qv);

	if(!QString::compare(sblog,"1"))
	{ 
		QString slog_path = ui.m_logpath->text();
		if (slog_path != "")
		{
			QVariant qv_logpath(slog_path);
			m_conf.Set("log", "logpath", qv_logpath);
		}
	}

	if ("1" == sblog)
	{
		m_pmti_tool->set_write_log(true);
		m_pfile_watch->set_write_log(true);
		m_pdf_watch->set_write_log(true);
	}	
	else
	{
		m_pmti_tool->set_write_log(false);
		m_pfile_watch->set_write_log(false);
		m_pdf_watch->set_write_log(false);
	}
	//end 日志相关

	//begin 设置检查结果存放路径
	QString spatient_path = ui.m_patient_path->text();
	if (spatient_path != "")
	{
		QVariant qv_patient(spatient_path);
		m_conf.Set("report", "path", qv_patient);
		m_pfile_watch->modifyWatchPath(spatient_path);
		m_pdf_watch->setReport_path(spatient_path);
	}

	QString spdf_path = ui.m_pdf_path->text();
	if (spdf_path != "")
	{
		QVariant qv_pdf(spdf_path);
		m_conf.Set("pdf", "pdfpath", qv_pdf);
		m_pdf_watch->modifyWatchPath(spdf_path);
	}
	//end 设置检查结果存放路径

	//begin FTP信息
	QString s_ftp_ip = ui.m_ftp_ip->text();
	if (s_ftp_ip != "")
	{
		QVariant qv_ftp_ip(s_ftp_ip);
		m_conf.Set("ftp", "ip", qv_ftp_ip);
	}
	
	QString s_ftp_port = ui.m_ftp_port->text();
	if (s_ftp_port != "")
	{
		QVariant qv_ftp_port(s_ftp_port);
		m_conf.Set("ftp", "port", qv_ftp_port);
	}
	
	QString s_ftp_user = ui.m_ftp_user->text();
	if (s_ftp_user != "")
	{
		QVariant qv_ftp_user(s_ftp_user);
		m_conf.Set("ftp", "user", qv_ftp_user);
	}

	QString s_ftp_psw = ui.m_ftp_psw->text();
	if (s_ftp_psw != "")
	{
		QVariant qv_ftp_psw(s_ftp_psw);
		m_conf.Set("ftp", "psw", qv_ftp_psw);
	}
	//end FTP信息

	//begin share 信息
	QString s_share_ip = ui.m_share_ip->text();
	QVariant qv_share_ip(s_share_ip);
	m_conf.Set("share", "ip", qv_share_ip);

	QString s_share_folder = ui.m_share_folder->text();
	QVariant qv_share_folder(s_share_folder);
	m_conf.Set("share", "folder", qv_share_folder);
	//end share 信息

	//begin 数据源信息
	QString s_dsn = ui.m_dsn->text();
	if (s_dsn!="")
	{
		QVariant qv_dsn(s_dsn);
		m_conf.Set("dsninfo","dsn",qv_dsn);
	}

	QString s_db_user = ui.m_db_user->text();
	if (s_db_user != "")
	{
		QVariant qv_db_user(s_db_user);
		m_conf.Set("dsninfo","db_user",qv_db_user);
	}

	QString s_db_psw = ui.m_db_psw->text();
	if (s_db_psw != "")
	{
		QVariant qv_db_psw(s_db_psw);
		m_conf.Set("dsninfo","db_psw",qv_db_psw);
	}
	//end 数据源信息
}

void CSetDlg::set_fieldinfo()
{
	QString s_tb_name = ui.m_tablename->text();
	if (s_tb_name != "")
	{
		QVariant qv_tb_name(s_tb_name);
		m_conf.Set("db","info_tablename",qv_tb_name);
	}

	QString s_man_store = ui.m_man_store->currentText();
	if (s_man_store != "")
	{
		QVariant qv_man_store(s_man_store);
		m_conf.Set("db","man_store",qv_man_store);
	}

	QString s_patient_name = ui.m_patient_name->text();
	if (s_patient_name != "")
	{
		QVariant qv_patient_name(s_patient_name);
		m_conf.Set("db", "patient_name", qv_patient_name);
	}

	QString s_patient_age = ui.m_patient_age->text();
	if (s_patient_age != "")
	{
		QVariant qv_patient_age(s_patient_age);
		m_conf.Set("db", "patient_age", qv_patient_age);
	}

	QString s_age_convert = ui.m_age_convert->currentText();
	QString s_convert_flag = QString::fromLocal8Bit("是");
	if (!QString::compare(s_age_convert, s_convert_flag))
		s_convert_flag = "1";
	else
		s_convert_flag = "0";

	QVariant qv_age_convert(s_convert_flag);
	m_conf.Set("db","age_convert",qv_age_convert);

	QString s_patient_id = ui.m_patient_id->text();
	if (s_patient_id != "")
	{
		QVariant qv_patient_id(s_patient_id);
		m_conf.Set("db","patient_id",qv_patient_id);
	}

	QString s_patient_sex = ui.m_patient_sex->text();
	if (s_patient_sex != "")
	{
		QVariant qv_patient_sex(s_patient_sex);
		m_conf.Set("db","patient_sex",qv_patient_sex);
	}

	QString s_db_code = ui.m_db_code->text();
	if (s_db_code != "")
	{
		QVariant qv_db_code(s_db_code);
		m_conf.Set("db","code",qv_db_code);
	}

	QString s_db_type = ui.m_db_type->currentText();
	if (s_db_type != "")
	{
		QVariant qv_db_type(s_db_type);
		m_conf.Set("db","type",qv_db_type);
	}
}
//end private functions