/*******************************************************************
   *	socketTcp.h
   *    DESCRIPTION:TCP socket ��Ķ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	Last modify: 2005-09-02
   *	net4cpp 2.1
   *******************************************************************/
#ifndef __YY_SOCKET_TCP_H__
#define __YY_SOCKET_TCP_H__

#ifndef _NOSSL_D
#define _SURPPORT_OPENSSL_ //���Ҫ֧��SSL���붨��˺�
						//ͬʱ�ڱ���ѡ���м���opensslͷ�ļ��Ϳ��ļ�·��
						//<net4cpp2.1Ŀ¼>/OPENSSL
						//<net4cpp2.1Ŀ¼>/OPENSSL/lib
						//tools�˵�-->Options�Ӳ˵�--->directoriesҳ
#endif
#ifdef _SURPPORT_OPENSSL_
	#include <openssl/crypto.h>
	#include <openssl/x509.h>
	#include <openssl/pem.h>
	#include <openssl/ssl.h>
	#include <openssl/err.h>
    #pragma comment( lib, "libeay32lib" )
	#pragma comment( lib, "SSLeay32lib" )
#endif
#include "socketBase.h"

namespace net4cpp21
{
	class socketTcp : public socketBase
	{
	public:
		socketTcp(){}
		socketTcp(socketTcp &sockTcp):socketBase(sockTcp){ return; }
		socketTcp & operator = (socketTcp &sockTcp) { socketBase::operator = (sockTcp); return *this; }
		virtual ~socketTcp(){}

		//����ָ����TCP���񡣳ɹ����ر���socket�˿ڣ����򷵻ش���
		virtual SOCKSRESULT Connect(const char *host,int port,time_t lWaitout=-1)
		{
			SOCKSRESULT iret=(host)?setRemoteInfo(host,port):SOCKSERR_OK;
			if(iret==SOCKSERR_OK) iret=Connect(lWaitout,0,NULL);
			return iret;
		}
		virtual SOCKSRESULT Accept(time_t lWaitout,socketTcp *psock);
		//TCP������������������˿�
		SOCKSRESULT ListenX(int port,BOOL bReuseAddr,const char *bindIP);
		SOCKSRESULT ListenX(int startport,int endport,BOOL bReuseAddr,const char *bindIP);
		SOCKSRESULT Connect(const char *host,int port,int bindport,const char *bindip)
		{
			SOCKSRESULT iret=(host)?setRemoteInfo(host,port):SOCKSERR_OK;
			if(iret==SOCKSERR_OK) iret=Connect(-1,bindport,bindip);
			return iret;
		}
	protected:
		//�ɹ����ر���socket�˿ڣ����򷵻ش���
//		SOCKSRESULT Connect(time_t lWaitout); //yyc remove 2007-08-07
		SOCKSRESULT Connect(time_t lWaitout,int bindport,const char *bindip); //yyc add 2007-08-07
	};

#ifdef _SURPPORT_OPENSSL_
	class socketSSL : public socketTcp
	{
	public:
		socketSSL();
		socketSSL(socketSSL &sockSSL);
		socketSSL & operator = (socketSSL &sockSSL);
		virtual ~socketSSL();

		virtual void Close();
		bool ifSSL() const { return m_ctx!=NULL; }
		bool ifSSLVerify() const { return m_bSSLverify; } //SSL�����Ƿ���Ҫ�ͻ�����֤
		//����SSL��֤��˽Կ����
		//bNotfile -- ָʾstrCaCert&strCaKeyָ�����֤���ļ�������֤������
		//���bNotfile=true��strCaCert��strCaKeyΪ������Ĭ�ϵ�֤���˽Կ
		void setCacert(const char *strCaCert,const char *strCaKey,const char *strCaKeypwd,bool bNotfile,
					   const char *strCaRootFile=NULL,const char *strCRLfile=NULL);
		void setCacert(socketSSL *psock,bool bOnlyCopyCert);
		//����SSLЭ��,�����ӻ��߽���һ�����Ӻ�,�ɹ�������
		bool SSL_Associate();
		//��ʼ��SSL,bInitServer:ָ���ǳ�ʼ������˻��ǿͻ���
		//���psock!=NULL����psock��֤������ʼ��SSL�����
		bool initSSL(bool bInitServer,socketSSL *psock=NULL);
		
		SOCKSRESULT Accept(time_t lWaitout,socketSSL *psock)
		{
			SOCKSRESULT sr=socketTcp::Accept(lWaitout,psock);
			if(sr>0 && psock){
				psock->m_ssltype=SSL_INIT_NONE;
				psock->m_ctx=this->m_ctx;
			}
			return sr;
		}

	protected:
		virtual size_t v_read(char *buf,size_t buflen);
		//!!! SSL_peek�鿴SSL���ݺ��ı�socket�Ŀɶ���־����ʱ���ͨ��
		//select �ж�socket���������Զ���ز��ɶ�
		virtual size_t v_peek(char *buf,size_t buflen);
		virtual size_t v_write(const char *buf,size_t buflen);
		void freeSSL();
	private:
		SSL_INIT_TYPE m_ssltype;//SSL��ʼ������
		SSL_CTX *m_ctx;
		SSL *    m_ssl;
		//SSL�����֤�飬˽Կ�Լ�˽Կ����
		std::string m_cacert;//SSL ֤��
		std::string m_cakey;//SSL ˽Կ
		std::string m_cakeypass;//SSL˽Կ����
		bool m_bNotfile;//ָ��m_cacert&m_cakeyָ�����֤���ļ�������֤���ַ���
		bool m_bSSLverify; //SSL�Ƿ�Ҫ��֤�ͻ���֤��
		std::string m_carootfile; //CA��֤��֤�飬����У��ͻ���֤�����α
		std::string m_crlfile; //CRL�б��ļ�
	};
	
	typedef socketSSL socketTCP;
#else
	typedef socketTcp socketTCP;
#endif
}//?namespace net4cpp21

#endif

