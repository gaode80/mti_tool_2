#include "mti_tool_2.h"

//#include "CFtpClient.h"
#include <direct.h>

//begin class MyApp
MyApp::MyApp(int &argc, char** argv):QApplication(argc,argv)
{
	//
}
MyApp::~MyApp()
{
	//
}
bool MyApp::winEventFilter(MSG* msg, long* result)
{
	if (WM_HOTKEY == msg->message)
	{
		if (VK_F11 == msg->wParam)
		{
			emit getF11HotKey();
		}
		else if (VK_F10 == msg->wParam)
		{
			//emit getShowHotKey();
			emit getF10HotKey();
		}
		return true;
	}
	return false;
}
//end class MyApp



//begin class SystemTray
SystemTray::SystemTray(QWidget* parent) : QSystemTrayIcon(parent)
{
	this->createActions();
	this->setContextMenu(m_popMenu);
}

SystemTray::~SystemTray()
{
	//
}

void SystemTray::createActions()
{
	m_popMenu = new QMenu();

	this->setIcon(QIcon("../Resources/images/star.png"));
	this->setToolTip(tr("MTI Tool"));
	this->show();
	// message in bubble
	this->showMessage(tr("MTI Tool"),tr("MTI Tool"),QSystemTrayIcon::Information, 3000);

	//创建托盘项
	action_show = new QAction(this);
	action_quit = new QAction(this);

	//设置托盘项图标
	action_show->setIcon(QIcon(":/icon/open"));
	action_quit->setIcon(QIcon(":/icon/quit"));

	//设置托盘项文本
	action_show->setText(tr("Show"));
	action_quit->setText(tr("Quit"));

	//添加菜单项
	m_popMenu->addAction(action_show);
	m_popMenu->addSeparator();
	m_popMenu->addAction(action_quit);

	//设置信号连接
	QObject::connect(action_show,SIGNAL(triggered(bool)),this, SIGNAL(showWidget()));
	QObject::connect(action_quit,SIGNAL(triggered(bool)),this, SIGNAL(quitWidget()));

	this->setContextMenu(m_popMenu);
	this->show();

}//end createActions
//end class SystemTray



//begin class mti_tool_2
mti_tool_2::mti_tool_2(int islog, QString sbasepath, QWidget *parent) : QMainWindow(parent)
{
	InitData();

	m_plog = new CLog(islog, sbasepath, "mtitool2.log");
	assert(m_plog);
	
	/*
	CFtpClient fc(m_plog);
	fc.login2server("192.168.1.68", 21);
	fc.inputUserName("915689421@qq.com");
	fc.inputPassWord("gaojy19800517");
	//fc.Put("先通康桥.txt", "d:\\先通康桥.txt");
	//fc.delete_file("先通康桥.txt");
	//fc.create_directory("2019");
	//fc.set_curr_path("2019");
	//fc.delete_directory("11");
	//fc.Rename("2019/456.txt", "123.txt");
	fc.Get("2019/123.txt", "d://555.txt");
	*/

	/*
	int irett = _mkdir("\\\\192.168.1.24\\jpg\\123");

	bool brett = CopyFile(L"D:\\123.txt", L"\\\\192.168.1.67\\sharefolder\\123\\123.txt", TRUE);
	
	getAgeByBirthday("1980\\05\\17");
	*/

	QIcon icon("../Resources/images/star.png");   
	this->setWindowIcon(icon);
	ui.setupUi(this);
	this->setWindowTitle("Mti Tool");
	
	//begin 菜单  设置->信息配置
	m_pmenu = menuBar()->addMenu(QString::fromLocal8Bit("设置"));

	myAct_info = new QAction(this);
	myAct_info->setText(QString::fromLocal8Bit("信息配置"));
	myAct_info->setStatusTip(QString::fromLocal8Bit("信息配置"));
	connect(myAct_info, SIGNAL(triggered()), this, SLOT(setInformation()));

	m_pmenu->addAction(myAct_info);

	m_hospital_num = m_conf.Get("hospital", "serial_num").toInt();
	m_psetdlg = new CSetDlg(this,"./Config.ini");
	m_psetdlg->setWindowTitle(QString::fromLocal8Bit("信息设置"));
	m_psetdlg->setAttribute(Qt::WA_ShowModal, true);
	//end 菜单  设置->信息配置

	tray = new SystemTray(this);
	connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	connect(tray,SIGNAL(showWidget()),this,SLOT(showNormal()));
	connect(tray, SIGNAL(quitWidget()),this, SLOT(quit()));

	QString s_enter = m_conf.Get("userinfo","enter").toString();
	if("1" == s_enter.trimmed())
		connect(ui.editID, SIGNAL(returnPressed()), this, SLOT(idChanged1()));
	else
	    connect(ui.editID, SIGNAL(textChanged(const QString&)), this, SLOT(idChanged(const QString&)));

	//托盘显示提示信息
	tray->showMessage(QString("MTI Tool"), QString("MTI Tool"));

	ui.statusBar->showMessage(tr("hello,dear doctor!"));
}

