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


#include "../include/sysconfig.h"
#include "../include/httpreq.h"
#include "../include/cCoder.h"
#include "../utils/cTime.h"
#include "../utils/utils.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;

httpRequest::httpRequest()
{
	m_httpreq_iType=HTTP_REQ_UNKNOWN;
	m_httpreq_dwVer=MAKELONG(1,1);
	m_httpreq_bReceiveALL=true;
	m_httpreq_lContentlen=0;
	m_bParseParams=true;
}

void httpRequest::init_httpreq(bool ifKeepHeader)
{
	m_httpreq_iType=HTTP_REQ_UNKNOWN;
	m_httpreq_strUrl="";
	m_httpreq_dwVer=MAKELONG(1,1);
	m_httpreq_params_GET.clear();
	m_httpreq_params_POST.clear();
	if(!ifKeepHeader) m_httpreq_HEADER.clear();
	m_httpreq_COOKIE.clear();
	m_httpreq_bReceiveALL=true;
	m_httpreq_postdata.Resize(0);
	m_httpreq_lContentlen=0;
}

bool httpRequest::bKeepAlive() 
{
	std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Connection");
	if(it!=m_httpreq_HEADER.end())
		if(strcmp((*it).second.c_str(),"Keep-Alive")==0)	return true;
	return false;
}

time_t httpRequest::IfModifiedSince()
{
	std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("If-Modified-Since");
	if(it!=m_httpreq_HEADER.end())
	{
		cTime t; const char *pvalue=(*it).second.c_str();
		if(t.parseDate(pvalue)) return t.Gettime();
	}
	return 0;
}

void httpRequest:: set_contentType(HTTPREQ_CONTENT_TYPE itype,const char *lpBoundary)
{
	if(itype==HTTP_CONTENT_TEXTXML)
		m_httpreq_HEADER["Content-Type"]="text/xml";
	else if(itype==HTTP_CONTENT_MULTIPART)
	{
		char buf[128];
		sprintf(buf,"multipart/form-data; boundary=%s",lpBoundary);
		m_httpreq_HEADER["Content-Type"]=buf;
	}
	else
		m_httpreq_HEADER["Content-Type"]="application/x-www-form-urlencoded";
	return;
}

//Content-Type: text/xml; charset=utf-8
//Content-Type: application/x-www-form-urlencoded   //form �ύ����
//Content-Type: multipart/form-data; boundary=---------------------------7d4f19130094
HTTPREQ_CONTENT_TYPE httpRequest:: get_contentType(std::string *strBoundary)
{
	std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Content-Type");
	if(it!=m_httpreq_HEADER.end())
	{	
		const char *ptr,*pvalue=(*it).second.c_str();
		if(strncasecmp(pvalue,"multipart/",10)==0)
		{
			if(strBoundary){
				if( (ptr=strstr(pvalue+10,"boundary=")) )
					strBoundary->assign(ptr+9);
				else *strBoundary="";
			}
			return HTTP_CONTENT_MULTIPART;
		}else if(strncasecmp(pvalue,"text/xml",8)==0)
			return HTTP_CONTENT_TEXTXML;
		else if(strncasecmp(pvalue,"application/x-www-form-u",24)==0)
			return HTTP_CONTENT_APPLICATION;
		else return HTTP_CONTENT_UNKNOWED;
	}
	return HTTP_CONTENT_APPLICATION;
}

//Content-Type: text/xml; charset=utf-8
//Content-Type: application/x-www-form-urlencoded; charset=utf-8   //form �ύ����
//�����ύcontent�ı��뷽ʽ
const char *httpRequest:: get_contentCharset()
{
	std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Content-Type");
	if(it==m_httpreq_HEADER.end()) return NULL;

	const char *ptr,*pvalue=(*it).second.c_str();
	if( (ptr=strchr(pvalue,';'))==NULL ) return NULL;
	ptr++; while(*ptr==' ') ptr++; //ȥ��ǰ���ո�
	if(strncasecmp(ptr,"charset=",8)!=0) return NULL;
	ptr+=8;while(*ptr==' ') ptr++; //ȥ��ǰ���ո�
	return ptr;	
}

