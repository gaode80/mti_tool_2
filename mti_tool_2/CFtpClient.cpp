#include "CFtpClient.h"

#include <assert.h>
#include <WinSock2.h>
#include <WS2tcpip.h> 
#pragma comment(lib,"ws2_32.lib")

CFtpClient::CFtpClient(CLog *plog)
{
	m_plog = plog;
	assert(m_plog);

	WSADATA wsd;
	int resStartup = WSAStartup(MAKEWORD(2, 2), &wsd);
	if (0 != resStartup)
	{
		m_plog->Trace("init cmd socket fail!");
		return;
	}

	m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);
}

CFtpClient::~CFtpClient()
{
	
}

//begin public function
int CFtpClient::login2server(const string& serverip, int port)
{
	if(Connect(m_cmdSocket, serverip, port) < 0) 
		return -1; 

	m_strResponse = serverResponse(m_cmdSocket);	
	m_plog->Trace("FTP Server return in login2server %s",m_strResponse.c_str());

	return parseResponse(m_strResponse);
}

int CFtpClient::inputUserName(const std::string& userName)
{
	string strCommandLine = parseCommand(FTP_COMMAND_USERNAME, userName); 	
	m_strUserName = userName; 	
	if (Send(m_cmdSocket, strCommandLine) < 0)
	{ 
		m_plog->Trace("send username fail,command=%s",strCommandLine.c_str());
		return -1; 
	} 	
	
	m_strResponse = serverResponse(m_cmdSocket);	
	//printf("Response: %s\n", m_strResponse.c_str()); 	
	m_plog->Trace("send username,ftpserver response = %s", m_strResponse.c_str());

	return parseResponse(m_strResponse);
}

int CFtpClient::inputPassWord(const string& password)
{
	string strCommandLine = parseCommand(FTP_COMMAND_PASSWORD, password);
	m_strPassWord = password;	
	
	if(Send(m_cmdSocket,strCommandLine) < 0)
	{ 
		m_plog->Trace("send password fail,command=%s", strCommandLine.c_str());
		return -1; 
	}

	m_bLogin = true; 		
	m_strResponse = serverResponse(m_cmdSocket);		
	//printf("Response: %s\n", m_strResponse.c_str()); 		
	m_plog->Trace("send password,ftpserver response =%s",m_strResponse.c_str());

	return parseResponse(m_strResponse); 
	
}

int CFtpClient::Put(const string& strRemoteFile, const string& strLocalFile)
{
	string strCmdLine;	
	const unsigned long dataLen = FTP_DEFAULT_BUFFER;	
	char strBuf[dataLen] = { 0 };	
	long nSize = getFileLength(strRemoteFile);	
	unsigned long nLen = 0;

	//FILE* pFile = fopen(strLocalFile.c_str(), "rb");  // 以只读方式打开  且文件必须存在	
	FILE* pFile = nullptr;
	fopen_s(&pFile,strLocalFile.c_str(), "rb");
	assert(pFile != NULL); 	
	
	int data_fd = socket(AF_INET, SOCK_STREAM, 0);	
	assert(data_fd != -1);
	
	if (createDataLink(data_fd) < 0)
	{
		m_plog->Trace("create data link fail in function Put");
		return -1;
	}

	if(nSize == -1) 
		strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFile);
	else  	
		strCmdLine = parseCommand(FTP_COMMAND_APPEND_FILE, strRemoteFile); 

	if (Send(m_cmdSocket,strCmdLine) < 0) 
	{
		m_plog->Trace("send cmd to ftp server fail,cmd=%s",strCmdLine.c_str());
		Close(data_fd);		
		return -1; 
	}
	
	m_plog->Trace("send cmd=%s to ftp server,response=%s",strCmdLine.c_str(), serverResponse(m_cmdSocket).c_str());

	fseek(pFile, nSize, SEEK_SET);	
	while (!feof(pFile)) 
	{ 
		nLen = fread(strBuf,1,dataLen,pFile);		
		if(nLen < 0) 
		 break; 		
		
		if (Send(data_fd, strBuf) < 0) 
		{
			m_plog->Trace("upload file to ftp server fail in function Put");
			Close(data_fd);			
			return -1; 
		} 
	} 	
			
	Close(data_fd);		
	fclose(pFile); 	
	
	return 0;
	
}

