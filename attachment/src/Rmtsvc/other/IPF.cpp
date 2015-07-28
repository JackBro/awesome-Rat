/*
**	FILENAME			IPF.h
**
**	PURPOSE				ͼ���ļ��Ĵ򿪣��洢,��ʽת��
**						Ŀǰֻ֧��BMP,JPEG����ͼ���ʽ
**
**	CREATION DATE		2003-12-24
**	LAST MODIFICATION	2003-12-24
**
**	AUTHOR				yyc
**
**	http://hi.baidu.com/yycblog/home
*/

#include <windows.h>
#include <stdio.h>
#include "IPF.h"

#ifdef __cplusplus
	extern "C" {
#endif // __cplusplus

#include "../libs/jpeg/jpeglib.h"

#ifdef __cplusplus
	}
#endif // __cplusplus

#pragma comment( lib, "libs/bin/jpeg-r-dll" )


//��λͼ�ļ� -- 
//[in] filename ---- λͼ�ļ���
//[out] lpbi ---- ����λͼ��Ϣ
//[out] lppBits ---- ����λͼ����ָ��(DWORD ����),�û����뱣֤�ռ��㹻��
//				���lppBits==NULL���������λͼ��Ϣ
//���أ����ʧ�ܷ���0�����򷵻ط�0(ͼ�����ݴ�С)
IPFRESULT cImageF :: IPF_LoadBMPFile(const char *filename,LPBITMAPINFO lpbi,LPBYTE lpBits)
{
	if(filename==NULL || lpbi==NULL) return 0;
	FILE * file=NULL;
	if((file=fopen(filename,"rb"))==NULL) return 0;
	DWORD dataLen=0;
	BITMAPFILEHEADER bmfHeader;
	fseek(file,0,SEEK_SET);
	if( fread((void *)&bmfHeader,1,sizeof(BITMAPFILEHEADER),file)==sizeof(BITMAPFILEHEADER)
		&& bmfHeader.bfType ==0x4D42 )//��λͼ�ļ�
	{
		fseek(file,sizeof(BITMAPFILEHEADER),SEEK_SET);
		fread((void *)lpbi,1,bmfHeader.bfOffBits-sizeof(BITMAPFILEHEADER),file);
		fseek(file,0,SEEK_END);
		dataLen=ftell(file)-bmfHeader.bfOffBits;
		if(lpBits)
		{ 
			// ��ȡλ����
			fseek(file,bmfHeader.bfOffBits,SEEK_SET);
			fread((void *)lpBits,1,dataLen,file);
		}//?if(lppBits)
	}//?if( fread((void *)&bmfHeader,1
	fclose(file);
	return dataLen;
}

