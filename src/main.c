#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "info_mp3.h"

struct ID3v2_label ID3v2Label; 
struct Label_header MP3LHeader;
struct Label_frame_header MP3LFramHeader;
struct Frame_VBR_header MP3VBRHeader;
struct Frame_header Fheader;


struct MP3_info MP3Info;


int getLabelHeader(FILE *fp)
{
	int ret;
	printf("get header\n");
	char buf[20];
	memset(buf, 0, sizeof(buf));
	fseek(fp,0,SEEK_SET);
	ret = fread(buf, sizeof(char), 10, fp);
	memcpy(&MP3LHeader, buf, sizeof(MP3LHeader));
	printf("header=%.*s\n", 3, MP3LHeader.Header);
	printf("ver=%d\n", (int)MP3LHeader.ver);
	printf("ReVer=%c\n", MP3LHeader.ReVer);
	printf("Flag=%d\n", (int)MP3LHeader.Flag);
	
	int total_size;
	total_size = (MP3LHeader.Size[0]&0x7F)*0x200000 \
				+(MP3LHeader.Size[1]&0x7F)*0x4000 \
				+(MP3LHeader.Size[2]&0x7F)*0x80 \
				+(MP3LHeader.Size[3]&0x7F);
	ID3v2Label.TotalSize = total_size;
	printf("Size=%d\n", total_size);

    return (ID3v2Label.TotalSize -= 10);
}

int getLabelFrameHeader(FILE *fp)
{

    if (ID3v2Label.TotalSize < 1) {
        printf("getLabelFrameHeader: Label end\n");
        return 0;
    }
	char buf[20];
	memset(buf, 0 , sizeof(buf));
	fread(buf, sizeof(char), 10, fp);
	memset(&MP3LFramHeader, 0, sizeof(MP3LFramHeader));
	memcpy(&MP3LFramHeader, buf, sizeof(MP3LFramHeader));
	printf("FrameID=%.*s\n", 4, MP3LFramHeader.FrameID);
	int FSize;
	FSize = MP3LFramHeader.Size[0]*0x1000000 \
		   +MP3LFramHeader.Size[1]*0x10000 \
		   +MP3LFramHeader.Size[2]*0x100 \
		   +MP3LFramHeader.Size[3];
	printf("Size=%d\n", FSize);
	ID3v2Label.LabelFrameSize = FSize;
	/* Flags[2]
	只定义了 6 位,另外的 10 位为 0,但大部分的情况下 16 位都为 0 就可以了。格式如下:
	abc00000 ijk00000
	a -- 标签保护标志,设置时认为此帧作废
	b -- 文件保护标志,设置时认为此帧作废
	c -- 只读标志,设置时认为此帧不能修改(但我没有找到一个软件理会这个标志)
	i -- 压缩标志,设置时一个字节存放两个 BCD 码表示数字
	j -- 加密标志(没有见过哪个 MP3 文件的标签用了加密)
	k -- 组标志,设置时说明此帧和其他的某帧是一组
	*/
    return (ID3v2Label.TotalSize -= 10);
}

int getLabelFrameContent(FILE *fp)
{
    if (ID3v2Label.TotalSize < 1) {
        printf("getLabelFrameContent: Label end\n");
        return 0;
    }
    if (ID3v2Label.LabelFrameSize < 1){
        printf("getLabelFrameContent: Frame null\n");
        return 0;
    }
    char buf[1024];
    memset(buf, 0 , sizeof(buf));
	fread(buf, sizeof(char), ID3v2Label.LabelFrameSize, fp);
	printf("buf=[%s]\n", buf+1);

	return (ID3v2Label.TotalSize -= ID3v2Label.LabelFrameSize);
}



int findFrameMPEGHeader(FILE *fp)
{
	int sync = 0x7FF;
	char buf[1024];
    memset(buf, 0 , sizeof(buf));
    
    do{
		fread(buf, sizeof(char), 2, fp);
		//printf("sync=%d\n", (buf[0]&sync && buf[1]&sync));
	}while(!(buf[0]&sync && buf[1]&sync));
	fseek(fp, -2L, SEEK_CUR);
	return 0;
}