mti_tool_2::~mti_tool_2()
{
	if (m_plog)
	{
		delete m_plog;
		m_plog = nullptr;
	}
}

void mti_tool_2::registerGlobalKey()
{
	if (RegisterHotKey((HWND)this->winId(), 0x79, 0, VK_F10))
	{
		//0x79 is VK_F10, wParam is 121
		qDebug("F10 registered for mti_tool.");
	}
	if (RegisterHotKey((HWND)this->winId(), 0x7A, 0, VK_F11))
	{
		//wParam is 122
		qDebug("F11 registered for mti_tool.");
	}
}

void mti_tool_2::setWatchPointer(FileSystemWatcher* pfile, CPdfFileWatcher* ppdf)
{
	assert(pfile);
	assert(ppdf);

	m_pfile_watch = pfile;
	m_pdf_watch = ppdf;

	m_psetdlg->setWatchPointer(m_pfile_watch, m_pdf_watch);
}

void mti_tool_2::set_write_log(bool flag)
{
	m_plog->set_logflag(flag);
}

void mti_tool_2::change_has_enter(bool flag)
{
	if (flag)
	{
		disconnect(ui.editID, SIGNAL(returnPressed()), this, SLOT(idChanged1()));
		disconnect(ui.editID, SIGNAL(textChanged(const QString&)), this, SLOT(idChanged(const QString&)));
		connect(ui.editID, SIGNAL(returnPressed()), this, SLOT(idChanged1()));
	}
	else
	{
		disconnect(ui.editID, SIGNAL(returnPressed()), this, SLOT(idChanged1()));
		disconnect(ui.editID, SIGNAL(textChanged(const QString&)), this, SLOT(idChanged(const QString&)));
		connect(ui.editID, SIGNAL(textChanged(const QString&)), this, SLOT(idChanged(const QString&)));
	}	
}

//begin slots functions
void mti_tool_2::setInformation()
{
	m_psetdlg->initControl();
	m_psetdlg->show();
}

void mti_tool_2::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
	{
		showNormal();
		break;
	}
	default:
		break;
	}
}

void mti_tool_2::quit()
{
	tray->hide();
	qApp->quit();
}

void mti_tool_2::hotKeyShowWidget()
{
	showNormal();
}