int CFtpClient::quitServer(void)
{
    string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");	
	if (Send(m_cmdSocket, strCmdLine) < 0) 
	{
		m_plog->Trace("quit server fail,cmd=%s",strCmdLine.c_str());
		return -1; 
	}
	else 
	{ 
		m_strResponse = serverResponse(m_cmdSocket);			
		m_plog->Trace("quit server ,ftp server resp=%s", m_strResponse.c_str());
		return parseResponse(m_strResponse); 
	}
}
//end public function

//begin public function no use
int CFtpClient::delete_file(const string& strRemoteFile)
{
	assert(m_cmdSocket != INVALID_SOCKET); 	

	string strCmdLine = parseCommand(FTP_COMMAND_DELETE_FILE, strRemoteFile); 	
	
	if(Send(m_cmdSocket,strCmdLine)<0) 
	{ 
		m_plog->Trace("send cmd=%s fail",strCmdLine.c_str());
		return -1; 
	} 	
	
	m_strResponse = serverResponse(m_cmdSocket);		
	m_plog->Trace("send cmd=%s,ftp server response=%s",strCmdLine.c_str(), m_strResponse.c_str());

	return parseResponse(m_strResponse);
}

int CFtpClient::set_curr_path(const string& path)
{
	assert(m_cmdSocket != INVALID_SOCKET); 	

	std::string strCmdLine = parseCommand(FTP_COMMAND_CHANGE_DIRECTORY, path); 	
	if(Send(m_cmdSocket, strCmdLine) < 0) 
	{ 
		m_plog->Trace("send cmd=%s fail", strCmdLine.c_str());
		return -1;
	}			
	
	m_strResponse = serverResponse(m_cmdSocket);			
	m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	return parseResponse(m_strResponse);	
}

int CFtpClient::create_directory(const string& strRemoteDir)
{
	assert(m_cmdSocket != INVALID_SOCKET); 	
    
	string strCmdLine = parseCommand(FTP_COMMAND_CREATE_DIRECTORY, strRemoteDir); 	
	if(Send(m_cmdSocket, strCmdLine) < 0) 
	{ 
		m_plog->Trace("send cmd=%s fail", strCmdLine.c_str());
		return -1; 
	}		
	
	m_strResponse = serverResponse(m_cmdSocket); 	
	m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	return parseResponse(m_strResponse);	
}

int CFtpClient::delete_directory(const string& strRemoteDir)
{
	assert(m_cmdSocket != INVALID_SOCKET); 	
	
	string strCmdLine = parseCommand(FTP_COMMAND_DELETE_DIRECTORY, strRemoteDir); 	
	if (Send(m_cmdSocket, strCmdLine) < 0) 
	{ 
		m_plog->Trace("send cmd=%s fail", strCmdLine.c_str());
		return -1; 
	}		
	
	m_strResponse = serverResponse(m_cmdSocket); 	
	m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	return parseResponse(m_strResponse);	
}

int CFtpClient::rename(const string& strRemoteFile, const string& strNewFile)
{
	assert(m_cmdSocket != INVALID_SOCKET); 	
	
	string strCmdLine = parseCommand(FTP_COMMAND_RENAME_BEGIN, strRemoteFile);	
	
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		m_plog->Trace("send cmd = % s fail", strCmdLine.c_str());
		return -1;
	}
	m_strResponse = serverResponse(m_cmdSocket).c_str();
	m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	string strCmdLine2 = parseCommand(FTP_COMMAND_RENAME_END, strNewFile);
	if (Send(m_cmdSocket, strCmdLine2) < 0)
	{
		m_plog->Trace("send cmd = % s fail", strCmdLine2.c_str());
		return -1;
	}
	m_strResponse = serverResponse(m_cmdSocket);		
	m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine2.c_str(), m_strResponse.c_str());

	return parseResponse(m_strResponse);	
}

