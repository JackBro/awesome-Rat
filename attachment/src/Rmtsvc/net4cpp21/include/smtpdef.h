/*******************************************************************
   *	smtpdef.h
   *    DESCRIPTION:����smtpЭ�����õ��ĳ������ṹ�Լ�enum�Ķ���
   *				
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *	
   *	net4cpp 2.1
   *******************************************************************/

#ifndef __YY_SMTPDEF_H__
#define __YY_SMTPDEF_H__

#define SMTP_SERVER_PORT	25 //Ĭ��smtp����Ķ˿�
#define SMTP_MAX_PACKAGE_SIZE 128 //���ͻ����smtp������С
#define SMTP_MAX_RESPTIMEOUT 10 //s ��Ӧ�����ʱ


#define SOCKSERR_SMTP_RESP -301 //������Ӧ����
#define SOCKSERR_SMTP_CONN SOCKSERR_SMTP_RESP-1 //���ӷ�����ʧ��
#define SOCKSERR_SMTP_AUTH SOCKSERR_SMTP_RESP-2 //smtp��֤ʧ��
#define SOCKSERR_SMTP_SURPPORT SOCKSERR_SMTP_RESP-3 //��֧�ֵ�smtp��֤��ʽ
#define SOCKSERR_SMTP_RECIPIENT SOCKSERR_SMTP_RESP-4 //û��ָ���ʼ�������
#define SOCKSERR_SMTP_FAILED SOCKSERR_SMTP_RESP-5 //һ���Դ���
#define SOCKSERR_SMTP_EMLFILE SOCKSERR_SMTP_RESP-6 //������ʼ���ʽ
#define SOCKSERR_SMTP_EMAIL SOCKSERR_SMTP_RESP-7 //��Ч���ʼ���ַ
#define SOCKSERR_SMTP_DNSMX SOCKSERR_SMTP_RESP-8 //MX��������ʧ��
#define SOCKSERR_SMTP_4XX   SOCKSERR_SMTP_RESP-9

typedef enum         //����֧�ֵ�SMTP��֤����
{
	SMTPAUTH_NONE=0,
	SMTPAUTH_LOGIN=1, //��Ч�� PLAIN��ֻΪ���� SMTP ��֤��Ԥ��׼ʵ������ݡ�ȱʡ����£��˻��ƽ����� SMTP ʹ��
	SMTPAUTH_CRAM_MD5=2, //һ��ѯ��/��Ӧ��֤���ƣ������� APOP����Ҳ�ʺ�������Э�����ʹ�á��� RFC 2195 ���Ѷ���
	SMTPAUTH_DIGEST_MD5=4, //RFC 2831 �ж����ѯ��/��Ӧ��֤����
	SMTPAUTH_8BITMIME=8,
	SMTPAUTH_PLAIN=16 //PLAIN �˻���ͨ�����紫���û��Ĵ��ı�����������Ϻ�������������
}SMTPAUTH_TYPE;


#include <fstream>
#include <vector>
#include <string>
class mailMessage
{
	int m_contentType; //"text/plain" "text/html"
	std::string m_strSubject;
	std::string m_strBody;
	std::string m_strBodyCharset;//�ʼ����ĵı����ַ�����Ĭ��Ϊutf-8
	std::vector<std::string> m_attachs;//Ҫ���͵ĸ���

	std::string m_strName;//�����ߵ�����
	std::string m_strFrom;//�����ߵ�����
	std::vector<std::pair<std::string,std::string> > m_vecTo; //����,first --�ռ���email��second �ռ����ǳ�
	std::vector<std::pair<std::string,std::string> > m_vecCc; //����,first --�ռ���email��second �ռ����ǳ�
	std::vector<std::pair<std::string,std::string> > m_vecBc; //����,first --�ռ���email��second �ռ����ǳ�
	
	std::string m_strMailFile;//���ɵ��ʼ����ļ�·������
	long m_lMailFileStartPos;//�ʼ����ļ����ʼ������ĵ���ʼλ��
	bool m_bDeleteFile;//�����ͷ�ʱ�Ƿ�ɾ��m_strMailFile�ļ�
public:
	enum RECIPIENT_TYPE { TO, CC, BCC };
	enum EMAILBODY_TYPE { TEXT_BODY,HTML_BODY };
	
	mailMessage():m_contentType(TEXT_BODY),m_strBodyCharset("utf-8"),m_bDeleteFile(false),m_lMailFileStartPos(0){}
	~mailMessage();
	const char *from() { return m_strFrom.c_str(); }
	std::string &body() { return m_strBody;}
	std::vector<std::pair<std::string,std::string> > & vecTo(RECIPIENT_TYPE rt=TO)
	{
		if(rt==CC)
			return m_vecCc;
		if(rt==BCC)
			return m_vecBc;
		return m_vecTo;
	}
	void setFrom(const char *from,const char *name)
	{
		if(from) m_strFrom.assign(from);
		if(name) m_strName.assign(name);
	}
	