void httpRequest::set_requestRange(long lstartpos,long lendpos)
{
	if(lstartpos>0 || lendpos!=-1)
	{//�������������Χ
		char buf[64];
		(lendpos!=-1)?sprintf(buf,"bytes=%d-%d",lstartpos,lendpos):
					  sprintf(buf,"bytes=%d-",lstartpos);
		m_httpreq_HEADER["Range"]=buf;
	}
	return;
}
//Range: bytes=650833-651856, 643665-650832, 32768-643664
//���ܻ��ж����Χ,���ط�Χ����
int httpRequest::get_requestRange(long *lpstartpos,long *lpendpos,int idx)
{
	std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Range");
	if(it==m_httpreq_HEADER.end()) return 0;
	const char *p,*ptr,*pvalue=(*it).second.c_str();
	if(strncmp(pvalue,"bytes=",6)!=0) return 0;
	pvalue+=6; while(*pvalue==' ') pvalue++;//ȥ��ǰ���ո�
	int iRangeNums=0;
	while(true)
	{
		ptr=strchr(pvalue,',');
		if(ptr) *(char *)ptr=0;
		
		if(idx==iRangeNums++)
		{
			if( (p=strchr(pvalue,'-')) )
			{
				if(lpstartpos) *lpstartpos=atol(pvalue);
				if(lpendpos) if( (*lpendpos=atol(p+1))<=0) *lpendpos=-1;
			}else{
				if(lpstartpos) *lpstartpos=atol(pvalue);
				if(lpendpos) *lpendpos=-1;
			}
		}//?if(idx==iRangeNums++)

		if(ptr) *(char *)ptr=','; else break;
		pvalue=ptr+1; while(*pvalue==' ') pvalue++;//ȥ��ǰ���ո�
	}
	return iRangeNums;
}

