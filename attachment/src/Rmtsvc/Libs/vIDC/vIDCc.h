/*******************************************************************
   *	vIDCc.h
   *    DESCRIPTION:vIDC�ͻ���Ķ���
   *
   *    AUTHOR:yyc
   *	http://hi.baidu.com/yycblog/home
   *
   *******************************************************************/
   
#ifndef __YY_CVIDCC_H__
#define __YY_CVIDCC_H__

#include "../../net4cpp21/include/proxyclnt.h"
#include "../../net4cpp21/include/buffer.h"
#include "vidcdef.h"
#include "vidccdef.h"

namespace net4cpp21
{
	//vidcc�ͻ�����
	class vidcClient : public socketProxy
	{
	public:
		explicit vidcClient(const char *strname,const char *strdesc);
		virtual ~vidcClient();
		VIDCSINFO &vidcsinfo() { return m_vidcsinfo; }
		void Destroy(); //���ٲ��ͷ���Դ
		//����ָ����vIDCs����
		SOCKSRESULT ConnectSvr();
		void DisConnSvr(); //�Ͽ���vIDCs������
		bool mapinfoDel(const char *mapname);
		mapInfo * mapinfoGet(const char *mapname,bool bCreate);
		
		//�ɹ�����SOCKSERR_OK
		int Mapped(const char *mapname,mapInfo *pinfo); //ӳ��ָ���ķ���
		int Unmap(const char *mapname,mapInfo *pinfo); //ȡ��ӳ��ָ���ķ���

		void xml_list_mapped(cBuffer &buffer,VIDC_MAPTYPE maptype);
		void str_list_mapped(const char *vname,std::string &strini);
	private:
		bool sendCommand(int response_expected,const char *buf,int buflen);
		void parseCommand(const char *ptrCommand);
		static void onPipeThread(vidcClient *pvidcc);
		static void onCommandThread(vidcClient *pvidcc);
	private:
		std::map<std::string,mapInfo *> m_mapsets; //ӳ�伯��
		time_t m_lTimeout;//���ȴ���ʱ����s
		std::string m_strName; //��vidcc������
		std::string m_strDesc;
		VIDCSINFO m_vidcsinfo;
		cThreadPool m_threadpool;//�����̳߳�

		char m_szLastResponse[VIDC_MAX_COMMAND_SIZE]; //�������һ�δ�vIDCs�������
	};
	
	class vidccSets
	{
	public:
		vidccSets();
		~vidccSets();
		void Destroy();
		void autoConnect();
		vidcClient *GetVidcClient(const char *vname,bool bCreate);
		bool DelVidcClient(const char *vname);
		
		bool xml_info_vidcc(cBuffer &buffer,const char *vname,VIDC_MAPTYPE maptype);
		void xml_list_vidcc(cBuffer &buffer);
		void str_list_vidcc(std::string &strini);
	private:
		cMutex m_mutex;
		//ÿ��vidcClient��Ӧ����һ��vIDCs
		std::map<std::string,vidcClient *> m_vidccs;
		std::string m_strName; //vidcc������
		std::string m_strDesc;
	};

}//?namespace net4cpp21

#endif

