#ifndef __MP3_H__
#define __MP3_H__

// Tag_header
struct Tag_header {
    char Header[3]; // It must be 'ID3'
    char ver;       // The version of ID3
    char ReVer;     // the subversion of ID3
    char Flag;
    char Size[4];
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

struct Frame_header {
	char FrameID[4];
	char Size[4];
	char Flags[2];
};


#endif
