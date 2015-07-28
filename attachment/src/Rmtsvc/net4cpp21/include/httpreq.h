/*******************************************************************
   *	httpreq.h
   *    DESCRIPTION:HTTP �����������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2006-02-08
   *
   *	net4cpp 2.1
   *	
   *******************************************************************/

#ifndef __YY_HTTP_REQUEST_H__
#define __YY_HTTP_REQUEST_H__

#include "httpdef.h"
#include "Buffer.h"
#include "socketTcp.h"

#include <map>

namespace net4cpp21
{
	class httpRequest 
	{
	public:
		httpRequest();
		~httpRequest(){}
		HTTPREQ_TYPE get_reqType() const { return m_httpreq_iType; }
		DWORD get_httpVer() const { return m_httpreq_dwVer; }
		std::string& url() { return m_httpreq_strUrl; }
		bool ifReceivedAll() const { return m_httpreq_bReceiveALL; }
		bool bKeepAlive();
		time_t IfModifiedSince();
		void set_requestRange(long lstartpos,long lendpos);
		//���ܻ��ж����Χ,���ط�Χ����
		int get_requestRange(long *lpstartpos,long *lpendpos,int idx=0);
		void set_Authorization(const char *user,const char *pswd);
		HTTPAUTH_TYPE get_Authorization(std::string &user,std::string &pswd);
		//����POST��������
		void set_contentType(HTTPREQ_CONTENT_TYPE itype,const char *lpBoundary);
		HTTPREQ_CONTENT_TYPE get_contentType(std::string *strBoundary);
		const char * get_contentCharset();
		long get_contentLen() const { return m_httpreq_lContentlen; }
		//�����ύ����ָ�룬�����Է�Form����������
		cBuffer &get_contentData() { return m_httpreq_postdata; }
		void ifParseParams(bool b) { m_bParseParams=b; } //�����Ƿ�����ύ����������HTTP �������

		//��ȡhttp��������
		const char *Request(const char *reqname){
			if(reqname==NULL) return NULL;

			std::map<std::string,std::string>::iterator it=
				m_httpreq_params_GET.find(reqname);
			if(it!=m_httpreq_params_GET.end())
				return (*it).second.c_str();
			it=m_httpreq_params_POST.find(reqname);
			return (it!=m_httpreq_params_POST.end())?
				((*it).second.c_str()):NULL;
		}
		const char *Cookies(const char *cookiename)
		{
			std::map<std::string,std::string>::iterator it=
				(cookiename)?m_httpreq_COOKIE.find(cookiename):m_httpreq_COOKIE.end();
			return (it!=m_httpreq_COOKIE.end())?((*it).second.c_str()):NULL;
		}
		const char *Header(const char *pheader)
		{//��ȡָ����http����ͷ
			std::map<std::string,std::string>::iterator it=
				(pheader)?m_httpreq_HEADER.find(pheader):m_httpreq_HEADER.end();
			return (it!=m_httpreq_HEADER.end())?((*it).second.c_str()):NULL;
		}

		//---------------------------------------------------------------
		void init_httpreq(bool ifKeepHeader=false);//��ʼ��http����׼�������µ�http����
		std::map<std::string,std::string> &QueryString() { return m_httpreq_params_GET;}
		std::map<std::string,std::string> &Form() { return m_httpreq_params_POST;}
		std::map<std::string,std::string> &Cookies() { return m_httpreq_COOKIE; }
		std::map<std::string,std::string> &Header() { return m_httpreq_HEADER; }
	
		//����http���󲢷���,�ɹ�����SOCKSERR_OK
		SOCKSRESULT send_req(socketTCP *psock,const char *lpszurl);
		void SetPostData(const char *buf,long buflen) //����Ҫ���͵�POST����
		{
			m_httpreq_params_POST.clear(); //��ʱPOST Param��Ч
			m_httpreq_postdata.len()=0; //���ԭ�е�����
			m_httpreq_postdata.Resize(buflen+1);
			if(m_httpreq_postdata.str()==NULL) return; 
			::memcpy(m_httpreq_postdata.str(),buf,buflen);
			m_httpreq_postdata.len()=buflen;
		}
		const char * encodeReqestH(unsigned long &lret)
		{
			SOCKSRESULT sr=send_req(NULL,NULL);
			if(sr!=SOCKSERR_OK) return NULL;
			lret=m_httpreq_postdata.len();
			return m_httpreq_postdata.str();
		}
		//--------------------------------------------------------------

		//���ղ����봦��http����
		SOCKSRESULT recv_reqH(socketTCP *psock,time_t timeout=HTTP_MAX_RESPTIMEOUT);
		bool recv_remainder(socketTCP *psock,long receiveBytes=-1); //����ʣ��δ�����������
		//����Basic�ʺ����봮
		static bool ParseAuthorizationBasic(const char *str,
										  std::string &username,std::string &password);
	private:
		HTTPREQ_TYPE ParseRequest(const char *httpreqH);
		HTTPREQ_TYPE ParseFirstRequestLine(const char *lpszLine);
		void EncodeFirstRequestLine(cBuffer &buf);
		void parseURL(const char *lpszurl);
		void encodeURL(cBuffer &buf);
		void parseParam(char *strParam,char delm,
							 std::map<std::string,std::string> &maps,const char *ptrCharset);
		void encodeParam(cBuffer &buf,char delm,
							 std::map<std::string,std::string> &maps);

	private:
		DWORD m_httpreq_dwVer; //httpЭ��汾
		HTTPREQ_TYPE m_httpreq_iType; //http��������
		long m_httpreq_lContentlen;
		std::string m_httpreq_strUrl;
		
		//�����URL�ύ�Ĳ���
		std::map<std::string,std::string> m_httpreq_params_GET;
		//�����POST���ύ�Ĳ���
		std::map<std::string,std::string> m_httpreq_params_POST;
		std::map<std::string,std::string> m_httpreq_HEADER;
		std::map<std::string,std::string> m_httpreq_COOKIE;
		bool m_httpreq_bReceiveALL;//�Ƿ��Ѿ�http������������
		cBuffer m_httpreq_postdata; //������յĲ���post����

		bool m_bParseParams; //�Ƿ���в���������Ĭ��Ϊ��
							//��Ҫ�Ǹ��������ʹ�ã���������ڽ��մ���HTTPS����ʱ��������������ٶ�
	};

}//?namespace net4cpp21

#endif
