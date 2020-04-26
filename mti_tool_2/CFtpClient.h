#pragma once

/*******************************************************
author:       gaojy
create date:  2019-11-19
function:     FTP �ͻ�����
********************************************************/

#include "CLog.h"

#include <string>
#include <list>

using namespace std;

#define FTP_DEFAULT_BUFFER			1024*4							//FTP���ػ���Ĭ�ϴ�С

#define FTP_COMMAND_BASE			1000
#define FTP_COMMAND_END				FTP_COMMAND_BASE + 30
#define FTP_COMMAND_USERNAME		FTP_COMMAND_BASE + 1			//�û���
#define FTP_COMMAND_PASSWORD		FTP_COMMAND_BASE + 2			//����
#define FTP_COMMAND_QUIT			FTP_COMMAND_BASE + 3			//�˳�
#define FTP_COMMAND_CURRENT_PATH	FTP_COMMAND_BASE + 4			//��ȡ�ļ�·��
#define FTP_COMMAND_TYPE_MODE		FTP_COMMAND_BASE + 5			//�ı䴫��ģʽ
#define FTP_COMMAND_PSAV_MODE		FTP_COMMAND_BASE + 6			//�����˿�ģʽ
#define FTP_COMMAND_DIR				FTP_COMMAND_BASE + 7			//��ȡ�ļ��б�
#define FTP_COMMAND_CHANGE_DIRECTORY FTP_COMMAND_BASE + 8			//�ı�·��
#define FTP_COMMAND_DELETE_FILE		FTP_COMMAND_BASE + 9			//ɾ���ļ�
#define FTP_COMMAND_DELETE_DIRECTORY FTP_COMMAND_BASE + 10			//ɾ��Ŀ¼/�ļ���
#define FTP_COMMAND_CREATE_DIRECTORY FTP_COMMAND_BASE + 11			//����Ŀ¼/�ļ���
#define FTP_COMMAND_RENAME_BEGIN    FTP_COMMAND_BASE  +12			//��ʼ������
#define FTP_COMMAND_RENAME_END      FTP_COMMAND_BASE + 13			//����������
#define FTP_COMMAND_FILE_SIZE		FTP_COMMAND_BASE + 14			//��ȡ�ļ���С
#define FTP_COMMAND_DOWNLOAD_POS	FTP_COMMAND_BASE + 15			//�����ļ���ָ��λ�ÿ�ʼ
#define FTP_COMMAND_DOWNLOAD_FILE	FTP_COMMAND_BASE + 16			//�����ļ�
#define FTP_COMMAND_UPLOAD_FILE		FTP_COMMAND_BASE + 17			//�ϴ��ļ�
#define FTP_COMMAND_APPEND_FILE		FTP_COMMAND_BASE + 18			//׷�������ļ�	


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
	int login2server(const string& serverip, int port);    //����FTP������
	int inputUserName(const std::string& userName);        //�����û���
	int inputPassWord(const std::string& password);        //��������
	int Put(const string& strRemoteFile, const string& strLocalFile);     //�������ݵ������

	int quitServer(void);                                  //�˳�FTP

//begin no use 
public:
	int delete_file(const string& strRemoteFile);                //ɾ���ļ�
	int set_curr_path(const string& path);                       //���õ�ǰ·��

	int create_directory(const string& strRemoteDir);            //����Ŀ¼/�ļ���
	int delete_directory(const string& strRemoteDir);            //ɾ��Ŀ¼/�ļ���

	int rename(const string& strRemoteFile, const string& strNewFile);     //������
	string getcontent_bypath(const string& path);                          //��ȡָ��Ŀ¼�µ��ļ����ļ���

	int Get(const string& strRemoteFile, const string& strLocalFile);      //�ӷ����������ļ�

//end no use

private:
	int Connect(unsigned int socketfd, const string& ser_ip, int port);
	
	string serverResponse(int sockfd);                                     //��������ftp�����ֵ
	int getData(unsigned int fd,char* strBuf,unsigned long length);        //��ȡ����������
	int parseResponse(const std::string& str);                             //�������������ص�����
	
	string parseCommand(unsigned int command, const string& strParam);     //�ϳɷ��͵�������������
	
	int Send(int fd, const string& cmd);                                   //��������
	int Send(unsigned int fd, const char* cmd, const size_t len);          //��������

	long getFileLength(const std::string& strRemoteFile);   //��ȡ������ļ����ݵĴ�С
	int createDataLink(int data_fd);                        //�����ͻ�����FTP�����֮�������ͨ��
	string Pasv();                                          //���÷����Ϊ����ģʽ
	int SplitString(string strSrc, list<string>& strArray, string strFlag);                  //�����ַ���
	int ParseString(list<std::string> strArray, unsigned long& nPort,string& strServerIp);   //����PASVģʽ���ص��ַ�����ȡFTP�˿ںź�FTP������IP
	void Close(int sock);                                   //�ر��׽���

	int setTransferMode(type mode);                         //���ô����ʽ 2����  ����ascii��ʽ����
 	
	int download(const string &strRemoteFile,const string &strLocalFile,int pos = 0,unsigned int length = 0);   //�����ļ�
	
private:
	CLog *m_plog;

	int m_cmdSocket;                  //���������socket
	string m_strResponse;             //��������Ӧ��Ϣ����
	string m_commandStr;              //�����������
	unsigned int m_nCurrentCommand;   //��ǰʹ�õ��������
	string m_strUserName;             //��ǰ�û���
	string m_strPassWord;             //�û�����

	bool m_bLogin;                    //�Ƿ��ѵ�¼
};

