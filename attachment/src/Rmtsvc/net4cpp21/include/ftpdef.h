/*******************************************************************
   *	ftpdef.h
   *    DESCRIPTION:����ftpЭ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-16
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_FTPDEF_H__
#define __YY_FTPDEF_H__

#include <map>
#include <string>
#include "IPRules.h"

//FTP��������
#define FTP_SERVER_PORT	21 //Ĭ��FTP����Ķ˿�
#define FTP_MAX_COMMAND_SIZE 256 //FTP��������ֽڳ���
#define FTP_MAX_LOGINTIMEOUT 10 //s FTP�ͻ������¼��ʱ��������ʱ����ر�����

//ftp���ؽ����������
#define SOCKSERR_FTP_RESP -301 //������Ӧ����
#define SOCKSERR_FTP_AUTH SOCKSERR_FTP_RESP-1 //ftp��֤ʧ��
#define SOCKSERR_FTP_SURPPORT SOCKSERR_FTP_RESP-2 //��֧�ֵ�ftp������ܴ��䷽ʽ
#define SOCKSERR_FTP_REST SOCKSERR_FTP_RESP-3 //��վ�㲻֧�ֶϵ�����
#define SOCKSERR_FTP_DATACONN SOCKSERR_FTP_RESP-4 //�����������ݶ˿�
#define SOCKSERR_FTP_LIST SOCKSERR_FTP_RESP-5 //ListӦ�����
#define SOCKSERR_FTP_RETR SOCKSERR_FTP_RESP-6 //RETRӦ�����
#define SOCKSERR_FTP_STOR SOCKSERR_FTP_RESP-7 //StorӦ�����
#define SOCKSERR_FTP_FILE SOCKSERR_FTP_RESP-8 //�ļ���������
#define SOCKSERR_FTP_FAILED SOCKSERR_FTP_RESP-9 //һ���Դ���
#define SOCKSERR_FTP_DENY SOCKSERR_FTP_RESP-10 //���ʾܾ�
#define SOCKSERR_FTP_NOEXIST SOCKSERR_FTP_RESP-11 //Ŀ¼���ļ�������
#define SOCKSERR_FTP_EXIST SOCKSERR_FTP_RESP-12 //Ŀ¼���ļ�����
#define SOCKSERR_FTP_USER SOCKSERR_FTP_RESP-13 //�����ڵ��ʺ�
#define SOCKSERR_FTP_UNKNOWED SOCKSERR_FTP_RESP-14 //δ֪����

#define FTP_DATACONN_PORT 0 //FTP���ݴ���ģʽ
#define FTP_DATACONN_PASV 1

//ftp��־�¼�����
#define FTP_LOGEVENT_LOGIN 1
#define FTP_LOGEVENT_LOGOUT 2
#define FTP_LOGEVENT_UPLOAD 4
#define FTP_LOGEVENT_DWLOAD 8
#define FTP_LOGEVENT_DELETE 16
#define FTP_LOGEVENT_RMD 32
#define FTP_LOGEVENT_SITE 64

//������֤���ܷ�ʽ
#define OTP_NONE 0
#define OTP_MD4 1
#define OTP_MD5 2

//�����ʺ�����
#define ACCOUNT_NORMAL 0
#define ACCOUNT_ADMIN 1
#define ACCOUNT_ROOT 2

typedef struct _ftpaccount //ftp�ʺ���Ϣ
{
	std::string m_username;//�ʺ�,�ʺŲ����ִ�Сд(�ʺ�ת��ΪСд)
	std::string m_userpwd;//����,�������==""������������֤
	std::string m_username_root;//���ʺ�������ROOTȨ���û��������ֵ��Ϊ��˵�����ʺ���ROOT�û���̬������
	unsigned long m_maxupratio;//����������� K/s,���=0����
	unsigned long m_maxdwratio;//����������� K/s,���=0����
	unsigned long m_maxupfilesize;//��������ļ���С KBytes ,0--����
	unsigned long m_maxdisksize;//����������ʹ�ÿռ� KBytes,0--����
	unsigned long m_curdisksize;//��ǰ��ʹ�ô��̿ռ� KBytes.
	std::map<std::string,std::pair<std::string,long> > m_dirAccess;//Ŀ¼����Ȩ��,Ŀ¼���ִ�Сд
			//first --- string : ftp����Ŀ¼·���������/����������/ �� /aa/��
			//second --- pair : ��ftp��Ŀ¼��Ӧ��ʵ��Ŀ¼��Ŀ¼�ķ���Ȩ�ޣ�ʵ��Ŀ¼����Ϊ\��β(winƽ̨)
	net4cpp21::iprules m_ipRules;//ip���ʹ���
	long m_loginusers;//��ǰ�Դ��ʺŵ�¼ftp������û�����,ֻ��û���û�����ʱ����ɾ�����ʺ�
	long m_maxLoginusers;//���ƴ��ʺŵ����ͬʱ��¼�û���,<=0������ 
	time_t m_limitedTime;//���ƴ��ʺ�ֻ��ĳ������֮ǰ��Ч��==0������
	long m_bitQX; //0~1bit ������֤��ʽ
				  //2~3bit �����ʺ�����
				  //4bit �Ƿ���ʾ�����ļ�

	bool bDsphidefiles() { return ((m_bitQX & 0x10)!=0); }
	void bDsphidefiles(bool b)
	{
		if(b)
			m_bitQX |=0x10;
		else
			m_bitQX &=0xffffffef;
		return;
	}
	long lRemoteAdmin() { return (m_bitQX & 0x0c)>>2; }
	void lRemoteAdmin(long l)
	{
		l =(l & 0x3)<<2; 
		m_bitQX &=0xfffffff3;
		m_bitQX |=l; return;
	}
	long lPswdMode() { return (m_bitQX & 0x03); }
	void lPswdMode(long l)
	{
		l &=0x3; 
		m_bitQX &=0xfffffffc;
		m_bitQX |=l; return;
	}
}FTPACCOUNT;

//FTP��дȨ�޳�������
#define FTP_ACCESS_FILE_READ 1 //�ļ�������
#define FTP_ACCESS_FILE_WRITE 2 //�ļ�д����
#define FTP_ACCESS_FILE_DELETE 4 //�ļ�ɾ������
#define FTP_ACCESS_FILE_EXEC 8 //�ļ�ִ������
#define FTP_ACCESS_DIR_LIST 16 //����Ŀ¼���
#define FTP_ACCESS_DIR_CREATE 32 //������Ŀ¼
#define FTP_ACCESS_DIR_DELETE 64 //����ɾ��Ŀ¼
#define FTP_ACCESS_SUBDIR_INHERIT 128 //��Ŀ¼�̳н�ֹ

#define FTP_ACCESS_ALL 0x7f
#define FTP_ACCESS_NONE 0x0
#endif

/*
The following are the FTP commands:

            USER <SP> <username> <CRLF>
            PASS <SP> <password> <CRLF>
            ACCT <SP> <account-information> <CRLF>
            CWD  <SP> <pathname> <CRLF>
            CDUP <CRLF>
            SMNT <SP> <pathname> <CRLF>
            QUIT <CRLF>
            REIN <CRLF>
            PORT <SP> <host-port> <CRLF>
            PASV <CRLF>
            TYPE <SP> <type-code> <CRLF>
            STRU <SP> <structure-code> <CRLF>
            MODE <SP> <mode-code> <CRLF>
            RETR <SP> <pathname> <CRLF>
            STOR <SP> <pathname> <CRLF>
            STOU <CRLF>
            APPE <SP> <pathname> <CRLF>
            ALLO <SP> <decimal-integer>
                [<SP> R <SP> <decimal-integer>] <CRLF>
            REST <SP> <marker> <CRLF>
            RNFR <SP> <pathname> <CRLF>
            RNTO <SP> <pathname> <CRLF>
            ABOR <CRLF>
            DELE <SP> <pathname> <CRLF>
            RMD  <SP> <pathname> <CRLF>
            MKD  <SP> <pathname> <CRLF>
            PWD  <CRLF>
            LIST [<SP> <pathname>] <CRLF>
            NLST [<SP> <pathname>] <CRLF>
            SITE <SP> <string> <CRLF>
            SYST <CRLF>
            STAT [<SP> <pathname>] <CRLF>
            HELP [<SP> <string>] <CRLF>
            NOOP <CRLF>

  The syntax of the above argument fields (using BNF notation
         where applicable) is:

            <username> ::= <string>
            <password> ::= <string>
            <account-information> ::= <string>
            <string> ::= <char> | <char><string>
            <char> ::= any of the 128 ASCII characters except <CR> and
            <LF>
            <marker> ::= <pr-string>
            <pr-string> ::= <pr-char> | <pr-char><pr-string>
            <pr-char> ::= printable characters, any
                          ASCII code 33 through 126
            <byte-size> ::= <number>
            <host-port> ::= <host-number>,<port-number>
            <host-number> ::= <number>,<number>,<number>,<number>
            <port-number> ::= <number>,<number>
            <number> ::= any decimal integer 1 through 255
            <form-code> ::= N | T | C
            <type-code> ::= A [<sp> <form-code>]
                          | E [<sp> <form-code>]
                          | I
                          | L <sp> <byte-size>
            <structure-code> ::= F | R | P
            <mode-code> ::= S | B | C
            <pathname> ::= <string>
            <decimal-integer> ::= any decimal integer

*/