/*
//��JPEG�ļ� -- 
//[in] filename ---- JPEG�ļ���
//[out] lpbi ---- ����λͼ��Ϣ
//[out] lpBits ---- ����λͼ����ָ��(DWORD ����)���û����뱣֤�ռ��㹻��
//				���lpBits==NULL���������λͼ��Ϣ
//���أ����ʧ�ܷ���0�����򷵻ط�0(ͼ�����ݴ�С)
IPFRESULT cImageF :: IPF_LoadJPEGFile(const char *filename,LPBITMAPINFO lpbi,LPBYTE lpBits)
{
	if(filename==NULL || lpbi==NULL) return 0;
	FILE * file=NULL;
	if((file=fopen(filename,"rb"))==NULL) return 0;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	// Now we can initialize the JPEG compression object. 
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, file);
	jpeg_read_header(&cinfo, TRUE);//�õ�ͼ���ͷ��Ϣ
	
//	if(cinfo.out_color_components==3)  
//		cinfo.out_color_space=JCS_RGB;
//	else
//		cinfo.out_color_space=JCS_GRAYSCALE;
	jpeg_start_decompress(&cinfo);

	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biPlanes = 1;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed = 0;
	lpbi->bmiHeader.biClrImportant = 0;
	lpbi->bmiHeader.biCompression =BI_RGB;
	lpbi->bmiHeader.biBitCount =cinfo.out_color_components*8;
	lpbi->bmiHeader.biHeight =cinfo.output_height;
	lpbi->bmiHeader.biWidth =cinfo.output_width;
	long lEffWidth=DIBSCANLINE_WIDTHBYTES(lpbi->bmiHeader.biWidth *lpbi->bmiHeader.biBitCount );
	lpbi->bmiHeader.biSizeImage =lEffWidth * lpbi->bmiHeader.biHeight ;
	
	if( lpbi->bmiHeader.biBitCount==8 ){
		lpbi->bmiHeader.biClrUsed=256;//(1<<8);
		RGBQUAD	*lpRGB=(RGBQUAD *)((LPSTR)lpbi+lpbi->bmiHeader.biSize);
		int ratio=lpbi->bmiHeader.biClrUsed/256;
		for(int i=0; i <(int)lpbi->bmiHeader.biClrUsed;i++) {
			lpRGB[i].rgbBlue=i/ratio;
			lpRGB[i].rgbGreen=i/ratio;
			lpRGB[i].rgbRed=i/ratio;
			lpRGB[i].rgbReserved=0;
		}
	}//?if( (lpbi->bmiHeader.biBitCount==8...

	if(lpBits)
	{ 
		JSAMPROW ptr=lpBits+lEffWidth*(lpbi->bmiHeader.biHeight-1);
		//ע��jpeg�������Ǵ��ϵ��¶�����windowsλͼ�����ݴ洢�Ǵ����һ�л��Ǵ漴
		//���µ���
		//�����ɫ����ȷ�����RGB����ɫ���ˣ�����"jpeg/jmorecfg.h"�ļ�����
		// Process data 
		 while (cinfo.output_scanline < cinfo.output_height) 
		 {
			 jpeg_read_scanlines(&cinfo,&ptr,1);
			 ptr-=lEffWidth;
		  }//?while(
		 jpeg_finish_decompress(&cinfo);
	}//?if(lppBits)

	jpeg_destroy_decompress(&cinfo);
	fclose(file);
	return lpbi->bmiHeader.biSizeImage;
}
*/
/*
//�洢BMP�ļ� -- 
//[in] filename ---- Ŀ��λͼ�ļ���
//[in] lpbi ---- λͼ��Ϣ
//[in] lpBits ---- λͼ����ָ��
//���أ����ʧ�ܷ���0�����򷵻��ļ���С
IPFRESULT cImageF :: IPF_SaveBMPFile(const char *filename,LPBITMAPINFO lpbi,LPBYTE lpBits)
{
	if (lpbi==NULL || lpBits==NULL ) return 0;
	if (filename==NULL || filename[0]==0) return 0;
	
	FILE *fp=::fopen(filename, "w+b");// �ô�����ʽ���ļ�(������)
	if(!fp) return 0;

	::fseek(fp, 0, SEEK_SET);
	//д���ļ�ͷ��Ϣ
	BITMAPFILEHEADER	bmf;	
	bmf.bfType = 0x4D42;		//('BM')
	// �ļ�ͷ�ߴ磫��Ϣͷ�ߴ磫��ɫ��ߴ磫λ���ݳߴ�
	DWORD dataSize=DIBSCANLINE_WIDTHBYTES(lpbi->bmiHeader.biWidth *lpbi->bmiHeader.biBitCount) 
		*lpbi->bmiHeader.biHeight;
	DWORD palSize=PaletteSize((LPBITMAPINFOHEADER)lpbi);
    bmf.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+palSize+dataSize;
    bmf.bfReserved1 = 0; 
    bmf.bfReserved2 = 0;
	// �ļ�ͷ�ߴ磫��Ϣͷ�ߴ磫��ɫ��ߴ�
    bmf.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +palSize;
	
	// д���ļ�ͷ��Ϣ
	::fwrite((const void *)&bmf, sizeof(BITMAPFILEHEADER), 1, fp);

	// д����Ϣ�����ݺ�����
	LPBYTE lp = lpBits;
	
	::fwrite((const void *)lpbi, sizeof(BITMAPINFOHEADER) +palSize, 1, fp);
	DWORD dwB, dwC;
	// �Էֶη�ʽд��λ���ݣ�ÿ���γ���Ϊ32KB��
	dwB = dataSize/32768;			// ������32768��
	dwC = dataSize - (dwB*32768);	// ����
	for (;dwB!=0;dwB--)
	{
		::fwrite((const void *)lp, 32768, 1, fp);
		lp = (LPBYTE)((DWORD)lp+32768UL);
	}
	// д��ʣ���λ����
	::fwrite((const void *)lp, dwC, 1, fp);

	::fclose(fp);
	return bmf.bfSize;
}
*/