HTTPAUTH_TYPE httpRequest::get_Authorization(std::string &user,std::string &pswd)
{
	std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Authorization");
	if(it!=m_httpreq_HEADER.end())
	{
		const char *pvalue=(*it).second.c_str();
		if( ParseAuthorizationBasic(pvalue,user,pswd) ) return HTTP_AUTHORIZATION_PLAINTEXT;
	}
	return HTTP_AUTHORIZATION_ANONYMOUS;
}
void httpRequest::set_Authorization(const char *user,const char *pswd)
{//�����http������������ʽ����web�������ڴ����÷��ʵ��ʺŶ�����
	if(user==NULL || pswd==NULL) return;
	std::string s1,s("Basic ");
	s.append(user); s.append(":"); s.append(pswd);
	int l=cCoder::Base64EncodeSize(s.length())+1;
	s1.resize(l); //����ռ�
	l=cCoder::base64_encode((char *)s.c_str(),s.length(),(char *)s1.c_str());
	s1[l]=0; m_httpreq_HEADER["Authorization"]=s1.c_str();
	return;
}
//����http����,�ɹ�����SOCKSERR_OK
SOCKSRESULT httpRequest::send_req(socketTCP *psock,const char *lpszurl)
{
//	ASSERT(psock);
	if(lpszurl) parseURL(lpszurl); //�Ƚ����������url���룬�ֱ𱣴���param_GET��url��
	//������POST����Ӧ�ȱ���POST����
	if(m_httpreq_params_POST.size()>0) 
		encodeParam(m_httpreq_postdata,'&',m_httpreq_params_POST);
	
//	if(m_httpreq_iType==HTTP_REQ_UNKNOWN) //yyc remove 2009-12-15
//		m_httpreq_iType=(m_httpreq_postdata.len()>0)?HTTP_REQ_POST:HTTP_REQ_GET;
	if(m_httpreq_iType==HTTP_REQ_UNKNOWN){ //yyc add 2009-12-15
		std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.end();
		if(m_httpreq_postdata.len()>0) 
			m_httpreq_iType=HTTP_REQ_POST;
		else{
			std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Content-Length");
			string s=(it!=m_httpreq_HEADER.end())?((*it).second):"";
			if(s!="" && atol(s.c_str())>0) m_httpreq_iType=HTTP_REQ_POST;
		}
		if(m_httpreq_iType==HTTP_REQ_UNKNOWN) m_httpreq_iType=HTTP_REQ_GET;
	}//?if(m_httpreq_iType==HTTP_REQ_UNKNOWN)

	//����http����ͷ
	int l; cBuffer contentBuffer(1024); //http���󻺳�
	EncodeFirstRequestLine(contentBuffer);
	
	if(m_bParseParams){//yyc add 2007-12-10 �Դ���ת�������ж�
		std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.find("Content-Length");
		if( it==m_httpreq_HEADER.end() && //�Ƿ���Content����
			(m_httpreq_iType==HTTP_REQ_POST || m_httpreq_postdata.len()>0) ) 
		{
			l=sprintf(contentBuffer.str()+contentBuffer.len(),"%d",m_httpreq_postdata.len());
			m_httpreq_HEADER["Content-Length"]=contentBuffer.str()+contentBuffer.len();
			if(m_httpreq_HEADER.count("Content-Type")<1) 
				m_httpreq_HEADER["Content-Type"]="application/x-www-form-urlencoded";
		}
//		if( (it=m_httpreq_HEADER.find("Accept"))==m_httpreq_HEADER.end()) //yyc modify 2007-12-10
		if( (it=m_httpreq_HEADER.find("User-Agent"))==m_httpreq_HEADER.end())
		{
			l=sprintf(contentBuffer.str()+contentBuffer.len(),
				"Accept: */*\r\nAccept-Language: zh-cn\r\n"
				"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\n");
			contentBuffer.len()+=l;
		}
	}//?if(m_bParseParams) //yyc add 2007-12-10
	
	if(m_httpreq_HEADER.size()>0)
	{
		std::map<std::string,std::string>::iterator it=m_httpreq_HEADER.begin();
		for(;it!=m_httpreq_HEADER.end();it++)
		{
			l=(*it).first.length()+(*it).second.length()+4;
			if(l<128) l=128; //���Ǳ���һ���Ŀռ����Ƶ�����䲢�ƶ�
			if(contentBuffer.Space()<l)
				if(contentBuffer.Resize(contentBuffer.size()+l)==NULL) break;
			l=sprintf(contentBuffer.str()+contentBuffer.len(),"%s: %s\r\n",
				(*it).first.c_str(),(*it).second.c_str());
			contentBuffer.len()+=l;
		}
	}//?if(m_httpreq_HEADER.size()>0)

	if(m_httpreq_COOKIE.size()>0)
	{
		l=sprintf(contentBuffer.str()+contentBuffer.len(),"Cookie: ");
		contentBuffer.len()+=l;
		encodeParam(contentBuffer,';',m_httpreq_COOKIE);
		l=sprintf(contentBuffer.str()+contentBuffer.len(),"\r\n");
		contentBuffer.len()+=l;
	}
	 
	if(contentBuffer.Space()<=2) 
		if(contentBuffer.Resize(contentBuffer.size()+2)==NULL) return SOCKSERR_MEMORY;
	l=sprintf(contentBuffer.str()+contentBuffer.len(),"\r\n");
	contentBuffer.len()+=l;
	
	SOCKSRESULT sr=SOCKSERR_OK; //��ӡ���͵�http����ͷ
	RW_LOG_DEBUG("[httpreq] Sending HTTP Request Header,len=%d\r\n%s",contentBuffer.len(),contentBuffer.str());
	if(psock==NULL) //������ȡҪ���͵����ݣ���������
	{
		if(contentBuffer.Space()<=m_httpreq_postdata.len())
			if(contentBuffer.Resize(contentBuffer.size()+m_httpreq_postdata.len())==NULL) return SOCKSERR_MEMORY;
		memcpy(contentBuffer.str()+contentBuffer.len(),m_httpreq_postdata.str(),m_httpreq_postdata.len());
		contentBuffer.len()+=m_httpreq_postdata.len();
	}
	else if( (sr=psock->Send(contentBuffer.len(),contentBuffer.str(),-1)) >0 )
	{
		if(m_httpreq_postdata.len()>0){
//			RW_LOG_DEBUG("[httpreq] Sending HTTP Request POST Data, len=%d\r\n%s.\r\n",
//				m_httpreq_postdata.len(),m_httpreq_postdata.str());
			sr=psock->Send(m_httpreq_postdata.len(),m_httpreq_postdata.str(),-1);
		}
	}
	m_httpreq_postdata=contentBuffer; //��ʱm_httpreq_postdata�����˷��͵�http����ͷ
	return (sr<0)?sr:SOCKSERR_OK;
}

