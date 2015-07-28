/*******************************************************************
   *	httprsp.h
   *    DESCRIPTION:HTTP ��Ӧ��������
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

#ifndef __YY_HTTP_RESPONSE_H__
#define __YY_HTTP_RESPONSE_H__

#include "httpdef.h"
#include "Buffer.h"
#include "socketTcp.h"

#include <map>

namespace net4cpp21
{
	typedef struct _TNew_Cookie
	{
		std::string cookie_name;
		std::string cookie_value;
		std::string cookie_expires;
		std::string cookie_path;
	}TNew_Cookie; //�����õ�cookie�ṹ
	class httpResponse 
	{
	public:
		httpResponse();
		~httpResponse(){}
		bool ifReceivedAll() const { return m_httprsp_bReceiveALL; }
		int get_respcode() const { return m_respcode; }
		DWORD get_httpVer() const { return m_httprsp_dwVer; }
		long lContentLength() const { return m_httprsp_lContentlen; }
		void lContentLength(long l) { m_httprsp_lContentlen=l; }
		MIMETYPE_ENUM get_contentType();
		const char * get_contentCharset();
		MIMETYPE_ENUM get_mimetype();
		void set_mimetype(MIMETYPE_ENUM mt);
		//�����Ѿ����յĲ���content���ݵ��ֽڳ��Ⱥ�bufferָ��
		long lReceivedContent() { return m_httprsp_data.len(); }
		const char *szReceivedContent() { return m_httprsp_data.str(); }
		
		TNew_Cookie *SetCookie(const char *cookiename)
		{
			std::map<std::string,TNew_Cookie>::iterator it=
				(cookiename)?m_httprsp_SETCOOKIE.find(cookiename):m_httprsp_SETCOOKIE.end();
			return (it!=m_httprsp_SETCOOKIE.end())?( &(*it).second ):NULL;
		}
		void SetCookie(const char *cookiename,const char *cookieval,const char *path);
		const char *Header(const char *pheader)
		{//��ȡָ����http����ͷ
			std::map<std::string,std::string>::iterator it=
				(pheader)?m_httprsp_HEADER.find(pheader):m_httprsp_HEADER.end();
			return (it!=m_httprsp_HEADER.end())?((*it).second.c_str()):NULL;
		}
		
		//---------------------------------------------------------------
		void init_httprsp();//��ʼ��http��Ӧ׼������/�����µ�http��Ӧ
		std::map<std::string,std::string> &Header() { return m_httprsp_HEADER; }
		std::map<std::string,TNew_Cookie> &Cookies() { return m_httprsp_SETCOOKIE; }
		//-------------------���봦��http��Ӧ----------------------------
		SOCKSRESULT recv_rspH(socketTCP *psock,time_t timeout=HTTP_MAX_RESPTIMEOUT);
		bool recv_remainderX(socketTCP *psock,long receiveBytes,time_t timeout);
		bool recv_remainder(socketTCP *psock,long receiveBytes=-1){
			//����ʣ��HTTP��Ӧδ�������Body����
			return recv_remainderX(psock,receiveBytes,HTTP_MAX_RESPTIMEOUT);
		}
		//����http��ӦΪָ�����ļ�(������http��Ӧͷ)
		//���ر����ļ��Ĵ�С��==0��������
		unsigned long save_resp(socketTCP *psock,const char *filename);
		//--------------------���뷢��http��Ӧ---------------------------
		void AddHeader(std::string &headName,std::string &headValue){ m_httprsp_HEADER[headName]=headValue;}
		//����catch����ͷ
		//"No-cache" - Do not cache this page at all, even if for use by the same client
		//"No-store" - The response and the request that created it must not be stored on any cache, 
		//				whether shared or private. The storage inferred here is non-volatile storage, 
		//				such as tape backups. This is not an infallible security measure.
		//"Private" , "Public"
		void CacheControl(const char *str){ if(str) m_httprsp_HEADER["Cache-control"]=std::string(str);}
		void NoCache(); //��ֹ����
		
		//����http��Ӧͷ
		SOCKSRESULT send_rspH(socketTCP *psock,int respcode,const char *respDesc);
		//�����ļ����ɹ�����SOCKSERR_OK
		SOCKSRESULT sendfile(socketTCP *psock,const char *filename,
			MIMETYPE_ENUM mt=MIMETYPE_UNKNOWED,long startPos=0,long endPos=-1);
		SOCKSRESULT sendfile(socketTCP *psock,const char *filename,
			MIMETYPE_ENUM mt,long* lpstartPos,long* lpendPos,int iRangeNums);

		static MIMETYPE_ENUM MimeType(const char *filename);
	private:
		//����http��Ӧͷ��������Ӧ��
		int ParseResponse(const char *httprspH);
		void parse_SetCookie(const char *strParam);
		void parseParam(char *strParam,char delm,
							 std::map<std::string,std::string> &maps);
		void encodeParam(cBuffer &buf,char delm,
							 std::map<std::string,std::string> &maps);
	private:
		int m_respcode;
		DWORD m_httprsp_dwVer; //httpЭ��汾
		long m_httprsp_lContentlen;
		std::map<std::string,std::string> m_httprsp_HEADER;
		//httprsp��Ӧ���յ������õ�cookie��Ϣ
		std::map<std::string,TNew_Cookie> m_httprsp_SETCOOKIE;
		bool m_httprsp_bReceiveALL;//�Ƿ��Ѿ�http������������
		cBuffer m_httprsp_data; //������յĲ���/ȫ����ӦBody����
	};

}//?namespace net4cpp21

#endif

/* yyc remove 2007-12-12
private:
	std::map<std::string,std::string> m_httprsp_COOKIE;
std::map<std::string,std::string> &Cookies() { return m_httprsp_COOKIE; }
const char *Cookies(const char *cookiename)
{
	std::map<std::string,std::string>::iterator it=
		(cookiename)?m_httprsp_COOKIE.find(cookiename):m_httprsp_COOKIE.end();
	return (it!=m_httprsp_COOKIE.end())?((*it).second.c_str()):NULL;
}

	if(!m_httprsp_COOKIE.empty()){
		std::map<std::string,std::string>::iterator it=m_httprsp_COOKIE.begin();
		for(;it!=m_httprsp_COOKIE.end();it++)
			outbuf.len()+=sprintf(outbuf.str()+outbuf.len(),"Set-Cookie: %s=%s; path=/\r\n",
			(*it).first.c_str(),(*it).second.c_str());
	} 
*/