//�洢JPEG�ļ� -- 
//Ŀǰֻ֧�ֽ�8λ�Ҷ�ͼ��24λ���ɫ
//[in] filename ---- Ŀ��λͼ�ļ���
//[in] lpbi ---- λͼ��Ϣ
//[in] lpBits ---- λͼ����ָ��
//[in] quality --- jpegѹ������ (0~100)
//���أ����ʧ�ܷ���0�����򷵻��ļ���С
IPFRESULT cImageF :: IPF_SaveJPEGFile(const char *filename,LPBITMAPINFOHEADER lpbih,LPBYTE lpBits,int quality)
{
	if (lpbih==NULL || lpBits==NULL ) return 0;
	if (filename==NULL || filename[0]==0) return 0;
	//Ŀǰֻ֧�ֽ�8λ�Ҷ�ͼ��24λ���ɫ
	if(lpbih->biBitCount!=8 && lpbih->biBitCount!=24) return 0;
	// ��֧��ѹ��λͼ
//	ASSERT(lpbih->biCompression == BI_RGB);
	//����ԭͼ����п�
	if(lpbih->biSizeImage==0)
		lpbih->biSizeImage=lpbih->biHeight * 
			DIBSCANLINE_WIDTHBYTES(lpbih->biWidth *lpbih->biBitCount);
	long lEffWidth =lpbih->biSizeImage/lpbih->biHeight;

	FILE *fp=::fopen(filename, "w+b");// �ô�����ʽ���ļ�(������)
	if(!fp) return 0;
	::fseek(fp, 0, SEEK_SET);

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);
	cinfo.image_width =lpbih->biWidth; 	// image width and height, in pixels
	cinfo.image_height =lpbih->biHeight;
	if (lpbih->biBitCount==24) {
		cinfo.input_components = 3;		// # of color components per pixel 
		cinfo.in_color_space = JCS_RGB; 	// colorspace of input image 
	 } else {
		cinfo.input_components = 1;		// # of color components per pixel 
		cinfo.in_color_space = JCS_GRAYSCALE; 	// colorspace of input image 
	 }
	jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    cinfo.dct_method = JDCT_FASTEST;
    cinfo.optimize_coding = TRUE;
	jpeg_start_compress(&cinfo, TRUE);
	//ע��jpeg�������Ǵ��ϵ��¶�����windowsλͼ�����ݴ洢�Ǵ����һ�л��Ǵ漴
	//���µ���
	//�����ɫ����ȷ�����RGB����ɫ���ˣ�����"jpeg/jmorecfg.h"�ļ�����
	BYTE * IterImage=lpBits+lEffWidth*(cinfo.image_height-1);
	while (cinfo.next_scanline < cinfo.image_height) 
	{	
		 jpeg_write_scanlines(&cinfo, &IterImage, 1);
		 IterImage -= lEffWidth;
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fseek(fp,0,SEEK_END);
	lEffWidth=ftell(fp);
	fclose(fp);
	return lEffWidth;
}