//����http����ͷ
//�ɹ�������������HTTPREQ_TYPE
SOCKSRESULT httpRequest::recv_reqH(socketTCP *psock,time_t timeout)
{
//	ASSERT(psock);
	const char * pFoundTerminator=NULL; //http����ͷ�Ƿ�������
	cBuffer buffer(1024); char *pbuf;
	while(	psock->status()==SOCKS_CONNECTED ) 
	{
		pbuf=buffer.str()+buffer.len();
		//�������timeout��û�յ����ݿ���Ϊ�ͻ����쳣
		int iret=psock->Receive(pbuf,buffer.Space()-1,timeout);
		if(iret<0) break;//==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
		*(pbuf+iret)=0; buffer.len()+=iret;
		if( (pFoundTerminator=strstr(buffer.str(),"\r\n\r\n")) ) break;
		if(buffer.Space()<256) //Ԥ����һ���Ŀռ䣬�Ա�������������ͷ
			if( buffer.Resize(buffer.size()+256)==NULL ) break;						
	}//?while
	if(pFoundTerminator==NULL) return SOCKSERR_HTTP_RESP; //δ���յ�������http����ͷ
	//��ӡ���յ�http����ͷ
	*(char *)pFoundTerminator=0;
	RW_LOG_DEBUG("[httpreq] Received HTTP Request Header\r\n%s\r\n",buffer.str());
	*(char *)pFoundTerminator='\r';

	init_httpreq(); //��ʼ��http �������ı���
	if(ParseRequest(buffer.str())==HTTP_REQ_UNKNOWN) return HTTP_REQ_UNKNOWN;
	if(m_httpreq_lContentlen>0)
	{//�ж�post�����Ƿ�������
		pbuf=(char *)pFoundTerminator+4; //post������ʼ��ַ
		long receivedBytes=buffer.len()-(pbuf-buffer.str());
		if(receivedBytes>=m_httpreq_lContentlen)
		{//post�����Ѿ��������
			m_httpreq_bReceiveALL=true; *(pbuf+m_httpreq_lContentlen)=0;
			if(m_bParseParams && get_contentType(NULL)==HTTP_CONTENT_APPLICATION)
			{//����form�ύ����
				const char *ptrCharset=get_contentCharset();
				parseParam(pbuf,'&',m_httpreq_params_POST,ptrCharset);
			}
			else //������������form���ύ�ı��������򱣴浽m_httpreq_postdata
			{
				m_httpreq_postdata.Resize(receivedBytes+1);
				::memcpy(m_httpreq_postdata.str(),pbuf,receivedBytes);
				m_httpreq_postdata.len()=m_httpreq_lContentlen;
				m_httpreq_postdata[m_httpreq_lContentlen]=0;
			}
		}//?if(receivedBytes>=m_httpreq_lContentlen)
		else //û�н�����ϣ�����������ʱ��ŵ�m_httpreq_postdata��
		{
			m_httpreq_bReceiveALL=false;
			if(receivedBytes>0) ::memmove(buffer.str(),pbuf,receivedBytes);
			buffer.len()=receivedBytes;
			m_httpreq_postdata=buffer;
		}
	}//?if(m_httpreq_lContentlen>0)
	else m_httpreq_bReceiveALL=true;
	return m_httpreq_iType;
}
//����ʣ���δ�������POST����
//receiveBytes - ����ָ�����ֽڳ���  <=0:��������δ������
//�ɹ�������,���򷵻ؼ�
bool httpRequest::recv_remainder(socketTCP *psock,long receiveBytes)
{
//	ASSERT(psock);
//	RW_LOG_DEBUG("[httpRequest] -stat=%d bReceiveALL=%d ContentLen=%d,received=%d\r\n",
//		psock->status(),m_httpreq_bReceiveALL,m_httpreq_lContentlen,m_httpreq_postdata.len());
	long l,remainBytes=receiveBytes;
	while(	!m_httpreq_bReceiveALL )
	{
		if(m_httpreq_lContentlen>m_httpreq_postdata.len())
		{
			if(m_httpreq_postdata.Space()<1024) //Ԥ����һ���Ŀռ�
				if( m_httpreq_postdata.Resize(m_httpreq_postdata.size()+1024)==NULL ) 
					break;

			char *pbuf=m_httpreq_postdata.str()+m_httpreq_postdata.len();
			l=m_httpreq_postdata.Space()-1;
			if(receiveBytes>0 && l>remainBytes) l=remainBytes;
			//�������HTTP_MAX_RESPTIMEOUT��û�յ����ݿ���Ϊ�ͻ����쳣
//			RW_LOG_DEBUG("[httpRequest] - Receive %d Bytes...\r\n",l);
			l=psock->Receive(pbuf,l,HTTP_MAX_RESPTIMEOUT);
//			RW_LOG_DEBUG("[httpRequest] - Receive Data return %d\r\n",l);
			if(l<0) break;
			if(l==0){ cUtils::usleep(SCHECKTIMEOUT); continue; }//==0������������������������
			m_httpreq_postdata.len()+=l; 
			m_httpreq_postdata[m_httpreq_postdata.len()]=0;
			if( receiveBytes>0) if( (remainBytes-=l)<=0 ) break; //������ָ��������
		} else m_httpreq_bReceiveALL=true;
	}//?while
//	RW_LOG_DEBUG("[httpRequestX] -stat=%d bReceiveALL=%d ContentLen=%d,received=%d\r\n",
//		psock->status(),m_httpreq_bReceiveALL,m_httpreq_lContentlen,m_httpreq_postdata.len());
	m_httpreq_postdata[m_httpreq_postdata.len()]=0;
	if( m_bParseParams && m_httpreq_bReceiveALL && 
		get_contentType(NULL)==HTTP_CONTENT_APPLICATION)
	{//����form�ύ����
		const char *ptrCharset=get_contentCharset();
		parseParam(m_httpreq_postdata.str(),'&',m_httpreq_params_POST,ptrCharset);
		m_httpreq_postdata.Resize(0);//�ͷſռ�
	}
	
	if( receiveBytes>0) if(remainBytes<=0) return true;
	return m_httpreq_bReceiveALL;
}
//����http����ͷ,����http��������
HTTPREQ_TYPE httpRequest::ParseRequest(const char *httpreqH)
{
	//����ÿһ������
	bool bFirstLine = true;
	const char *ptrLineStart=httpreqH;
	const char *ptrLineEnd=strchr(ptrLineStart,'\n');
	while(true)
	{
		if(ptrLineEnd){
			*(char *)ptrLineEnd='\0';
			if(*(ptrLineEnd-1)=='\r') *(char*)(ptrLineEnd-1)='\0';
		}

		if(bFirstLine)
		{//�����һ������
			if( ParseFirstRequestLine(ptrLineStart)==HTTP_REQ_UNKNOWN)
				return HTTP_REQ_UNKNOWN;
			bFirstLine=false;
		}
		else
		{//��������������
			const char *pvalue,*pFoundTerminator=strchr(ptrLineStart,':');
			if(pFoundTerminator)
			{
				*(char *)pFoundTerminator='\0';
				pvalue=pFoundTerminator+1;
				while(*pvalue==' ') pvalue++;//ɾ��ǰ���ո�
				
				if(strcmp(ptrLineStart,"Content-Length")==0)
				{
					if(!m_bParseParams) //�����������...
						m_httpreq_HEADER[ptrLineStart]=string(pvalue);
					m_httpreq_lContentlen=atol(pvalue);
					//��Ϊ�������POST������û��content�����Ժ�����ݽ����������ж�
					if(m_httpreq_iType!=HTTP_REQ_POST) m_httpreq_lContentlen=0;
				}
				else if(m_bParseParams && strcmp(ptrLineStart,"Cookie")==0)
				{//cookie���ݸ�ʽ��Cookie: name=value; name=value...
					std::string strtmp(pvalue);
					parseParam((char *)strtmp.c_str(),';',m_httpreq_COOKIE,NULL);
				}
				else 
					m_httpreq_HEADER[ptrLineStart]=string(pvalue);

				*(char *)pFoundTerminator=':';
			}//?if(pFoundTerminator)
		}//?if(bFirstLine)...else

		if(ptrLineEnd==NULL) 
			break;
		else
		{
			*(char *)ptrLineEnd='\n';
			if(*(ptrLineEnd-1)=='\0') *(char*)(ptrLineEnd-1)='\r';
			//����������http����ͷ����
			if(ptrLineStart[0]=='\r' || ptrLineStart[0]=='\n') break; 
		}
		ptrLineStart=ptrLineEnd+1;
		ptrLineEnd=strchr(ptrLineStart,'\n');
	}//?while
	return m_httpreq_iType;
}
//����http�����һ�е����ݣ�����http��������
HTTPREQ_TYPE httpRequest::ParseFirstRequestLine(const char *lpszLine)
{
//	ASSERT(lpszLine);
	const char *ptrStart=lpszLine;
	if(strncasecmp(ptrStart,"GET ",4)==0)
	{
		m_httpreq_iType=HTTP_REQ_GET;
		ptrStart+=4;
	}
	else if(strncasecmp(ptrStart,"POST ",5)==0)
	{
		m_httpreq_iType=HTTP_REQ_POST;
		ptrStart+=5;
	}
	else if(strncasecmp(ptrStart,"HEAD ",5)==0)
	{
		m_httpreq_iType=HTTP_REQ_HEAD;
		ptrStart+=5;
	}
	else if(strncasecmp(ptrStart,"PUT ",4)==0)
	{
		m_httpreq_iType=HTTP_REQ_PUT;
		ptrStart+=4;
	}
	else if(strncasecmp(ptrStart,"LINK ",5)==0)
	{
		m_httpreq_iType=HTTP_REQ_LINK;
		ptrStart+=5;
	}
	else if(strncasecmp(ptrStart,"DELETE ",7)==0)
	{
		m_httpreq_iType=HTTP_REQ_DELETE;
		ptrStart+=7;
	}
	else if(strncasecmp(ptrStart,"UNLINK ",7)==0)
	{
		m_httpreq_iType=HTTP_REQ_UNLINK;
		ptrStart+=7;
	}
	else if(strncasecmp(ptrStart,"CONNECT ",8)==0)
	{
		m_httpreq_iType=HTTP_REQ_CONNECT;
		ptrStart+=8;
	}
	else
	{
		m_httpreq_iType=HTTP_REQ_UNKNOWN; 
		return m_httpreq_iType;
	}

	while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	//��ȡ����URL
	const char *pFoundTerminator=strchr(ptrStart,' ');
	if(pFoundTerminator){
		m_httpreq_strUrl.assign(ptrStart,pFoundTerminator-ptrStart);
		ptrStart=pFoundTerminator+1;
	}
	else{ m_httpreq_strUrl.assign(ptrStart); ptrStart=NULL; }
	//����http URL -----------------start---------------------------
	if(m_httpreq_iType==HTTP_REQ_CONNECT)
	{
		m_httpreq_HEADER["PHost"]=m_httpreq_strUrl;
		m_httpreq_strUrl="";
	}else{ //�����HTTP�������͹�����URL�д���������Ϣ����������URL��ַhttp://xxxx
		char c=0;
		if(m_httpreq_strUrl.length()>10) { c=m_httpreq_strUrl[10]; m_httpreq_strUrl[10]=0;}
		pFoundTerminator=strstr(m_httpreq_strUrl.c_str(),"://");
		if(c!=0) m_httpreq_strUrl[10]=c;
		if(pFoundTerminator) //��������·����Ӧȥ������ͷ��Ϣ
		{//"PHost"ͷ���Զ���ģ���Ҫ��Ϊ�˸���������ȡ������������Ϣ��
			if( (pFoundTerminator=strchr(pFoundTerminator+3,'/')) )
			{
				m_httpreq_HEADER["PHost"]=string(m_httpreq_strUrl.c_str(),pFoundTerminator-m_httpreq_strUrl.c_str());
				m_httpreq_strUrl.erase(0,pFoundTerminator-m_httpreq_strUrl.c_str());
			}else{
				m_httpreq_HEADER["PHost"]=m_httpreq_strUrl;
				m_httpreq_strUrl="/";
			}
		}//?if(pFoundTerminator)
	}
	//����http URL ----------------- end ---------------------------
	if(m_bParseParams) parseURL(NULL);

	//��ȡhttpЭ��汾
	//No version included in the request, so set it to HTTP v0.9
	m_httpreq_dwVer=MAKELONG(9, 0);
	if(ptrStart==NULL) return m_httpreq_iType;
	m_httpreq_dwVer=0; //����ض�����Э��汾
	while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	if(strncasecmp(ptrStart,"HTTP/",5)==0)
	{
		ptrStart+=5;
		if( (pFoundTerminator=strchr(ptrStart,'.')) )
		{
			WORD wMinorVersion=(WORD)atoi(pFoundTerminator+1);
			WORD wMajorVersion=(WORD)atoi(ptrStart);
			m_httpreq_dwVer=MAKELONG(wMinorVersion, wMajorVersion);
		}
	}
	return m_httpreq_iType;
}

