/*
**	FILENAME			IPF.h
**
**	PURPOSE				ͼ���ļ��Ĵ򿪣��洢,��ʽת��
**						Ŀǰֻ֧��BMP,JPEG����ͼ���ʽ
**
**	CREATION DATE		2003-12-24
**	LAST MODIFICATION	2005-09-22 ȥ����tiff��Pixͼ���ʽ����
**
**	AUTHOR				yyc
**
**	http://hi.baidu.com/yycblog/home
*/

#ifndef __IPF_20031224_H__
#define __IPF_20031224_H__

//----------------const define------------------
#define BMPINFOSIZE 2048
// DIBSCANLINE_WIDTHBYTES ִ��DIBɨ���е�DWORD����������������bits����
// DIB��Ϣ�ṹ�г�ԱbiWidth��biBitCount�ĳ˻�����Ľ���Ǿ��������һ��
// ɨ������ռ���ֽ�����
#define DIBSCANLINE_WIDTHBYTES(bits)    (((bits)+31)/32*4)
// DDBSCANLINE_WIDTHBYTES ִ��DDBɨ���е�WORD����������������bits����
// DDB��Ϣ�ṹ�г�ԱbmWidth��bmBitCount�ĳ˻�����Ľ���Ǿ��������һ��
// ɨ������ռ���ֽ�����
#define DDBSCANLINE_WIDTHBYTES(bits)    (((bits)+15)/16*2)

#ifndef BI_JPEG
	#define BI_JPEG        4L
#endif

#define SWAP_INT(x,y) \
{ \
	int l=(x); \
	(x)=(y); \
	(y)=l; \
}
//�����ֽڸ���
//#define BITSCOPY(dst,src,c) \
//	memcpy((dst),(src),(c))

#define BITSCOPY(dst,src,c) \
	{ \
		for(int n=0;n<(c);n++) \
			*((dst)+n)=*((src)+n); \
	}
//----------------------------------------------

#define IPFRESULT DWORD

class cImageF
{
public:
	//��λͼ�ļ� -- 
	//[in] filename ---- λͼ�ļ���
	//[out] lpbi ---- ����λͼ��Ϣ,�������ܵĵ�ɫ����Ϣ���û����뱣֤�㹻��
	//					һ����˵����BMPINFOSIZE�ֽ��㹻��
	//[out] lppBits ---- ����λͼ����ָ��(DWORD ����)���û����뱣֤�ռ��㹻��
	//				���lpBits==NULL���������λͼ��Ϣ
	//���أ����ʧ�ܷ���0�����򷵻ط�0(ͼ�����ݴ�С)
	static IPFRESULT IPF_LoadBMPFile(const char *filename,LPBITMAPINFO lpbi,LPBYTE lpBits);
	//��JPEG�ļ� -- 
	//[in] filename ---- JPEG�ļ���
	//[out] lpbi ---- ����λͼ��Ϣ
	//[out] lpBits ---- ����λͼ����ָ��(DWORD ����)���û����뱣֤�ռ��㹻��
	//				���lpBits==NULL���������λͼ��Ϣ
	//���أ����ʧ�ܷ���0�����򷵻ط�0(ͼ�����ݴ�С)
//	static IPFRESULT IPF_LoadJPEGFile(const char *filename,LPBITMAPINFO lpbi,LPBYTE lpBits);
	//�洢BMP�ļ� -- 
	//[in] filename ---- Ŀ��λͼ�ļ���
	//[in] lpbi ---- λͼ��Ϣ
	//[in] lpBits ---- λͼ����ָ��
	//���أ����ʧ�ܷ���0�����򷵻��ļ���С
//	static IPFRESULT IPF_SaveBMPFile(const char *filename,LPBITMAPINFO lpbi,LPBYTE lpBits);

	//�洢JPEG�ļ� -- 
	//Ŀǰֻ֧�ֽ�8λ�Ҷ�ͼ��24λ���ɫ
	//[in] filename ---- Ŀ��λͼ�ļ���
	//[in] lpbi ---- λͼ��Ϣͷ
	//[in] lpBits ---- λͼ����ָ��
	//[in] quality --- jpegѹ������ (0~100)
	//���أ����ʧ�ܷ���0�����򷵻��ļ���С
	static IPFRESULT IPF_SaveJPEGFile(const char *filename,LPBITMAPINFOHEADER lpbih,LPBYTE lpBits,int quality);
	//lpBuf --- jpeg������ 
	//dwSize --- jpeg��������С
	//���أ����ʧ�ܷ���0�����򷵻��ļ���С
	static IPFRESULT IPF_SaveJPEGFile(const char *filename,LPBYTE lpBuf,DWORD dwSize);
	//��λͼ����ѹ��ΪJPEG������ -- 
	//Ŀǰֻ֧�ֽ�8λ�Ҷ�ͼ��24λ���ɫ
	//[in] lpbih ---- λͼ��Ϣͷ
	//[in] lpBits ---- λͼ����ָ��
	//[out] dstBuf ---- �洢ת�����JPEG���ݵĿռ�,�û����뱣֤�ռ��㹻��
	//					һ����˵�����ԭλͼһ����Ŀռ伴��
	//[in] quality --- jpegѹ������ (0~100)
	//���أ����ʧ�ܷ���0�����򷵻�ѹ����jpeg�����ݵĴ�С
	static IPFRESULT IPF_EncodeJPEG(LPBITMAPINFOHEADER lpbih,LPBYTE lpBits,LPBYTE dstBuf,int quality);
	//��jpeg���ݽ�ѹ��Ϊλͼ������ -- 
	//[in] srcBuf ---- jpeg����ָ��
	//[in] dwSize ---- srcBufָ��Ŀռ�Ĵ�С
	//[out] lpbi ---- ����λͼ��Ϣ
	//[out] lpBits ---- ����λͼ����ָ��(DWORD ����)���û����뱣֤�ռ��㹻��
	//				���lpBits==NULL���������λͼ��Ϣ
	//���أ����ʧ�ܷ���0�����򷵻ط�0(ͼ�����ݴ�С)
//	static IPFRESULT IPF_DecodeJPEG(LPBYTE srcBuf,DWORD dwSize,LPBITMAPINFO lpbi,LPBYTE lpBits);

	//��׽����ͼ�� --- 24λ���ɫͼ��
	//���hWnd==NULL��׽������Ļ
	//lpbih --- biWidth ��biHeightָ����׽���ڵĿ��,==0��׽���ڵĿ���
	//			biCompressionָ��ͼ�����ݵ�ѹ����ʽ��Ŀǰֻ֧��BI_RGB(��ѹ��)��BI_JPEG(jpegѹ��)
	//			����ͼ�����Ϣ
	//lpBits --- ����ͼ�����ݻ�ѹ�����ͼ������
	//			���==NULL,���������ͼ��������Ҫ�Ŀռ��С
	//quality --- ���ָ����BI_JPEGѹ����˲���ָ��jpegѹ������
	//lprc --- ָ����׽���ڵ�����==NULL��Ϊ������������
	//ʧ�ܷ���0�����򷵻�ͼ�����ݴ�С
	//ifCapCursor�Ƿ񲶻������
	static IPFRESULT capWindow(HWND hWnd,LPBITMAPINFOHEADER lpbih,LPBYTE lpBits,int quality,bool ifCapCursor);

	//��ȡָ��DIB�ĵ�ɫ��ߴ磨���ֽ�Ϊ��λ��
//	static WORD PaletteSize(LPBITMAPINFOHEADER lpbih);

};

#endif