//lpBuf --- jpeg������ 
//dwSize --- jpeg��������С
//���أ����ʧ�ܷ���0�����򷵻��ļ���С
IPFRESULT cImageF::IPF_SaveJPEGFile(const char *filename,LPBYTE lpBuf,DWORD dwSize)
{
	if (lpBuf==NULL || dwSize==0 ) return 0;
	if (filename==NULL || filename[0]==0) return 0;
	FILE *fp=::fopen(filename, "w+b");// �ô�����ʽ���ļ�(������)
	if(!fp) return 0;
	::fseek(fp, 0, SEEK_SET);

	LPBYTE lp = lpBuf;
	DWORD dwB, dwC;
	// �Էֶη�ʽд��λ���ݣ�ÿ���γ���Ϊ32KB��
	dwB = dwSize/32768;			// ������32768��
	dwC = dwSize - (dwB*32768);	// ����
	for (;dwB!=0;dwB--)
	{
		::fwrite((const void *)lp, 32768, 1, fp);
		lp = (LPBYTE)((DWORD)lp+32768UL);
	}
	// д��ʣ���λ����
	::fwrite((const void *)lp, dwC, 1, fp);
	::fclose(fp);
	return dwSize;
}

//��λͼ����ѹ��ΪJPEG������ -- 
//Ŀǰֻ֧�ֽ�8λ�Ҷ�ͼ��24λ���ɫ
//[in] lpbih ---- λͼ��Ϣͷ
//[in] lpBits ---- λͼ����ָ��
//[out] dstBuf ---- �洢ת�����JPEG���ݵĿռ�,�û����뱣֤�ռ��㹻��
//					һ����˵�����ԭλͼһ����Ŀռ伴��
//[in] quality --- jpegѹ������ (0~100)
//���أ����ʧ�ܷ���0�����򷵻�ѹ����jpeg�����ݵĴ�С
METHODDEF void init_destination (j_compress_ptr cinfo)
{
	return;
}
METHODDEF boolean empty_output_buffer (j_compress_ptr cinfo)
{
	return true;
}
METHODDEF void term_destination (j_compress_ptr cinfo)
{
	return;
}
IPFRESULT cImageF :: IPF_EncodeJPEG(LPBITMAPINFOHEADER lpbih,LPBYTE lpBits,LPBYTE dstBuf,int quality)
{
	if (lpbih==NULL || lpBits==NULL || dstBuf==NULL ) return 0;
	//Ŀǰֻ֧�ֽ�8λ�Ҷ�ͼ��24λ���ɫ
	if(lpbih->biBitCount!=8 && lpbih->biBitCount!=24) return 0;
	//����ԭͼ����п�
	if(lpbih->biSizeImage==0)
		lpbih->biSizeImage=lpbih->biHeight * 
			DIBSCANLINE_WIDTHBYTES(lpbih->biWidth *lpbih->biBitCount);
	long lEffWidth =lpbih->biSizeImage/lpbih->biHeight;
	LPBYTE lpDstBits=dstBuf;
	if(dstBuf==lpBits)
	{//Դ��ַ��Ŀ�ĵ�ַ��ͬ
		if( (lpDstBits=(LPBYTE)::malloc(lpbih->biSizeImage))==NULL)
			return 0;
	}
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr dest;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	dest.next_output_byte = lpDstBits;
	dest.free_in_buffer = lpbih->biSizeImage;
	dest.init_destination = init_destination;//NULL;
	dest.empty_output_buffer = empty_output_buffer;//NULL;
	dest.term_destination = term_destination;//NULL;
	cinfo.dest=&dest;
	cinfo.image_width =lpbih->biWidth; 	// image width and height, in pixels
	cinfo.image_height =lpbih->biHeight;
	if (lpbih->biBitCount==24) {
		cinfo.input_components = 3;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	 } else {
		cinfo.input_components = 1;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
	 }
	jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    cinfo.dct_method = JDCT_FASTEST;
    cinfo.optimize_coding = TRUE;
	
	jpeg_start_compress(&cinfo, TRUE);
	
	//ע��jpeg�������Ǵ��ϵ��¶�����windowsλͼ�����ݴ洢�Ǵ����һ�л��Ǵ漴
	//���µ���
	//�����ɫ����ȷ�����RGB����ɫ���ˣ�����"jpeg/jmorecfg.h"�ļ�����
	BYTE * IterImage=lpBits+lEffWidth*(cinfo.image_height-1);
	while (cinfo.next_scanline < cinfo.image_height) 
	{	
		 jpeg_write_scanlines(&cinfo, &IterImage, 1);
		 IterImage -= lEffWidth;
	}
	
	jpeg_finish_compress(&cinfo);
	lEffWidth=lpbih->biSizeImage-dest.free_in_buffer;
	jpeg_destroy_compress(&cinfo);

	if(dstBuf==lpBits)
	{//Դ��ַ��Ŀ�ĵ�ַ��ͬ
		memcpy((void *)lpBits,(const void *)lpDstBits,lEffWidth);
		::free(lpDstBits);
	}
	return lEffWidth;
}
/*
//��jpeg���ݽ�ѹ��Ϊλͼ������ -- 
//[in] srcBuf ---- jpeg����ָ��
//[in] dwSize ---- srcBufָ��Ŀռ�Ĵ�С
//[out] lpbi ---- ����λͼ��Ϣ
//[out] lpBits ---- ����λͼ����ָ��(DWORD ����)���û����뱣֤�ռ��㹻��
//				���lpBits==NULL���������λͼ��Ϣ
//���أ����ʧ�ܷ���0�����򷵻ط�0(ͼ�����ݴ�С)
IPFRESULT cImageF :: IPF_DecodeJPEG(LPBYTE srcBuf,DWORD dwSize,LPBITMAPINFO lpbi,LPBYTE lpBits)
{
	if(srcBuf==NULL || dwSize<=0 || lpbi==NULL) return 0;
	
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_source_mgr src;
	cinfo.err = jpeg_std_error(&jerr);
	// Now we can initialize the JPEG compression object. 
	jpeg_create_decompress(&cinfo);
	src.next_input_byte = srcBuf;
	src.bytes_in_buffer = dwSize;
	src.init_source = NULL;
	src.fill_input_buffer=NULL;
	src.skip_input_data=NULL;
	src.resync_to_restart=NULL;
	src.term_source=NULL;
	cinfo.src=&src;

	jpeg_read_header(&cinfo, TRUE);//�õ�ͼ���ͷ��Ϣ
	jpeg_start_decompress(&cinfo);
	
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biPlanes = 1;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed = 0;
	lpbi->bmiHeader.biClrImportant = 0;
	lpbi->bmiHeader.biCompression =BI_RGB;
	lpbi->bmiHeader.biBitCount =cinfo.out_color_components*8;
	lpbi->bmiHeader.biHeight =cinfo.output_height;
	lpbi->bmiHeader.biWidth =cinfo.output_width;
	long lEffWidth=DIBSCANLINE_WIDTHBYTES(lpbi->bmiHeader.biWidth *lpbi->bmiHeader.biBitCount );
	lpbi->bmiHeader.biSizeImage =lEffWidth * lpbi->bmiHeader.biHeight ;

	if( lpbi->bmiHeader.biBitCount==8 ){
		lpbi->bmiHeader.biClrUsed=256;//(1<<8);
		RGBQUAD	*lpRGB=(RGBQUAD *)((LPSTR)lpbi+lpbi->bmiHeader.biSize);
		int ratio=lpbi->bmiHeader.biClrUsed/256;
		for(int i=0; i <(int)lpbi->bmiHeader.biClrUsed;i++) {
			lpRGB[i].rgbBlue=i/ratio;
			lpRGB[i].rgbGreen=i/ratio;
			lpRGB[i].rgbRed=i/ratio;
			lpRGB[i].rgbReserved=0;
		}
	}//?if( (lpbi->bmiHeader.biBitCount==8...

	if(lpBits)
	{ 
		JSAMPROW ptr=lpBits+lEffWidth*(lpbi->bmiHeader.biHeight-1);
		//ע��jpeg�������Ǵ��ϵ��¶�����windowsλͼ�����ݴ洢�Ǵ����һ�л��Ǵ漴
		//���µ���
		//�����ɫ����ȷ�����RGB����ɫ���ˣ�����"jpeg/jmorecfg.h"�ļ�����
		// Process data 
		 while (cinfo.output_scanline < cinfo.output_height) 
		 {
			 jpeg_read_scanlines(&cinfo,&ptr,1);
			 ptr-=lEffWidth;
		  }//?while(
		 jpeg_finish_decompress(&cinfo);
	}//?if(lppBits)

	jpeg_destroy_decompress(&cinfo);

	return lpbi->bmiHeader.biSizeImage;
}
*/
//------------------------------------------------------------------------------------------
//��׽����ͼ�� --- 24λ���ɫͼ��
//���hWnd==NULL��׽������Ļ
//lpbih --- 
//			biCompressionָ��ͼ�����ݵ�ѹ����ʽ��Ŀǰֻ֧��BI_RGB(��ѹ��)��BI_JPEG(jpegѹ��)
//			����ͼ�����Ϣ
//lpBits --- ����ͼ�����ݻ�ѹ�����ͼ������
//			���==NULL,���������ͼ��������Ҫ�Ŀռ��С
//quality --- ���ָ����BI_JPEGѹ����˲���ָ��jpegѹ������
//ʧ�ܷ���0�����򷵻�ͼ�����ݴ�С
//ifCapCursor�Ƿ񲶻������
IPFRESULT cImageF::capWindow(HWND hWnd,LPBITMAPINFOHEADER lpbih,LPBYTE lpBits,int quality,bool ifCapCursor)
{
	if(lpbih==NULL) return 0;
	if(hWnd == NULL)
		if((hWnd = ::GetDesktopWindow())==NULL) return 0;
	
	RECT rect;
	::GetClientRect(hWnd, &rect);

	//Ҫ�����ԭͼ���������ʼ������Ϳ��
	long lX=0,tY=0;
	long lWidth=rect.right - rect.left;
	long lHeight=rect.bottom - rect.top ;

	lpbih->biSize = sizeof(BITMAPINFOHEADER);
	lpbih->biPlanes = 1;
	lpbih->biXPelsPerMeter=0;
	lpbih->biYPelsPerMeter=0;
	lpbih->biClrUsed = 0;
	lpbih->biClrImportant = 0;
	lpbih->biBitCount =24;
	lpbih->biHeight =lHeight;
	lpbih->biWidth =lWidth;
	long lEffWidth=DIBSCANLINE_WIDTHBYTES(lpbih->biWidth *lpbih->biBitCount );
	DWORD dwRet=lpbih->biSizeImage =lEffWidth * lpbih->biHeight ;
	if(lpBits==NULL) return dwRet;
	//�Ƿ�ѹ��
	BOOL ifComp=(lpbih->biCompression==BI_JPEG);
	lpbih->biCompression =BI_RGB;
	
	HDC hWndDC = NULL;
	HDC hMemDC = NULL;
	HBITMAP hMemBmp = NULL;
	HBITMAP hOldBmp = NULL;
	hWndDC = ::GetDC(hWnd);
	hMemDC = ::CreateCompatibleDC(hWndDC);
	hMemBmp = ::CreateCompatibleBitmap(hWndDC, lWidth, lHeight);
	hOldBmp = (HBITMAP)::SelectObject(hMemDC, hMemBmp);
	::BitBlt(hMemDC, 0, 0, lWidth, lHeight, hWndDC, lX, tY, SRCCOPY);
	
	if(ifCapCursor) //���������
	{
		POINT ptCursor;
		::GetCursorPos(&ptCursor);
		//�Ȼ��������µĴ��ھ�����õ��ô��ڵ��߳�ID
		//Attatch��ǰ�̵߳�ָ���Ĵ����߳�
		//��øô��ڵ�ǰ�������
		//Deattach
		//!!!�������������ֱ�ӵ���GetCursor()�����ǻ�õ�ǰ�̵߳Ĺ����
		//���û���������õ�����©�������
		HWND hw=::WindowFromPoint(ptCursor);
		if(hw==NULL) hw=::GetDesktopWindow();
		DWORD hdl=::GetWindowThreadProcessId(hw,NULL);
		::AttachThreadInput(::GetCurrentThreadId(),hdl,TRUE);
		HCURSOR hCursor=::GetCursor();
		::AttachThreadInput(::GetCurrentThreadId(),hdl,FALSE);
		//��ȡ����ͼ������ 
		ICONINFO IconInfo;
		if (::GetIconInfo(hCursor, &IconInfo))
		{
			ptCursor.x -= ((int) IconInfo.xHotspot);
			ptCursor.y -= ((int) IconInfo.yHotspot);
			
			//if (IconInfo.hbmMask != NULL)
			//	::DeleteObject(IconInfo.hbmMask);
			//if (IconInfo.hbmColor != NULL)
			//	::DeleteObject(IconInfo.hbmColor);
		}
		//�ڼ����豸�������ϻ����ù��
		::DrawIconEx(
		hMemDC, // handle to device context 
		ptCursor.x, ptCursor.y,
		hCursor, // handle to icon to draw 
		0,0, // width of the icon 
		0, // index of frame in animated cursor 
		NULL, // handle to background brush 
		DI_NORMAL | DI_COMPAT // icon-drawing flags 
		); 
	}//?if(ifCapCursor) //���������

	if(!::GetDIBits(hWndDC,hMemBmp,0,lHeight,lpBits,(LPBITMAPINFO)lpbih,DIB_RGB_COLORS))
		dwRet=0;
	else if(ifComp)
	{//�Ƿ����jpegѹ��
		dwRet=cImageF::IPF_EncodeJPEG(lpbih,lpBits,lpBits,quality);
		lpbih->biCompression =BI_JPEG;
	}
	::SelectObject(hMemDC, hOldBmp);
	::DeleteObject(hMemBmp);
	::DeleteDC(hMemDC);
	::ReleaseDC(hWnd, hWndDC);
	return dwRet;
}

