/*******************************************************************
   *	utils.h
   *    DESCRIPTION:���ߺ�����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-30
   *	net4cpp 2.1
   *******************************************************************/
  
#ifndef __YY_CUTILS_H__
#define __YY_CUTILS_H__

namespace net4cpp21
{
	class cUtils
	{
	public:
#ifdef WIN32
		static void usleep(unsigned int us)
		{
			Sleep(us/1000);
		}
		//����һ������ //����0�ɹ�
		static int execp(const char *pathfile,const char *args);
#else
		static void usleep(unsigned int us)
		{
			struct timeval to;
			//to.tv_sec = 0;
			//���üӱ�����us����>=0��С��1000000
			//Ϊ�����Ч�ʣ��˱����ɵ������ṩ
			//to.tv_usec = us;
			to.tv_sec =us/1000000L;
			to.tv_usec =us%1000000L;
			select(0, NULL, NULL, NULL, &to);
		}
#endif
		//�޳��ַ�����ǰ���ո��Լ�β���ո�
		static const char *strTrimLeft(const char *str)
		{
			if(str) for(;*str==' ';str++) NULL;
			return str;
		}
		static char *strTrimRight(char *str)
		{
			if(str)
			{
				char *ptr=str+strlen(str)-1;
				while(ptr>=str && *ptr==' ') *ptr--='\0';
			}
			return str;
		}
		static char *strTrim(char *str)
		{
			if(str){
				for(;*str==' ';str++) NULL;
				char *ptr=str+strlen(str)-1;
				while(ptr>=str && *ptr==' ') *ptr--='\0';
			}
			return str;
		}

		//�����滻�ַ��ĸ���
		static int Replace(char *str,char findC,char replaceC)
		{
			int count=0;
			for(;*str!='\0' && *str==findC;*str++=replaceC,count++) NULL;
			return count;
		}
	};

	class FILEIO
	{
	public:
#ifdef WIN32
		static bool fileio_rename(const char *filename,const char *rename)
		{
		//	char *p=(char *)rename; while(*p){ if(*p=='/') *p='\\'; p++;}
			return (::MoveFile(filename,rename))?true:false;
		}

		static bool fileio_deleteFile(const char *filename)
		{
			return (::DeleteFile(filename))?true:false;
		}
		static bool fileio_createDir(const char *spath)
		{
			return (::CreateDirectory(spath,NULL))?true:false;
		}
		static unsigned long fileio_deleteDir(const char *spath);
		
		//����ļ��������򷵻�-1
		//���ָ����·����һ��Ŀ¼�򷵻�-2
		//���򷵻��ļ���СBytes
		static long fileio_exist(const char *spath);
		static bool fileio_exec(char *filename,bool ifHide);
#endif
	};
}//?namespace net4cpp21

#endif


/*
//���ַ���һ�����������в���ָ����ֵ
	//���������±�,����-1δ�ҵ�
	//uiSize -- �����С, ifUp -- ָ���Ƿ�Ϊ��������
	template<class X> 
	int findit(const X iArray[],size_t uiSize,const X& xfind,bool ifUp)
	{
		if(uiSize==0) return -1;
		int idx,lbound=0,ubound=uiSize-1;
		if(ifUp)
		{
			while((idx=(ubound-lbound))>1)
			{
				idx=lbound+idx/2;
				if(iArray[idx]>xfind){ ubound=idx; continue; }
				if(iArray[idx]<xfind){ lbound=idx; continue; }
				return idx;
			}
		}
		else
		{
			while((idx=(ubound-lbound))>1)
			{
				idx=lbound+idx/2;
				if(iArray[idx]>xfind){ lbound=idx; continue; }
				if(iArray[idx]<xfind){ ubound=idx; continue; }
				return idx;
			}
		}
		if(iArray[lbound]==xfind) return lbound;
		if(iArray[ubound]==xfind) return ubound;
		return -1;
	}

	//����ж�һ������2��ָ����
	bool if2(long num)
	{
		return ((num-1) & num)==0);
	}

  //ð�ݷ�����
//��򵥵����򷽷���ð�����򷽷������ַ����Ļ���˼���ǣ����������Ԫ�ؿ������������еġ����ݡ���
//��С��Ԫ�رȽ��ᣬ�Ӷ�Ҫ���ϸ�����ð�������㷨������Ҫ����������ݡ����д������ɱ顣
//��νһ�鴦�������Ե����ϼ��һ��������У���ʱ��ע���������ڵ�Ԫ�ص�˳���Ƿ���ȷ��
//���������������Ԫ�ص�˳�򲻶ԣ������ᡱ��Ԫ�������棬�ͽ������ǵ�λ�á���Ȼ������
//һ��֮�󣬡����ᡱ��Ԫ�ؾ͸��������λ�ã��������֮�󣬡����ᡱ��Ԫ�ؾ͸����˴θ�λ�á�
//�����ڶ��鴦��ʱ���������λ���ϵ�Ԫ�����ǡ����ᡱԪ�أ����Բ��ؼ�顣һ��أ���i�鴦��ʱ��
//���ؼ���i��λ�����ϵ�Ԫ�أ���Ϊ����ǰ��i-1��Ĵ�����������ȷ���ź���	
template <class X>
void Sort(X iArray[],size_t lbound,size_t ubound)
{
	X tmp; 
	bool bSwap;
	for(size_t j=ubound;j>lbound;j--)
	{
		bSwap=false;
		for(size_t i=lbound;i<j;i++)
		{
			if(iArray[i]>iArray[i+1])
			{
				tmp=iArray[i];
				iArray[i]=iArray[i+1];
				iArray[i+1]=tmp;
				bSwap=true;
			}
		}
		if(!bSwap) break;
	}
	return;
}

//���������㷨�Ļ���˼�룺
//��������Ļ���˼���ǻ��ڷ��β��Եġ����������������ap..ar�������ģ�㹻С��ֱ�ӽ������򣬷������������
//
//�ֽ�(Divide)�������������ap..ar���ֳ������ǿ�������ap..aq��aq+1..ar��ʹap..aq����һԪ�ص�ֵ������aq+1..ar����һԪ�ص�ֵ��
//�ݹ����(Conquer)��ͨ���ݹ���ÿ��������㷨�ֱ��ap..aq��aq+1..ar��������
//�ϲ�(Merge)�����ڶԷֽ�������������е������Ǿ͵ؽ��еģ�������ap..aq��aq+1..ar���ź������Ҫִ���κμ���ap..ar�����ź���
//�����������Ƿ��Ϸ��η��Ļ�������ġ���ˣ����������Ƿ��η��ľ���Ӧ��ʵ��֮һ��
template <class X>
void quickSort(X v[],int n)
{
	if(n<=1)  return;
	int i,last = 0;
	X temp;
    for(i=1; i<n; i++)
    {
         if(v[i] < v[0] && (++last!=i) )
         {
				temp = v[last];
				v[last] = v[i];
				v[i] = temp;
         }
    }
	temp = v[last];
	v[last] = v[0];
	v[0] = temp;

    quickSort(v, last);    //recursively srot
    quickSort(v+last+1, n-last-1);    //each part
}
*/


