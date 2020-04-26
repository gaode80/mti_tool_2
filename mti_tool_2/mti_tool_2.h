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

	int getAgeByBirthday(QString birthday);      //�������ڼ��������

private:
	Ui::mti_tool_2Class ui;
	
	SystemTray* tray;

	QString m_dsn;        //����Դ
	QString m_user;       //���ݿ��û���
	QString m_psw;        //����
	//QString m_ip;         //���ݿ����ڻ���IP
	//QString m_port;       //���ݿ�Ӧ�ö˿�

	int m_patientid_len;          //����ID����
	int m_hospital_num;           //ҽԺ���
	QString m_beastouch_version;  //��������汾

	QMenu* m_pmenu;
	QAction* myAct_info;
	CSetDlg *m_psetdlg;

	CLog* m_plog;
	Config m_conf;

	FileSystemWatcher *m_pfile_watch;
	CPdfFileWatcher *m_pdf_watch;

	QSqlDatabase m_db;

	QString m_patientNL;          //��������       
	QString m_patientXM1;         //��������
	bool m_patientIsMan;          //�����Ƿ�Ϊ����

	QString m_info_tablename;     //�洢���߻�����Ϣ�ı���
	QString m_patient_id;         //���������ݿ�������ֶ���
	QString m_patient_name;       //��������������
	QString m_patient_age;        //�������䡣����
	QString m_patient_sex;        //�����Ա𡣡���
	QString m_man_store;          //���ԵĴ洢��ʽ
	QString m_age_convert;        //�����Ƿ���Ҫ����
	QString m_db_code;            //���ݿ����
	QString m_db_type;            //���ݿ�����

};