void mti_tool_2::idChanged1()
{
	QString stext = ui.editID->text();
	m_patientid_len = m_conf.Get("userinfo", "id_len").toInt();

	if (m_patientid_len == stext.size())
	{
		bool db_get_info_ok = false;

		QString sID(stext);    //病人ID
		QString sNL("100");    //年龄
		QString sXM1("aaa");   //姓名
		//QString sXM2("bbb");   //姓名2
		//QString sCSNYR("");    //出生日期

		bool isMan(false);
		bool bret = get_userinfo(sID);
		if (bret)
		{
			db_get_info_ok = true;

			sNL = m_patientNL;
			sXM1 = m_patientXM1;
			isMan = m_patientIsMan;

			m_plog->Trace("get userinfo by patientID success,patientID=%s,name=%s,age=%s",
				           sID.toLatin1().data(), sXM1.toUtf8().data(), sNL.toLatin1().data());
		}

		//写入本地控件
		if (m_patientid_len == stext.size() && db_get_info_ok)
		{
			ui.editNL->setText(sNL);
			ui.editXM->setText(sXM1);
			//ui.editCSNYR->setText(sCSNYR);

			ui.editXB->setText(isMan ? "Man" : "WoMan");

			ui.statusBar->showMessage(tr("HIS DB read OK!"));
			this->showMinimized();
		}
		else
		{
			if (m_patientid_len == stext.size())
			{
				ui.editID->setText("");
			}

			ui.editNL->setText("");
			ui.editXM->setText("");
			ui.editCSNYR->setText("");
			ui.editXB->setText("");
			
			ui.statusBar->showMessage(tr("HIS DB read ERROR!"));
			return;
		}
		//end 写入本地控件

		//把这些控件内容清空
		ui.editID->setText("");
		//ui.editNL->setText("");
		//ui.editXM->setText("");
		//ui.editCSNYR->setText("");
		//ui.editXB->setText("");

		// 写入SureTouch窗口控件
		if (db_get_info_ok)
		{
			//查找主窗口
			HWND AppWnd = findMTIApp();
			if (!AppWnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("can't get MTI Window Handle!"));
				ui.statusBar->showMessage(tr("can't get MTI Window Handle!"));
				return;
			}

			//查找TAB页的父窗口
			HWND wnd = findMTIPageCtrl(AppWnd);
			if (!wnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("error when fine TPageControl!"));
				ui.statusBar->showMessage(tr("error when fine TPageControl!"));
				return;
			}

			//查找患者/医师TAB页
			wnd = findMTIHZTabCtrl(wnd);
			if (!wnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("error when fine TTabSheet!"));
				ui.statusBar->showMessage(tr("error when fine TTabSheet!"));
				return;
			}

			//查找患者信息 group 控件
			wnd = findMTIHZGroupCtrl(wnd);
			if (!wnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("error when fine TGroupBox!"));
				ui.statusBar->showMessage(tr("error when fine TGroupBox!"));
				return;
			}

			//年龄
			HWND wnd_NL = findMTINLCtrl(wnd);
			::SendMessage(wnd_NL, WM_SETTEXT, 0, (LPARAM)sNL.data());

			//ID
			HWND wnd_ID = findMTIIDCtrl(wnd);
			::SendMessage(wnd_ID, WM_SETTEXT, 0, (LPARAM)sID.data());

			//姓名--拼音
			//HWND wnd_XM2 = findMTIXM2Ctrl(wnd);
			//::SendMessage(wnd_XM2, WM_SETTEXT, 0, (LPARAM)sXM2.data());

			//姓名
			HWND wnd_XM1 = findMTIXM1Ctrl(wnd);
			::SendMessage(wnd_XM1, WM_SETTEXT, 0, (LPARAM)sXM1.data());

			if (isMan)
			{
				HWND wnd_Man = findMTIManCtrl(wnd);
				::SendMessage(wnd_Man, BM_CLICK, 0, 0);
			}
			else
			{
				HWND wnd_Woman = findMTIWomanCtrl(wnd);
				::SendMessage(wnd_Woman, BM_CLICK, 0, 0);
			}
		}//end if (db_get_info_ok)
	}
}

