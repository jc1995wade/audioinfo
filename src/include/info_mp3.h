#ifndef __MP3_H__
#define __MP3_H__

#define uint32_t unsigned int 

struct MP3_info {
    char FormatString[30]; //格式
    uint32_t  FormatVersion;    //格式版本
    uint32_t  Duration;         //持续时间
    uint32_t  BitRate_Mode;     //码率模式
    uint32_t  BitRate;          //码率
    uint32_t  Channels;         //声道
    uint32_t  SamplingRate;     //采样率
    uint32_t  FrameRate;        //帧率
    char CompressionMode[10];  //压缩模式
};


// Tag_header
struct Label_header {
    char Header[3]; // It must be 'ID3'
    char ver;       // The version of ID3
    char ReVer;     // the subversion of ID3
    char Flag;
    char Size[4];
};

struct Label_frame_header {
	char FrameID[4];
	char Size[4];
	char Flags[2];
};

// Tag_Trailer
struct Tag_trailer {
    char Header[3];
    char Title[30];
    char Artist[30];
    char Album[30];
    char Year[4];
    char Comment[30];
    char Cenre[1];
};

struct ID3v2_label {
    int TotalSize;
    int LabelFrameSize;
};


struct Frame_header
{
    unsigned int sync1:8; //同步信息 1
    unsigned int error_protection:1; //CRC 校验
    unsigned int layer:2; //层
    unsigned int version:2; //版本
    unsigned int sync2:3; //同步信息 2
    unsigned int extension:1; //版权
    unsigned int padding:1; //填充空白字
    unsigned int sample_rate_index:2; //采样率索引
    unsigned int bit_rate_index:4; //位率索引
    unsigned int emphasis:2; //强调方式
    unsigned int original:1; //原始媒体
    unsigned int copyright:1; //版权标志
    unsigned int mode_extension:2; //扩展模式,仅用于联合立体声
    unsigned int channel_mode:2; //声道模式
};

struct Frame_VBR_header {
	unsigned char Header[4];
	unsigned int  Flag;
	unsigned int  FrameCount;
	unsigned int  Size;
	unsigned char Tco[100];
	unsigned int  Quality;
};


#endif