	//�����ʼ����⣬����
	void setBody(const char *strSubject,const char *strBody,EMAILBODY_TYPE bt=TEXT_BODY)
	{
		if(strSubject) m_strSubject.assign(strSubject);
		if(strBody) m_strBody.assign(strBody);
		m_contentType=bt; m_strMailFile="";
	}
	//��Ӹ���
	bool AddAtach(const char *filename,const char *filepath,const char *contentID);
	//����ռ���
	bool AddRecipient(const char *email,const char *nick,RECIPIENT_TYPE rt=TO);
	//����Base64������ʼ����ļ�
	//bDelete -- ָʾ��mailMessage�����ͷ�ʱ�Ƿ�ɾ�����ɵ��ļ�
	const char * createMailFile(const char *file,bool bDelete);
	long MailFileStartPos() const { return m_lMailFileStartPos; }
	void setBody(const char *mailfile,long startPos)
	{
		m_strMailFile.assign(mailfile);
		m_bDeleteFile=false;
		m_lMailFileStartPos=startPos;
		return;
	}
	int initFromemlfile(const char *emlfile);
};

#endif

/* ����Э�����
[56 202.108.9.193:25-->127.0.0.1:1159] 220 Coremail SMTP(Anti Spam) System (163c
om[20050206])
.
[10 127.0.0.1:1159-->202.108.9.193:25] EHLO yyc
.
[96 202.108.9.193:25-->127.0.0.1:1159] 250-smtp14
250-PIPELINING
250-AUTH LOGIN PLAIN NTLM
250-AUTH=LOGIN PLAIN NTLM
250 8BITMIME
.
[12 127.0.0.1:1159-->202.108.9.193:25] AUTH LOGIN
.
[18 202.108.9.193:25-->127.0.0.1:1159] 334 VXNlcm5hbWU6
.
[10 127.0.0.1:1159-->202.108.9.193:25] eXljbmV0
.
[18 202.108.9.193:25-->127.0.0.1:1159] 334 UGFzc3dvcmQ6
.
[10 127.0.0.1:1159-->202.108.9.193:25] cYY6d8N6
.
[31 202.108.9.193:25-->127.0.0.1:1159] 235 Authentication successful
.
[29 127.0.0.1:1159-->202.108.9.193:25] MAIL FROM: <yycnet@163.com>
.
[8 202.108.9.193:25-->127.0.0.1:1159] 250 Ok
.
[33 127.0.0.1:1159-->202.108.9.193:25] RCPT TO: <yycmail@263.sina.com>
.
[8 202.108.9.193:25-->127.0.0.1:1159] 250 Ok
.
[6 127.0.0.1:1159-->202.108.9.193:25] Data
.
[37 202.108.9.193:25-->127.0.0.1:1159] 354 End data with <CR><LF>.<CR><LF>
.
[662 127.0.0.1:1159-->202.108.9.193:25] From: "yyc" <yycnet@163.com>
To: yycmail@263.sina.com <yycmail@263.sina.com>
Subject: hi,test
Organization: xxxx
X-mailer: Foxmail 4.2 [cn]
Mime-Version: 1.0
Content-Type: text/plain;
      charset="GB2312"
Content-Transfer-Encoding: quoted-printable
Date: Mon, 8 Aug 2005 10:47:32 +0800

yycmail=A3=AC=C4=FA=BA=C3=A3=A1

=09         aaaaaaaaaaaaaaaaaa

=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=D6=C2
=C0=F1=A3=A1
 =09=09=09=09

=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1yyc
=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1yycnet@163.com
=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A1=A12005-08-08


.
.
[38 202.108.9.193:25-->127.0.0.1:1159] 250 Ok: queued as 2cB_NXrE9kIrXhAE.1
.
[6 127.0.0.1:1159-->202.108.9.193:25] QUIT
.
[9 202.108.9.193:25-->127.0.0.1:1159] 221 Bye
.
*/
/*
211 ϵͳ״̬��ϵͳ������Ӧ 
������214 ������Ϣ 
������220 ������� 
������221 ����رմ����ŵ� 
������250 Ҫ����ʼ�������� 
������251 �û��Ǳ��أ���ת���� 
������354 ��ʼ�ʼ����룬��.���� 
������421 ����δ�������رմ����ŵ���������ر�ʱ����Ӧ�������Ϊ���κ��������Ӧ�� 
������450 Ҫ����ʼ�����δ��ɣ����䲻���ã����磬����æ�� 
������451 ����Ҫ��Ĳ�������������г��� 
������452 ϵͳ�洢���㣬Ҫ��Ĳ���δִ�� 
������500 ��ʽ���������ʶ�𣨴˴���Ҳ���������й����� 
������501 ������ʽ���� 
������502 �����ʵ�� 
������503 ������������� 
������504 �����������ʵ�� 
������550 Ҫ����ʼ�����δ��ɣ����䲻���ã����磬����δ�ҵ����򲻿ɷ��ʣ� 
������551 �û��Ǳ��أ��볢�� 
������552 �����Ĵ洢���䣬Ҫ��Ĳ���δִ�� 
������553 �����������ã�Ҫ��Ĳ���δִ�У����������ʽ���� 
������554 ����ʧ�� 
*/