/*
>>3.3 ssl FTP��չ
��RFC 2228�У�ftpЭ����չ������ָ��:
AUTH (Authentication/Security Mechanism),
ADAT (Authentication/Security Data),
PROT (Data Channel Protection Level),
PBSZ (Protection Buffer Size),
CCC (Clear Command Channel),
MIC (Integrity Protected Command),
CONF (Confidentiality Protected Command), and
ENC (Privacy Protected Command).

���к�SSL��չ��ص���Ҫָ�������¼���:
AUTH (Э����չ��֤): ָ����չ��֤����,SSL��TLS��
PBSZ (Э�̱���������): �ƶ�����������,SSL/TLSģʽ�б���Ϊ0��
PROT (�л���������): �л��������𣬿���Ϊ"C"�ޱ�������"P"��������
*/

/*
*****Reply Codes by Function Groups******
200 Command okay.
         500 Syntax error, command unrecognized.
             This may include errors such as command line too long.
         501 Syntax error in parameters or arguments.
         202 Command not implemented, superfluous at this site.
         502 Command not implemented.
         503 Bad sequence of commands.
         504 Command not implemented for that parameter.

110 Restart marker reply.
             In this case, the text is exact and not left to the
             particular implementation; it must read:
                  MARK yyyy = mmmm
             Where yyyy is User-process data stream marker, and mmmm
             server's equivalent marker (note the spaces between markers
             and "=").
         211 System status, or system help reply.
         212 Directory status.
         213 File status.
         214 Help message.
             On how to use the server or the meaning of a particular
             non-standard command.  This reply is useful only to the
             human user.
         215 NAME system type.
             Where NAME is an official system name from the list in the
             Assigned Numbers document.
          
         120 Service ready in nnn minutes.
         220 Service ready for new user.
         221 Service closing control connection.
             Logged out if appropriate.
         421 Service not available, closing control connection.
             This may be a reply to any command if the service knows it
             must shut down.
         125 Data connection already open; transfer starting.
         225 Data connection open; no transfer in progress.
         425 Can't open data connection.
         226 Closing data connection.
             Requested file action successful (for example, file
             transfer or file abort).
         426 Connection closed; transfer aborted.
         227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
          
         230 User logged in, proceed.
         530 Not logged in.
         331 User name okay, need password.
         332 Need account for login.
         532 Need account for storing files.

150 File status okay; about to open data connection.
         250 Requested file action okay, completed.
         257 "PATHNAME" created.
         350 Requested file action pending further information.
         450 Requested file action not taken.
             File unavailable (e.g., file busy).
         550 Requested action not taken.
             File unavailable (e.g., file not found, no access).
         451 Requested action aborted. Local error in processing.
         551 Requested action aborted. Page type unknown.
         452 Requested action not taken.
             Insufficient storage space in system.
         552 Requested file action aborted.
             Exceeded storage allocation (for current directory or
             dataset).
         553 Requested action not taken.
             File name not allowed.

*****Numeric  Order List of Reply Codes***
110 Restart marker reply.
 In this case, the text is exact and not left to the
 particular implementation; it must read:
      MARK yyyy = mmmm
 Where yyyy is User-process data stream marker, and mmmm
 server's equivalent marker (note the spaces between markers
 and "=").
120 Service ready in nnn minutes.
125 Data connection already open; transfer starting.
150 File status okay; about to open data connection.

200 Command okay.
202 Command not implemented, superfluous at this site.
211 System status, or system help reply.
212 Directory status.
213 File status.
214 Help message.
 On how to use the server or the meaning of a particular
 non-standard command.  This reply is useful only to the
 human user.
215 NAME system type.
 Where NAME is an official system name from the list in the
 Assigned Numbers document.
220 Service ready for new user.
221 Service closing control connection.
 Logged out if appropriate.
225 Data connection open; no transfer in progress.
226 Closing data connection.
 Requested file action successful (for example, file
 transfer or file abort).
227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
230 User logged in, proceed.
250 Requested file action okay, completed.
257 "PATHNAME" created.

331 User name okay, need password.
332 Need account for login.
350 Requested file action pending further information.

421 Service not available, closing control connection.
 This may be a reply to any command if the service knows it
 must shut down.
425 Can't open data connection.
426 Connection closed; transfer aborted.
450 Requested file action not taken.
 File unavailable (e.g., file busy).
451 Requested action aborted: local error in processing.
452 Requested action not taken.
 Insufficient storage space in system.

500 Syntax error, command unrecognized.
 This may include errors such as command line too long.
501 Syntax error in parameters or arguments.
502 Command not implemented.
503 Bad sequence of commands.
504 Command not implemented for that parameter.
530 Not logged in.
532 Need account for storing files.
550 Requested action not taken.
 File unavailable (e.g., file not found, no access).
551 Requested action aborted: page type unknown.
552 Requested file action aborted.
 Exceeded storage allocation (for current directory or
 dataset).
553 Requested action not taken.
 File name not allowed.

*/