void httpRequest::EncodeFirstRequestLine(cBuffer &buf)
{
	if(m_httpreq_iType==HTTP_REQ_GET)
	{
		strcpy(buf.str()+buf.len(),"GET ");
		buf.len()+=4;
	}
	else if(m_httpreq_iType==HTTP_REQ_POST)
	{
		strcpy(buf.str()+buf.len(),"POST ");
		buf.len()+=5;
	}
	else if(m_httpreq_iType==HTTP_REQ_HEAD)
	{
		strcpy(buf.str()+buf.len(),"HEAD ");
		buf.len()+=5;
	}
	else if(m_httpreq_iType==HTTP_REQ_PUT)
	{
		strcpy(buf.str()+buf.len(),"PUT ");
		buf.len()+=4;
	}
	else if(m_httpreq_iType==HTTP_REQ_LINK)
	{
		strcpy(buf.str()+buf.len(),"LINK ");
		buf.len()+=5;
	}
	else if(m_httpreq_iType==HTTP_REQ_DELETE)
	{
		strcpy(buf.str()+buf.len(),"DELETE ");
		buf.len()+=7;
	}
	else if(m_httpreq_iType==HTTP_REQ_UNLINK)
	{
		strcpy(buf.str()+buf.len(),"UNLINK ");
		buf.len()+=7;
	}
	else if(m_httpreq_iType==HTTP_REQ_CONNECT)
	{
		strcpy(buf.str()+buf.len(),"CONNECT ");
		buf.len()+=8;
	}
	else return; //�����http����
	if(m_bParseParams){
		encodeURL(buf); //����URL
		if(buf.Space()<12) if(buf.Resize(buf.size()+12)==NULL) return;
		buf.len()+=sprintf(buf.str()+buf.len()," HTTP/%d.%d\r\n",
				HIWORD(m_httpreq_dwVer),LOWORD(m_httpreq_dwVer));
	}else{
		if(buf.Space()<m_httpreq_strUrl.length()+12)
			if(buf.Resize(buf.size()+m_httpreq_strUrl.length()+12)==NULL) return;
		buf.len()+=sprintf(buf.str()+buf.len(),"%s HTTP/%d.%d\r\n",
			m_httpreq_strUrl.c_str(),HIWORD(m_httpreq_dwVer),LOWORD(m_httpreq_dwVer));	
	}
	return;
}