void mti_tool_2::idChanged(const QString& text)
{
	m_patientid_len = m_conf.Get("userinfo", "id_len").toInt();
	
	if (m_patientid_len == text.size())
	{
		bool db_get_info_ok = false;

		QString sID(text);     //病人ID
		QString sNL("100");    //年龄
		QString sXM1("aaa");   //姓名
		//QString sXM2("bbb");   //姓名2
		//QString sCSNYR("");    //出生日期


		bool isMan(false);
		bool bret = get_userinfo(sID);
		if (bret)
		{
			db_get_info_ok = true;

			sNL = m_patientNL;
			sXM1 = m_patientXM1;
			isMan = m_patientIsMan;

			m_plog->Trace("get userinfo by patientID success,patientID=%s,name=%s,age=%s",
				sID.toLatin1().data(), sXM1.toUtf8().data(), sNL.toLatin1().data());
		}

		//写入本地控件
		if (m_patientid_len == text.size() && db_get_info_ok)
		{
			ui.editNL->setText(sNL);
			ui.editXM->setText(sXM1);
			//ui.editCSNYR->setText(sCSNYR);

			ui.editXB->setText(isMan?"Man":"WoMan");

			ui.statusBar->showMessage(tr("HIS DB read OK!"));
			this->showMinimized();
		}
		else
		{
			if (m_patientid_len == text.size())
			{
				ui.editID->setText("");
			}

			ui.editNL->setText("");
			ui.editXM->setText("");
			ui.editCSNYR->setText("");
			ui.editXB->setText("");

			ui.statusBar->showMessage(tr("HIS DB read ERROR!"));
			return;
		}
		//end 写入本地控件

		//把这些控件内容清空
		ui.editID->setText("");
		//ui.editNL->setText("");
		//ui.editXM->setText("");
		//ui.editCSNYR->setText("");
		//ui.editXB->setText("");

		// 写入SureTouch窗口控件
		if (db_get_info_ok)
		{
			//查找主窗口
			HWND AppWnd = findMTIApp();
			if (!AppWnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("can't get MTI Window Handle!"));
				ui.statusBar->showMessage(tr("can't get MTI Window Handle!"));
				return;
			}

			//查找TAB页的父窗口
			HWND wnd = findMTIPageCtrl(AppWnd);
			if (!wnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("error when fine TPageControl!"));
				ui.statusBar->showMessage(tr("error when fine TPageControl!"));
				return;
			}

			//查找患者/医师TAB页
			wnd = findMTIHZTabCtrl(wnd);
			if (!wnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("error when fine TTabSheet!"));
				ui.statusBar->showMessage(tr("error when fine TTabSheet!"));
				return;
			}

			//查找患者信息 group 控件
			wnd = findMTIHZGroupCtrl(wnd);
			if (!wnd)
			{
				//QMessageBox::information(this, tr("Warning"), tr("error when fine TGroupBox!"));
				ui.statusBar->showMessage(tr("error when fine TGroupBox!"));
				return;
			}

			//年龄
			HWND wnd_NL = findMTINLCtrl(wnd);
			::SendMessage(wnd_NL, WM_SETTEXT, 0, (LPARAM)sNL.data());

			//ID
			HWND wnd_ID = findMTIIDCtrl(wnd);
			::SendMessage(wnd_ID, WM_SETTEXT, 0, (LPARAM)sID.data());

			//姓名--拼音
			//HWND wnd_XM2 = findMTIXM2Ctrl(wnd);
			//::SendMessage(wnd_XM2, WM_SETTEXT, 0, (LPARAM)sXM2.data());

			//姓名
			HWND wnd_XM1 = findMTIXM1Ctrl(wnd);
			::SendMessage(wnd_XM1, WM_SETTEXT, 0, (LPARAM)sXM1.data());

			if (isMan)
			{
				HWND wnd_Man = findMTIManCtrl(wnd);
				::SendMessage(wnd_Man, BM_CLICK, 0, 0);
			}
			else
			{
				HWND wnd_Woman = findMTIWomanCtrl(wnd);
				::SendMessage(wnd_Woman, BM_CLICK, 0, 0);
			}
		}//end if (db_get_info_ok)
	}
}//end idChanged
//end slots functions

void mti_tool_2::closeEvent(QCloseEvent* event)
{
	if (tray->isVisible())
	{
		// minimize
		hide();
		event->ignore();
	}
	else
	{
		event->accept();
	}
}


//begin 查找窗体函数集
HWND mti_tool_2::findMTIApp()
{
	HWND AppWnd;
	QString ClassName("TMainForm");
	QString AppName = "";
	m_beastouch_version = m_conf.Get("userinfo", "version").toString();
	if("2.0" == m_beastouch_version)
	    AppName = "BreasTouch v2.0";
	else if("1.8.1" == m_beastouch_version)
	    AppName = "SureTouch v1.8.1";
	else
		AppName = "BreasTouch v2.0";

	LPCWSTR App = reinterpret_cast <LPCWSTR>(ClassName.data());
	LPCWSTR AppCaption = reinterpret_cast <LPCWSTR>(AppName.data());

	AppWnd = ::FindWindowW(App, AppCaption);
	if (nullptr == AppWnd)
		m_plog->Trace("find mainfram fail,classname=%s,appname=%s",ClassName.toLatin1().data(),AppName.toLatin1().data());

	return AppWnd;
}

