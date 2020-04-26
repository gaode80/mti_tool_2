#pragma once

/*******************************************************
author:       gaojy
create date:  2019-11-19
function:     FTP 客户端类
********************************************************/

#include "CLog.h"

#include <string>
#include <list>

using namespace std;

#define FTP_DEFAULT_BUFFER			1024*4							//FTP下载缓冲默认大小

#define FTP_COMMAND_BASE			1000
#define FTP_COMMAND_END				FTP_COMMAND_BASE + 30
#define FTP_COMMAND_USERNAME		FTP_COMMAND_BASE + 1			//用户名
#define FTP_COMMAND_PASSWORD		FTP_COMMAND_BASE + 2			//密码
#define FTP_COMMAND_QUIT			FTP_COMMAND_BASE + 3			//退出
#define FTP_COMMAND_CURRENT_PATH	FTP_COMMAND_BASE + 4			//获取文件路径
#define FTP_COMMAND_TYPE_MODE		FTP_COMMAND_BASE + 5			//改变传输模式
#define FTP_COMMAND_PSAV_MODE		FTP_COMMAND_BASE + 6			//被动端口模式
#define FTP_COMMAND_DIR				FTP_COMMAND_BASE + 7			//获取文件列表
#define FTP_COMMAND_CHANGE_DIRECTORY FTP_COMMAND_BASE + 8			//改变路径
#define FTP_COMMAND_DELETE_FILE		FTP_COMMAND_BASE + 9			//删除文件
#define FTP_COMMAND_DELETE_DIRECTORY FTP_COMMAND_BASE + 10			//删除目录/文件夹
#define FTP_COMMAND_CREATE_DIRECTORY FTP_COMMAND_BASE + 11			//创建目录/文件夹
#define FTP_COMMAND_RENAME_BEGIN    FTP_COMMAND_BASE  +12			//开始重命名
#define FTP_COMMAND_RENAME_END      FTP_COMMAND_BASE + 13			//重命名结束
#define FTP_COMMAND_FILE_SIZE		FTP_COMMAND_BASE + 14			//获取文件大小
#define FTP_COMMAND_DOWNLOAD_POS	FTP_COMMAND_BASE + 15			//下载文件从指定位置开始
#define FTP_COMMAND_DOWNLOAD_FILE	FTP_COMMAND_BASE + 16			//下载文件
#define FTP_COMMAND_UPLOAD_FILE		FTP_COMMAND_BASE + 17			//上传文件
#define FTP_COMMAND_APPEND_FILE		FTP_COMMAND_BASE + 18			//追加上载文件	


class CFtpClient
{
public:
	CFtpClient(CLog *plog);
	~CFtpClient();

	enum type 
	{
		binary = 0x31,
		ascii,
	};

public:
	int login2server(const string& serverip, int port);    //连接FTP服务器
	int inputUserName(const std::string& userName);        //输入用户名
	int inputPassWord(const std::string& password);        //输入密码
	int Put(const string& strRemoteFile, const string& strLocalFile);     //发送数据到服务端

	int quitServer(void);                                  //退出FTP

//begin no use 
public:
	int delete_file(const string& strRemoteFile);                //删除文件
	int set_curr_path(const string& path);                       //设置当前路径

	int create_directory(const string& strRemoteDir);            //创建目录/文件夹
	int delete_directory(const string& strRemoteDir);            //删除目录/文件夹

	int rename(const string& strRemoteFile, const string& strNewFile);     //重命名
	string getcontent_bypath(const string& path);                          //获取指定目录下的文件和文件夹

	int Get(const string& strRemoteFile, const string& strLocalFile);      //从服务器下载文件

//end no use

private:
	int Connect(unsigned int socketfd, const string& ser_ip, int port);
	
	string serverResponse(int sockfd);                                     //解析返回ftp命令的值
	int getData(unsigned int fd,char* strBuf,unsigned long length);        //获取服务器数据
	int parseResponse(const std::string& str);                             //解析服务器返回的数据
	
	string parseCommand(unsigned int command, const string& strParam);     //合成发送到服务器的命令
	
	int Send(int fd, const string& cmd);                                   //发送命令
	int Send(unsigned int fd, const char* cmd, const size_t len);          //发送命令

	long getFileLength(const std::string& strRemoteFile);   //获取服务端文件内容的大小
	int createDataLink(int data_fd);                        //创建客户端与FTP服务端之间的数据通道
	string Pasv();                                          //设置服务端为被动模式
	int SplitString(string strSrc, list<string>& strArray, string strFlag);                  //解析字符串
	int ParseString(list<std::string> strArray, unsigned long& nPort,string& strServerIp);   //解析PASV模式返回的字符串获取FTP端口号和FTP服务器IP
	void Close(int sock);                                   //关闭套接字

	int setTransferMode(type mode);                         //设置传输格式 2进制  还是ascii方式传输
 	
	int download(const string &strRemoteFile,const string &strLocalFile,int pos = 0,unsigned int length = 0);   //下载文件
	
private:
	CLog *m_plog;

	int m_cmdSocket;                  //发送命令的socket
	string m_strResponse;             //服务器回应信息缓存
	string m_commandStr;              //保存命令参数
	unsigned int m_nCurrentCommand;   //当前使用的命令参数
	string m_strUserName;             //当前用户名
	string m_strPassWord;             //用户密码

	bool m_bLogin;                    //是否已登录
};

