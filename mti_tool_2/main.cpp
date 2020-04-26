#include "mti_tool_2.h"
#include "FileSystemWatcher.h"
#include "CPdfFileWatcher.h"
#include "Config.h"

#include <QtWidgets/QApplication>
#include <QMessageBox>

bool checkMyselfExist()             //如果程序已经有一个在运行，则返回true
{
	HANDLE  hMutex = CreateMutex(NULL, FALSE, L"DevState");
	if (hMutex && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		CloseHandle(hMutex);
		hMutex = NULL;
		return true;
	}
	else
		return false;
}

int main(int argc, char *argv[])
{            
	MyApp a(argc, argv);
	bool bexist = checkMyselfExist();
	if (bexist)
	{
		QMessageBox::information(NULL, "alarm", "mti_tool.exe is running", QMessageBox::Yes, QMessageBox::Yes);
        return 0;
	}

	Config conf;
	int is_log = conf.Get("log","tracelog").toInt();
	QString slog_basepath = conf.Get("log","logpath").toString();
	QString sreport_path = conf.Get("report","path").toString();
	QString spdf_path = conf.Get("pdf","pdfpath").toString();

	mti_tool_2 w(is_log,slog_basepath);

	//固定窗体大小
	w.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	w.setMinimumSize(QSize(370, 290));  
	w.setMaximumSize(QSize(370, 290));

	w.show();
	QObject::connect(&a, SIGNAL(getHotF10Key()), &w, SLOT(hotKeyShowWidget()));

	//武汉大学人民医院
	FileSystemWatcher* fsw = nullptr; 
	CPdfFileWatcher* pfw = nullptr;  

	fsw = new FileSystemWatcher(is_log, slog_basepath);
	pfw = new CPdfFileWatcher(is_log, slog_basepath);

	fsw->addWatchPath(sreport_path);
	fsw->setPicture_path(spdf_path);

	//pdf文件增加监控
	pfw->addWatchPath(spdf_path);
	pfw->setReport_path(sreport_path);

	w.setWatchPointer(fsw, pfw);

	int iret = a.exec();
	
	if (fsw)
	{
		delete fsw;
		fsw = nullptr;
	}
	if (pfw)
	{
		delete pfw;
		pfw = nullptr;
	}

	return iret;
}