string CFtpClient::getcontent_bypath(const string& path)
{
	int dataSocket = socket(AF_INET, SOCK_STREAM, 0); 	
	if (createDataLink(dataSocket) < 0) 
	{ 
		m_plog->Trace("createDataLink fail for FTP_COMMAND_DIR");
		return ""; 
	}	
	
	// 数据连接成功	
	string strCmdLine = parseCommand(FTP_COMMAND_DIR, path); 	
	
	if(Send(m_cmdSocket,strCmdLine) < 0)	
	{		
		m_strResponse = serverResponse(m_cmdSocket).c_str();		
		m_plog->Trace("send cmd = % s fail", strCmdLine.c_str());
		m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine.c_str(), m_strResponse.c_str());
		closesocket(dataSocket);		
		return "";	
	}	
				
	m_strResponse = serverResponse(dataSocket); 
	m_plog->Trace("send cmd=%s,ftp server response=%s", strCmdLine.c_str(), m_strResponse.c_str());		
	closesocket(dataSocket); 		
	
	return m_strResponse;		
}

int CFtpClient::Get(const string& strRemoteFile, const string& strLocalFile)
{
	return download(strRemoteFile, strLocalFile);
}
//end public function no use

//begin private function
int CFtpClient::Connect(unsigned int socketfd, const string& ser_ip, int port)
{
	if (socketfd == INVALID_SOCKET)
	{
		m_plog->Trace("connect socket fail,socketfd is INVALID_SOCKET");
		return -1;
	}

	bool ret = false;
	struct sockaddr_in  addr;
	timeval stime;
	fd_set  set;

	unsigned long argp = 1;
	int error = -1;
	int len = sizeof(int);

	ioctlsocket(socketfd, FIONBIO, &argp);  //设置为非阻塞模式

	memset(&addr, 0, sizeof(struct sockaddr_in));	
	addr.sin_family = AF_INET;	
	addr.sin_port = htons(port);	
	//addr.sin_addr.s_addr = inet_addr(ser_ip.c_str());	
	inet_pton(AF_INET, ser_ip.c_str(),&addr.sin_addr.s_addr);
	//bzero(&(addr.sin_zero), 8);
	memset(&(addr.sin_zero), 0, 8);
	
	if (connect(socketfd, (struct sockaddr*) & addr, sizeof(struct sockaddr)) == -1)   //若直接返回 则说明正在进行TCP三次握手
	{
		stime.tv_sec = 1;         //设置为1秒超时
		stime.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socketfd, &set);

		if (select(socketfd + 1, NULL, &set, NULL, &stime) > 0)   //在这边等待 阻塞 返回可以读的描述符 或者超时返回0  或者出错返回-1		
		{			
			getsockopt(socketfd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t*)&len);			
			if(error == 0)			
				ret = true;						
			else			
				ret = false;					
		}		
	}
	else
	{
		m_plog->Trace("connect ftpserver success,server ip=%s,port = %d", ser_ip.c_str(), port);
		ret = true;
	}

	argp = 0;
	ioctlsocket(socketfd, FIONBIO, &argp);

	if (!ret)
	{
		closesocket(socketfd);
		m_plog->Trace("connect ftpserver fail,server ip=%s,port = %d",ser_ip.c_str(),port);

		return -1;
	}

	return 0;
}

string CFtpClient::serverResponse(int sockfd)
{
	if(sockfd == INVALID_SOCKET)
	{
		m_plog->Trace("socket is INVALID_SOCKET in serverResponse");
		return "";
	}

	int nRet = -1;	
	char buf[MAX_PATH] = { 0 }; 	
	m_strResponse.clear(); 	
	while ((nRet = getData(sockfd, buf, MAX_PATH)) > 0) 
	{ 
		buf[MAX_PATH - 1] = '\0';		
		m_strResponse += buf; 
	} 	
	
	return m_strResponse;
}