int findFrameVBRHeader(FILE *fp)
{
	int ret;
	unsigned char buf[1024];
    memset(buf, 0 , sizeof(buf));
    
    do{
		ret = fread(MP3VBRHeader.Header, sizeof(char), 4, fp);
		if (0==strncmp(MP3VBRHeader.Header, "Info",4) \
		      || 0==strncmp(MP3VBRHeader.Header, "Xing", 4)){
			printf("findFrameVBRHeader ok\n");
			break;
		}
	} while(ret);
	printf("VBR Header=[%.*s]\n", 4, MP3VBRHeader.Header);
	
	memset(buf, 0, sizeof(buf));
	ret = fread(buf, sizeof(char), 4, fp);
	int Flag;
	Flag = buf[0]*0x1000000 \
		   +buf[1]*0x10000 \
		   +buf[2]*0x100 \
		   +buf[3];
	MP3VBRHeader.Flag = Flag;
	printf("Flag=%d\n", Flag);
	unsigned int SumFrame;
	unsigned int SumSize;
	unsigned int Quality;
	if (Flag&0x0001) {
		// Number of frames
		ret = fread(buf, sizeof(char), 4, fp);
		SumFrame = buf[0]*0x1000000 \
				  +buf[1]*0x10000 \
				  +buf[2]*0x100 \
				  +buf[3];
		MP3VBRHeader.FrameCount = SumFrame;
		printf("SumFrame=%d\n", SumFrame);
	}
	if (Flag&(0x1<<1)) {
		// Storage file size
		ret = fread(buf, sizeof(char), 4, fp);
		SumSize = buf[0]*0x1000000 \
				 +buf[1]*0x10000 \
				 +buf[2]*0x100 \
				 +buf[3];
		MP3VBRHeader.Size = SumSize;
		printf("SumSize=%d\n", SumSize);
	}
	if (Flag&0x0004) {
		// TOC
		fseek(fp, 100, SEEK_CUR);
	}
	if (Flag&0x0008) {
		//Quality indicator
		ret = fread(buf, sizeof(char), 4, fp);
		Quality = buf[0]*0x1000000 \
				 +buf[1]*0x10000 \
				 +buf[2]*0x100 \
				 +buf[3];
		MP3VBRHeader.Quality = Quality;
		printf("Quality=%d\n", Quality);	
	}
	

	return 0;
}

const uint32_t FrameSamples[4][4] = {
	{ 0,    0,  0,    0    },   // 00 -NC
	{ 576 , 0,  576,  1152 },	// 01 -III  Layer
	{ 1152, 0,  1152, 1152 },   // 10 -II
	{ 384,  0,  384,  384  }    // 11 -I
};
//    2.5   NC   2     1   MPEG


const uint32_t SampleRate[4][3] =  {
	{ 32000, 16000, 8000  },  // 00 -2.5   MPEG
	{ 48000, 24000, 12000 },  // 01 -2
	{ 44100, 22050, 11025 },  // 10 -1
	{ 0,     0,     0     }   // 11 -NC
};
//     00     01     10  sample_rate_index                         

int getMP3Info(void)
{
	strncmp(MP3Info.FormatString, MP3LHeader.Header, 3);
	MP3Info.FormatVersion = MP3LHeader.ver;

	//总持续时间 = 总帧数 * 每帧采样数 / 采样率 (结果为秒)
	/* Fheader.version
	   00 MPEG 2.5
	   01 保留
	   10 MPEG 2
	   11 MPEG 1
	*/

	/* Fheader.layer
	   00 保留
	   01 Layer III
	   10 Layer II
	   11 Layer I
	*/
	uint32_t spf = FrameSamples[Fheader.layer][Fheader.version];
	printf("sample_rate_index=%d\n", Fheader.sample_rate_index);
	uint32_t sr = SampleRate[Fheader.sample_rate_index][Fheader.version];
	printf("FrameSamples=[%d], SampleRate=[%d]\n", spf, sr);
	MP3Info.Duration = MP3VBRHeader.FrameCount * spf;
	return 0;
}


int main(int argc, char *argv[])
{
    int ret;
	FILE *fp;
    if(argc  < 2) {
        printf("Please input file name\n");
        return -1;
    }
    char *file = argv[1];
    
    ret = access(file, F_OK); 
    if (ret) {
        printf("File [%s] does not exist\n", file);
        return -1;
    }
    printf("%s\n", file);

	fp = fopen((const char *)file, "r+");
	if(NULL == fp) {
	   printf("No such file");
	   return -1;
	}

	int size;
	size = getLabelHeader(fp);

	while(size >= 1) {
	    //printf("...size=%d\n", size);
	    size = getLabelFrameHeader(fp);
	    if (size >= 1) {
	        size = getLabelFrameContent(fp);
	    }
	}
	
	findFrameMPEGHeader(fp);
	

	fread(&Fheader, sizeof(Fheader), 1, fp);
	printf("layer=%d\n", Fheader.layer);
	printf("version=%d\n", Fheader.version);
	
	findFrameVBRHeader(fp);

	getMP3Info();
	fclose(fp);
    return 0;
}