HWND mti_tool_2::findMTIPageCtrl(HWND parentHWND)
{
	HWND wnd;
	{
		QString className("TPageControl");
		QString appName(tr(""));
		LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
		LPCWSTR lpszWindow = reinterpret_cast <LPCWSTR>(appName.data());

		wnd = ::FindWindowEx(parentHWND, 0, lpszClass, lpszWindow);
		if (nullptr == wnd)
			m_plog->Trace("find pagecontrol fail,classname = %s",className.toLatin1().data());

		return wnd;
	}
}

HWND mti_tool_2::findMTIHZTabCtrl(HWND parentHWND)
{
	QString className("TTabSheet");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	QString sTitle("");
	do
	{
		wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
		/*if (wnd)
		{
			int nCharacters = GetWindowTextLength(wnd);
			GetWindowText(wnd, lpszWindow, 100);
			lpszWindow[nCharacters] = 0;
			sTitle = QString::fromWCharArray(lpszWindow);
		}*/
		
	} while (!wnd);

	if (nullptr == wnd)
		m_plog->Trace("find TabCtrl fail,classname=%s",className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTIHZGroupCtrl(HWND parentHWND)
{
	QString className("TGroupBox");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	QString sTitle("");
	do
	{
		wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
		wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
		/*if (wnd)
		{
		int nCharacters = GetWindowTextLength(wnd);
		GetWindowText(wnd, lpszWindow, 100);
		lpszWindow[nCharacters] = 0;
		sTitle = QString::fromWCharArray(lpszWindow);
		}*/
	} while (!wnd);

	if (nullptr == wnd)
		m_plog->Trace("find GroupCtrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTINLCtrl(HWND parentHWND)
{
	QString className("TEdit");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	if (nullptr == wnd)
		m_plog->Trace("find NLCtrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTIIDCtrl(HWND parentHWND)
{
	QString className("TEdit");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	if (nullptr == wnd)
		m_plog->Trace("find IDCtrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTIXM2Ctrl(HWND parentHWND)
{
	QString className("TEdit");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	if (nullptr == wnd)
		m_plog->Trace("find XM2Ctrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTIXM1Ctrl(HWND parentHWND)
{
	QString className("TEdit");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	if (nullptr == wnd)
		m_plog->Trace("find XM1Ctrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTIManCtrl(HWND parentHWND)
{
	QString className("TRadioButton");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	if (nullptr == wnd)
		m_plog->Trace("find ManCtrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}

HWND mti_tool_2::findMTIWomanCtrl(HWND parentHWND)
{
	QString className("TRadioButton");
	QString appName(tr(""));
	LPCWSTR lpszClass = reinterpret_cast <LPCWSTR>(className.data());
	//LPWSTR lpszWindow = reinterpret_cast <LPWSTR>(appName.data());

	HWND wnd = 0;
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	wnd = ::FindWindowEx(parentHWND, wnd, lpszClass, 0);
	if (nullptr == wnd)
		m_plog->Trace("find WomanCtrl fail,classname=%s", className.toLatin1().data());

	return wnd;
}
//end 查找窗体函数集

inline void mti_tool_2::InitData()
{
	m_dsn="";        //数据源
	m_user="";       //数据库用户名
	m_psw="";        //密码
	//m_ip="";         //数据库所在机器IP
	//m_port="";       //数据库应用端口

	m_patientid_len =10;    //患者ID长度

	m_patientNL = "";
	m_patientXM1 = "";
	m_patientIsMan = false;

	m_plog = nullptr;
}

bool mti_tool_2::get_userinfo(QString userid)
{
	bool bret = true;

	bool bconnect = connect_db();
	if (!bconnect)
	{
		m_plog->Trace("connect DB failed,dsn=%s,user=%s,psw=%s",m_dsn.toLatin1().data(), m_user.toLatin1().data(), m_psw.toLatin1().data());
		return false;
	}

	m_info_tablename = m_conf.Get("db", "info_tablename").toString();
	m_patient_id = m_conf.Get("db", "patient_id").toString();
	m_patient_name = m_conf.Get("db", "patient_name").toString();
	m_patient_age = m_conf.Get("db", "patient_age").toString();
	m_patient_sex = m_conf.Get("db", "patient_sex").toString();

	m_man_store = m_conf.Get("db", "man_store").toString();
	m_age_convert = m_conf.Get("db", "age_convert").toString();
	m_db_code = m_conf.Get("db", "code").toString();
	m_db_type = m_conf.Get("db","type").toString();

	QString sql = "";
	if ("oracle" == m_db_type)
	{
		sql = "select " + m_patient_id;
		sql += ",convert(" + m_patient_name + ", 'utf8', '" + m_db_code + "') as " + m_patient_name + ",";
		sql += m_patient_age + ",convert(" + m_patient_sex + ", 'utf8', '" + m_db_code + "') as " + m_patient_sex + " from " + m_info_tablename;
		sql += " where " + m_patient_id + "=" + userid;
	}
	else if("sqlserver" == m_db_type)
	{
		sql = "select " + m_patient_id;
		sql += "," + m_patient_name + "," + m_patient_age + "," + m_patient_sex + " from " + m_info_tablename;
		sql += " where " + m_patient_id + "=" + userid;
	}
	else if ("mysql" == m_db_type)
	{
		//
	}

	QSqlQuery query(m_db);
	query.prepare(sql);
	if (query.exec())
	{
		if (query.next())
		{
			if ("0" == m_age_convert)
				m_patientNL = query.value(m_patient_age).toString();
			else
			{
				QString sbirth_day = query.value(m_patient_age).toString();
				int iage = getAgeByBirthday(sbirth_day);
				m_patientNL = QString::number(iage);
			}
			m_patientXM1 = query.value(m_patient_name).toString();
			m_patientIsMan = query.value(m_patient_sex).toString().contains(m_man_store);

			m_patientXM1 = m_patientXM1.trimmed();
		}
		else
		    bret = false;
	}
	else
		bret = false;

	if (!bret)
	{
		m_plog->Trace("get userinfo fail by id=%s",userid.toLocal8Bit().data());
		QMessageBox::critical(0, "alarm", "get userinfo fail");
	}
		
	close_db();
	return bret;
}

bool mti_tool_2::connect_db()
{
	if (!m_db.isOpen())
	{
		m_dsn = m_conf.Get("dsninfo", "dsn").toString();
		m_user = m_conf.Get("dsninfo", "db_user").toString();
		m_psw = m_conf.Get("dsninfo", "db_psw").toString();

		m_db = QSqlDatabase::addDatabase("QODBC");

		//***数据源配置时填写的那个DataSourceName
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

void mti_tool_2::close_db()
{
	if (m_db.isOpen())
		m_db.close();
}

int mti_tool_2::getAgeByBirthday(QString birthday)
{
	QString split = "";
	QString qtemp = "";
	int iage = 0;

	if (birthday.indexOf("-") >= 0)
		split = "-";
	else if (birthday.indexOf("/") >= 0)
		split = "/";
	else if (birthday.indexOf("\\") >= 0)
		split = "\\";

	int patient_year, patient_month, patient_day;
	int curr_year, curr_month, curr_day;

	QStringList qls = birthday.split(split);

	qtemp = qls.at(0);
	patient_year = qtemp.toInt();
	qtemp = qls.at(1);
	patient_month = qtemp.toInt();
	qtemp = qls.at(2);
	if (qtemp.length() > 2)
		qtemp = qtemp.left(2);
	patient_day = qtemp.toInt();

	time_t rawtime;
	//struct tm* ptminfo;
	struct tm tminfo;

	time(&rawtime);
	//ptminfo = localtime(&rawtime);
	localtime_s(&tminfo, &rawtime);
	curr_year = tminfo.tm_year + 1900;
	curr_month = tminfo.tm_mon + 1;
	curr_day = tminfo.tm_mday;

	//curr_month = 5;
	//curr_day = 17;
	iage = curr_year - patient_year;
	if (patient_month > curr_month)
		iage -= 1;
	else if (patient_month == curr_month)
	{
		if (patient_day > curr_day)
			iage -= 1;
	}

	return iage;
}
//end class mti_tool_2