int CFtpClient::getData(unsigned int fd, char* strBuf, unsigned long length)
{
	assert(strBuf != NULL);

	if(fd == INVALID_SOCKET)
		return -1;

	memset(strBuf, 0, length);	
	timeval stime;	
	int nLen; 	
	stime.tv_sec = 1;	
	stime.tv_usec = 0; 	

	fd_set	readfd;	
	FD_ZERO(&readfd);	
	FD_SET(fd, &readfd);
	
	if (select(fd + 1, &readfd, 0, 0, &stime) > 0) 
	{ 
		if ((nLen = recv(fd, strBuf, length, 0)) > 0) 
		    return nLen; 
		else 
		    return -2; 
	}	
	
	return 0;
	
}

int CFtpClient::parseResponse(const string& str)
{
	assert(!str.empty());

	string strData = str.substr(0, 3);
	unsigned int val = atoi(strData.c_str());

	return val;
}

string CFtpClient::parseCommand(unsigned int command, const string& strParam)
{
	if(command < FTP_COMMAND_BASE || command > FTP_COMMAND_END)
	{
		return "";
	}

	std::string strCommandLine;
	m_nCurrentCommand = command;
	m_commandStr.clear();

	switch (command)
	{
	case FTP_COMMAND_USERNAME:
		strCommandLine = "USER ";
		break;
	case FTP_COMMAND_PASSWORD:		
	    strCommandLine = "PASS ";		
	    break;	
	case FTP_COMMAND_QUIT:		
	    strCommandLine = "QUIT ";		
	    break;	
	case FTP_COMMAND_CURRENT_PATH:		
		strCommandLine = "PWD ";		
		break;
	case FTP_COMMAND_TYPE_MODE:		
		strCommandLine = "TYPE ";		
		break;	
	case FTP_COMMAND_PSAV_MODE:		
		strCommandLine = "PASV ";		
		break;	
	case FTP_COMMAND_DIR:		
		strCommandLine = "LIST ";		
		break;	
	case FTP_COMMAND_CHANGE_DIRECTORY:		
		strCommandLine = "CWD ";		
		break;	
	case FTP_COMMAND_DELETE_FILE:		
		strCommandLine = "DELE ";		
		break;	
	case FTP_COMMAND_DELETE_DIRECTORY:		
		strCommandLine = "RMD ";		
		break;	
	case FTP_COMMAND_CREATE_DIRECTORY:		
		strCommandLine = "MKD ";		
		break;	
	case FTP_COMMAND_RENAME_BEGIN:		
		strCommandLine = "RNFR ";		
		break;
	case FTP_COMMAND_RENAME_END:		
		strCommandLine = "RNTO ";		
		break;	
	case FTP_COMMAND_FILE_SIZE:		
		strCommandLine = "SIZE ";		
		break;	
	case FTP_COMMAND_DOWNLOAD_FILE:		
		strCommandLine = "RETR ";		
		break;	
	case FTP_COMMAND_DOWNLOAD_POS:		
		strCommandLine = "REST ";		
		break;	
	case FTP_COMMAND_UPLOAD_FILE:		
		strCommandLine = "STOR ";		
		break;	
	case FTP_COMMAND_APPEND_FILE:		
		strCommandLine = "APPE ";		
		break;	
	default:		
		break;		
	}

	strCommandLine += strParam;	
	strCommandLine += "\r\n"; 	
	m_commandStr = strCommandLine;		
	m_plog->Trace("upload command in parseCommand =%s", m_commandStr.c_str());

	return m_commandStr;
}

int CFtpClient::Send(int fd, const string& cmd)
{
	if(fd == INVALID_SOCKET)
		return -1;

	return Send(fd, cmd.c_str(), cmd.length());
}