//����URL
inline void httpRequest::parseURL(const char *lpszurl)
{
	if(lpszurl) m_httpreq_strUrl.assign(lpszurl);
	//�����������������url
	const char *pFoundTerminator=strchr(m_httpreq_strUrl.c_str(),'?');
	if( pFoundTerminator)
	{
		parseParam((char *)pFoundTerminator+1,'&',m_httpreq_params_GET,NULL);
		m_httpreq_strUrl.erase(pFoundTerminator-m_httpreq_strUrl.c_str());
	}
	//url���ܱ����룬���ں��ֽ��Ƚ���mime����Ȼ�����utf8���룬���Ҫ����
	{
		int urllen=m_httpreq_strUrl.length();
		int iret=cCoder::mime_decode(m_httpreq_strUrl.c_str(),urllen,(char *)m_httpreq_strUrl.c_str());
		if(iret<urllen) //ֻ���б�����п��ܽ���
		iret=cCoder::utf8_decode(m_httpreq_strUrl.c_str(),iret,(char *)m_httpreq_strUrl.c_str());
		if(iret!=0) m_httpreq_strUrl.erase(iret); //yyc modify 2009-09-15
		//m_httpreq_strUrl.erase(iret);			  //yyc remove ������󱣻����п��ܷ�utf8����
	}
	return;
}