//***********************************************************************************************
//**********************************************private function ********************************
/*
//��ȡָ��DIB�ĵ�ɫ��ߴ磨���ֽ�Ϊ��λ��
WORD cImageF::PaletteSize(LPBITMAPINFOHEADER lpbih)
{
	WORD size, wBitCount;

	// ���ÿ��������ռ��λ��
	wBitCount =lpbih->biBitCount;
				
	// 16λ��32λλͼ����ɫ����ռ������DWORD��ֵ����ʾ
	// �졢�̡�����λ�����е�����
	if ((wBitCount == 16)||(wBitCount == 32))
	{
		return sizeof(DWORD)*3;
	}
	else
	{
		// ��֧��ѹ��λͼ
		//ASSERT(lpbi->biCompression == BI_RGB);
		WORD wc;
		if (lpbih->biClrUsed) 
			wc=(WORD)(lpbih->biClrUsed);
		else
		{
			switch (lpbih->biBitCount)
			{
				case 1:
					wc=2;
					break;          // ��ɫλͼ��ֻ�кڰ�������ɫ
				case 4:
					wc=16;		
					break;			// ��׼VGAλͼ����16����ɫ
				case 8:
					wc=256;		
					break;			// SVGAλͼ����256����ɫ
				case	16:			// 64Kɫλͼ
				case	24:			// 16Mɫλͼ�����ɫ��
				case	32:			// 16M+ɫλͼ�����ɫ��
					wc=0;		
					break;			// ��ɫ����û����ɫ���ݷ���0
				default:
					return 0; //����
			}
		}
		size = (WORD)(wc * sizeof(RGBQUAD));
	}
	return size;	
}
*/