int CFtpClient::Send(unsigned int fd, const char* cmd, const size_t len)
{
	if((FTP_COMMAND_USERNAME != m_nCurrentCommand) && (FTP_COMMAND_PASSWORD != m_nCurrentCommand) && (!m_bLogin))
	{
		m_plog->Trace("Send fail,curCommand=%d,blogin=%d",m_nCurrentCommand,m_bLogin);
		return -1;
	}

	timeval timeout;	
	timeout.tv_sec = 1;	
	timeout.tv_usec = 0; 	
	
	fd_set  writefd;	
	FD_ZERO(&writefd);  	
	FD_SET(fd, &writefd); 	
	
	if(select(fd+1,0,&writefd,0,&timeout)>0) 
	{ 
		size_t nlen = len; 		
		int nSendLen = 0; 		
		
		while (nlen > 0) 
		{ 
			nSendLen = send(fd, cmd, (int)nlen, 0); 			
			if (nSendLen == -1) 				
				return -2;  			
			
			nlen = nlen - nSendLen;			
			cmd += nSendLen; 
	    }		
		
		return 0; 
	}

	return -1;
}

long CFtpClient::getFileLength(const std::string& strRemoteFile)
{
	assert(m_cmdSocket != INVALID_SOCKET); 

	string strCmdLine = parseCommand(FTP_COMMAND_FILE_SIZE, strRemoteFile); 	
	
	if (Send(m_cmdSocket, strCmdLine) < 0) 
	{ 
		m_plog->Trace("send get file length cmd fail,cmd=%s",strCmdLine.c_str());
		return -1; 
	} 	
	
	m_strResponse = serverResponse(m_cmdSocket); 	
	m_plog->Trace("get file length,ftpserver response=%s",m_strResponse.c_str());

	string strData = m_strResponse.substr(0, 3);	
	unsigned long val = atol(strData.c_str()); 	
	if (val == 213) 
	{ 
		strData = m_strResponse.substr(4);		
		//trace("strData: %s\n", strData.c_str());
		m_plog->Trace("get file length,response data=%d",strData.c_str());

		val = atol(strData.c_str()); 		
		return val; 
	} 

	return -1;
}
int CFtpClient::createDataLink(int data_fd)
{
	assert(data_fd != INVALID_SOCKET); 	

	string strData;	
	unsigned long nPort = 0;	
	string strServerIp; 	
	list<string> strArray; 	

	string parseStr = Pasv(); 	
	if (parseStr.size() <= 0)  
		return -1; 
	
	size_t nBegin = parseStr.find_first_of("(");	
	size_t nEnd = parseStr.find_first_of(")");	
	strData = parseStr.substr(nBegin + 1, nEnd - nBegin - 1); 	
	
	if( SplitString(strData ,strArray ,"," ) <0)		
	    return -1; 	
	
	if( ParseString( strArray , nPort , strServerIp) < 0)		
		return -1; 	
		
	if (Connect(data_fd, strServerIp, nPort) < 0)	
	{	
		m_plog->Trace("connect ftp server data link fail,serverip=%s,port=%d",strServerIp.c_str(),nPort);
		return -1;	
	} 	
	
	m_plog->Trace("connect ftp server data link success,serverip=%s,port=%d", strServerIp.c_str(), nPort);
	return 0;	
}

string CFtpClient::Pasv()
{
	string strCmdLine = parseCommand(FTP_COMMAND_PSAV_MODE, ""); 	
	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0) 
	{ 
		m_plog->Trace("set ftp server PSAV mode fail,cmd=%s",strCmdLine.c_str());
		return ""; 
	}
	else 
	{ 
		m_strResponse = serverResponse(m_cmdSocket); 
		m_plog->Trace("set ftp server PSAV mode success,server response=%s", m_strResponse.c_str());
		return m_strResponse; 
	}	
}

int CFtpClient::SplitString(string strSrc, list<string>& strArray, string strFlag)
{
	int pos = 1;  	
	while ((pos = (int)strSrc.find_first_of(strFlag.c_str())) > 0) 
	{ 
		strArray.insert(strArray.end(), strSrc.substr(0, pos));		
		strSrc = strSrc.substr(pos+1, strSrc.length()-pos-1);
	} 

	strArray.insert(strArray.end(),strSrc.substr(0,strSrc.length())); 

	return 0;
}

