#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "info_mp3.h"

struct Tag_header Mp3Header;
struct Tag_trailer Mp3Trailer;
struct Frame_header MP3Frameh;

void tag_getFrame(FILE *fp);


void tagGetTrailer(FILE *fp)
{
	
	// Get Header
	fseek(fp,-128,SEEK_END);
	fread(Mp3Trailer.Header,3,1,fp);
	// Get Title
	fseek(fp,-125,SEEK_END);
	fread(Mp3Trailer.Title,30,1,fp);
	// Get Artist
	fseek(fp,-95,SEEK_END);
	fread(Mp3Trailer.Artist,30,1,fp);
	// Get Album
	fseek(fp,-65,SEEK_END);
	fread(Mp3Trailer.Album,30,1,fp);
	// Get Year
	fseek(fp,-35,SEEK_END);
	fread(Mp3Trailer.Year,4,1,fp);
	// Get Comment
	fseek(fp,-31,SEEK_END);
	fread(Mp3Trailer.Comment,30,1,fp);
	// Get Cenre
	fseek(fp,-1,SEEK_END);
	fread(Mp3Trailer.Cenre,1,1,fp);
}
void tagGetHeader(FILE *fp)
{
	int ret;
	printf("get header\n");
	char buf[20];
	memset(buf, 0, sizeof(buf));
	fseek(fp,0,SEEK_SET);
	ret = fread(buf, sizeof(char), 10, fp);
	memcpy(&Mp3Header, buf, sizeof(Mp3Header));
	printf("header=%.*s\n", 3, Mp3Header.Header);
	printf("ver=%d\n", (int)Mp3Header.ver);
	printf("ReVer=%c\n", Mp3Header.ReVer);
	printf("Flag=%d\n", (int)Mp3Header.Flag);
	
	int total_size;
	total_size = (Mp3Header.Size[0]&0x7F)*0x200000 \
				+(Mp3Header.Size[1]&0x7F)*0x4000 \
				+(Mp3Header.Size[2]&0x7F)*0x80 \
				+(Mp3Header.Size[3]&0x7F);
	printf("Size=%d\n", total_size);

	tag_getFrame(fp);
	
}

void tag_getFrame(FILE *fp)
{
	char buf[20];
	memset(buf, 0 , sizeof(buf));
	fread(buf, sizeof(char), 10, fp);
	memset(&MP3Frameh, 0, sizeof(MP3Frameh));
	memcpy(&MP3Frameh, buf, sizeof(MP3Frameh));
	printf("FrameID=%.*s\n", 4, MP3Frameh.FrameID);
	int FSize;
	FSize = MP3Frameh.Size[0]*0x1000000 \
		   +MP3Frameh.Size[1]*0x10000 \
		   +MP3Frameh.Size[2]*0x100 \
		   +MP3Frameh.Size[3];
	printf("Size=%d\n", FSize);
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

	memset(buf, 0 , sizeof(buf));
	fread(buf, sizeof(char), 11, fp);
	printf("buf=[%s]\n", buf+1);
	
	memset(buf, 0 , sizeof(buf));
	fread(buf, sizeof(char), 4, fp);
	printf("buf=[%.*s]\n", 4, buf);


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
	if(fp == NULL) {
	   printf("No such file");
	}
	else{
		memset(&Mp3Header, 0, sizeof(Mp3Header));
		memset(&Mp3Trailer, 0, sizeof(Mp3Trailer));
		tagGetHeader(fp);
		//tagGetTrailer(fp);
	}

	//printf("\n%s\n",Mp3Trailer.Title);
	//printf("%s\n",Mp3Trailer.Artist);
	//printf("%s\n",Mp3Trailer.Album);
	//printf("%s\n",Mp3Trailer.Year);
	//printf("%s\n",Mp3Trailer.Comment);
	fclose(fp);
    return 0;
}

