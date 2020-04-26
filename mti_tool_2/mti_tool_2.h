#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mti_tool_2.h"
#include "CLog.h"
#include "CSetDlg.h"
#include "FileSystemWatcher.h"
#include "CPdfFileWatcher.h"

#include <windows.h>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMenu>

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QTextCodec>


class MyApp :public QApplication
{
	Q_OBJECT
public:
	MyApp(int &argc, char** argv);
	~MyApp();

	virtual bool winEventFilter(MSG *msg,long *result);
signals:
	//void getShowHotKey();
	void getF10HotKey();
	void getF11HotKey();
};
//end class MyApp

//begin class SystemTray
class SystemTray : public QSystemTrayIcon
{
	Q_OBJECT
public:
	explicit SystemTray(QWidget* parent = 0);
	virtual ~SystemTray();

signals:
	void showWidget();
	void quitWidget();

private:
	void createActions();

private:
	QMenu *m_popMenu;

	QAction* action_show;
	QAction* action_quit;
};
//end class SystemTray

class mti_tool_2 : public QMainWindow
{
	Q_OBJECT

public:
	mti_tool_2(int islog,QString sbasepath,QWidget *parent = Q_NULLPTR);
	~mti_tool_2();

	void registerGlobalKey();
	void setWatchPointer(FileSystemWatcher *pfile, CPdfFileWatcher *ppdf);

	void set_write_log(bool flag);
	int get_hospital_num() { return m_hospital_num; }

	void change_has_enter(bool flag);

public slots:
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void quit();
	void hotKeyShowWidget();
	void idChanged(const QString& text);
	void idChanged1();
	void setInformation();

protected:
	void closeEvent(QCloseEvent *event);

private:
	HWND findMTIApp();
	HWND findMTIPageCtrl(HWND parentHWND);
	HWND findMTIHZTabCtrl(HWND parentHWND);
	HWND findMTIHZGroupCtrl(HWND parentHWND);

	HWND findMTINLCtrl(HWND parentHWND);
	HWND findMTIIDCtrl(HWND parentHWND);
	HWND findMTIXM1Ctrl(HWND parentHWND);
	HWND findMTIXM2Ctrl(HWND parentHWND);
	HWND findMTIManCtrl(HWND parentHWND);
	HWND findMTIWomanCtrl(HWND parentHWND);

	void InitData();
	bool get_userinfo(QString userid);
	bool connect_db();
	void close_db();

	int getAgeByBirthday(QString birthday);      //根据日期计算出年龄

private:
	Ui::mti_tool_2Class ui;
	
	SystemTray* tray;

	QString m_dsn;        //数据源
	QString m_user;       //数据库用户名
	QString m_psw;        //密码
	//QString m_ip;         //数据库所在机器IP
	//QString m_port;       //数据库应用端口

	int m_patientid_len;          //患者ID长度
	int m_hospital_num;           //医院编号
	QString m_beastouch_version;  //触诊软件版本

	QMenu* m_pmenu;
	QAction* myAct_info;
	CSetDlg *m_psetdlg;

	CLog* m_plog;
	Config m_conf;

	FileSystemWatcher *m_pfile_watch;
	CPdfFileWatcher *m_pdf_watch;

	QSqlDatabase m_db;

	QString m_patientNL;          //患者年龄       
	QString m_patientXM1;         //患者姓名
	bool m_patientIsMan;          //患者是否为男性

	QString m_info_tablename;     //存储患者基本信息的表明
	QString m_patient_id;         //体检号在数据库里面的字段名
	QString m_patient_name;       //患者姓名。。。
	QString m_patient_age;        //患者年龄。。。
	QString m_patient_sex;        //患者性别。。。
	QString m_man_store;          //男性的存储方式
	QString m_age_convert;        //年龄是否需要计算
	QString m_db_code;            //数据库编码
	QString m_db_type;            //数据库类型

};