int CFtpClient::ParseString(list<std::string> strArray, unsigned long& nPort, string& strServerIp)
{
	if (strArray.size() < 6)		
		return -1; 	
	
	list<std::string>::iterator citor;	
	citor = strArray.begin();	
	strServerIp = *citor;	
	strServerIp += ".";	
	citor++;	
	strServerIp += *citor;	
	strServerIp += ".";	
	citor++;	
	strServerIp += *citor;	
	strServerIp += ".";	
	citor++;	
	strServerIp += *citor;	
	citor = strArray.end();	
	citor--;	
	nPort = atol((*citor).c_str());	
	citor--;	
	nPort += atol((*(citor)).c_str()) * 256;	
	
	return 0;
}

void CFtpClient::Close(int sock)
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
	sock = INVALID_SOCKET;
}

int CFtpClient::setTransferMode(type mode)
{
    string strCmdLine; 	
    switch (mode) 
	{ 
	case binary:		
		strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "I");		
		break;	
	case ascii:		
		strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "A");		
		break;	
	default:		
		break; 
	} 	
	
	if(Send(m_cmdSocket, strCmdLine.c_str()) < 0) 
	{ 
		m_plog->Trace("set cmd to ftp server fail,cmd=%s",strCmdLine.c_str());
		return -1;
	}												   
	
	m_strResponse = serverResponse(m_cmdSocket);		 		
	m_plog->Trace("set cmd=%s to ftp server,response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	return parseResponse(m_strResponse);												   
}

int CFtpClient::download(const string& strRemoteFile,const string& strLocalFile,int pos,unsigned int length)
{
	assert(length >= 0); 	
	
	FILE* file = NULL;	
	unsigned long nDataLen = FTP_DEFAULT_BUFFER;	
	char strPos[MAX_PATH] = { 0 };	
	
	int data_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(data_fd != -1); 	

	if((length != 0) && (length < nDataLen)) 
	    nDataLen = length;	
	
	char* dataBuf = new char[nDataLen];	
	assert(dataBuf != NULL);
	
	sprintf_s(strPos,"%d",pos);
	if(createDataLink(data_fd) < 0)
	{
		m_plog->Trace("create data link fail in download");
		return -1;
	}

	string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);	
	if (Send(m_cmdSocket, strCmdLine) < 0) 
	{ 
		m_plog->Trace("set cmd to ftp server fail,cmd=%s", strCmdLine.c_str());
		return -1; 
	}	
	m_strResponse = serverResponse(m_cmdSocket).c_str();
	m_plog->Trace("set cmd=%s to ftp server,response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE, strRemoteFile); 	
	if (Send(m_cmdSocket, strCmdLine) < 0) 
	{ 
		m_plog->Trace("set cmd to ftp server fail,cmd=%s", strCmdLine.c_str());
		return -1; 
	}	
	m_plog->Trace("set cmd=%s to ftp server,response=%s", strCmdLine.c_str(), m_strResponse.c_str());

	//file = fopen(strLocalFile.c_str(), "w+b");
	fopen_s(&file, strLocalFile.c_str(), "w+b");
	assert(file != NULL);

	int len = 0;	
	unsigned int nReceiveLen = 0;	
	while ((len = getData(data_fd, dataBuf, nDataLen)) > 0) 
	{
		nReceiveLen += len; 		
		/*int num = */fwrite(dataBuf,1,len,file);		
		memset(dataBuf,0,sizeof(dataBuf));			
				
		if(nReceiveLen == length && length != 0)			
			break; 		
		
		if((nReceiveLen + nDataLen) > length  && length != 0)		
		{			
			delete []dataBuf;			
			nDataLen = length - nReceiveLen;			
			dataBuf = new char[nDataLen];		
		}	
	} 	
	
	Close(data_fd);	
	fclose(file);	
	delete []dataBuf; 	
	
	return 0;		
}
//end private function