//����URL
inline void httpRequest::encodeURL(cBuffer &buf)
{
	int l=cCoder::MimeEncodeSize(m_httpreq_strUrl.length());
	if(l<128) l=128; //���Ǳ���һ���Ŀռ����Ƶ�����䲢�ƶ�
	if(buf.Space()<l)
		if(buf.Resize(buf.size()+l)==NULL) return;
	l=cCoder::mime_encodeURL(m_httpreq_strUrl.c_str(),m_httpreq_strUrl.length(),
		buf.str()+buf.len());
	buf.len()+=l;
	//����GET����
	if(m_httpreq_params_GET.size()>0)
	{
		buf[buf.len()]='?'; buf.len()++;
		encodeParam(buf,'&',m_httpreq_params_GET);
	}
	return;
}

//�����������
void httpRequest::parseParam(char *strParam,char delm,
							 std::map<std::string,std::string> &maps,const char *ptrCharset)
{
	if(strParam==NULL) return;
	int len,iret;
	bool ifutf_8=(ptrCharset && strcasecmp(ptrCharset,"utf-8")==0)?true:false;
	char *ptr,*ptrEnd,*ptrStart=strParam;
	while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	while( (ptrEnd=strchr(ptrStart,delm)) )
	{
		*ptrEnd=0;
		char *ptr=strchr(ptrStart,'=');
		if( (ptr=strchr(ptrStart,'=')) )
		{
			*ptr=0; len=ptr-ptrStart; iret=0;
			iret=cCoder::mime_decode(ptrStart,len,ptrStart);
			if(ifutf_8)
				iret=cCoder::utf8_decode(ptrStart,iret,ptrStart);
			ptr++;  len=ptrEnd-ptr; iret=0;
			iret=cCoder::mime_decode(ptr,len,ptr);
			if(ifutf_8)
				iret=cCoder::utf8_decode(ptr,iret,ptr);
			//nameӦ�����ִ�Сд�����ȫת��ΪСд
			::_strlwr(ptrStart);
			maps[ptrStart]=string(ptr);
		}
		ptrStart=ptrEnd+1;
		while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	}//?while
	//���ܻ���һ������û�д���
	if( (ptr=strchr(ptrStart,'=')) )
	{
		*ptr=0;
		iret=cCoder::mime_decode(ptrStart,ptr-ptrStart,ptrStart);
		if(ifutf_8)
			iret=cCoder::utf8_decode(ptrStart,iret,ptrStart);
		ptr++; len=strlen(ptr);
		//����Mozilla�����ʱ���� ���һ���������ܺ��лس����У���IEû��
//		while(len>0 && (*(ptr+len-1)=='\r' || *(ptr+len-1)=='\n') ) len--; //��ʱ�Ȳ�������Ϊ�����ᶪ��IE���͵�����\r\n
		iret=cCoder::mime_decode(ptr,len,ptr);
		if(ifutf_8)
			iret=cCoder::utf8_decode(ptr,iret,ptr);
		//nameӦ�����ִ�Сд�����ȫת��ΪСд
		::_strlwr(ptrStart);
		maps[ptrStart]=string(ptr);
	}
	return;
}
//�������
void httpRequest::encodeParam(cBuffer &buf,char delm,
							 std::map<std::string,std::string> &maps)
{
	std::map<std::string,std::string>::iterator it=maps.begin();
	for(;it!=maps.end();it++)
	{
		int l=cCoder::MimeEncodeSize((*it).first.length())+
			cCoder::MimeEncodeSize((*it).second.length())+4; //+1;
		if(l<128) l=128; //���Ǳ���һ���Ŀռ����Ƶ�����䲢�ƶ�
		if(buf.Space()<l)
			if(buf.Resize(buf.size()+l)==NULL) break;

		if(it!=maps.begin()) 
		{ 
			buf[buf.len()]=delm; buf.len()++;
			//����cookieÿ��cookieֵ��; �ֿ�
			if(delm==';'){ buf[buf.len()]=' '; buf.len()++; } 
		}
		l=cCoder::mime_encodeEx((*it).first.c_str(),(*it).first.length(),
			buf.str()+buf.len());
		buf.len()+=l; buf[buf.len()]='='; buf.len()++;
		l=cCoder::mime_encodeEx((*it).second.c_str(),(*it).second.length(),
			buf.str()+buf.len());
		buf.len()+=l; 
	}//?for
	return;
}

//�����ʺ�����
bool httpRequest::ParseAuthorizationBasic(const char *str,
										  std::string &username,std::string &password)
{
	const char *ptr,*ptrStart=str;
	while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	if(strncmp(ptrStart,"Basic ",6)==0)
	{
		username.assign(ptrStart+6);
		int iLen=cCoder::base64_decode((char *)username.c_str(),username.length(),(char *)username.c_str());
		username[iLen]=0;
		if( (ptr=strchr(username.c_str(),':')) )
		{
			username.erase(ptr-username.c_str());
			password.assign(ptr+1);
			return true;
		} else username="";
	}else if( (ptr=strchr(ptrStart,':')) )
	{ //����û�б�������봦��,��ʽ user:pswd
		username.assign(ptrStart,ptr-ptrStart);
		password.assign(ptr+1); return true;
	}
	return false;
}


