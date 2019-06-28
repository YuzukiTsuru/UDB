#pragma once

#ifndef _UDB_H_
#define _UDB_H_


#define M_PI			(3.14159265358979323846)

#define M_PI2           (6.28)

#define UDB_MAX_PATH	(260)

#define UDB_PBSIZE		(65536)


typedef struct {
	char inputWaveFile[UDB_MAX_PATH];
	char outputWaveFile[UDB_MAX_PATH];
	char frqFile[UDB_MAX_PATH];
	char GfrqFile[UDB_MAX_PATH];
	char gFile[UDB_MAX_PATH];

	int noteNum;
	double offset;
	double reqLength;
	double fixedLength;
	double endBlank;
	double fixSpeed;

	double volume;
	double modulation;
	double tempo;
	int nPitchBend;
	int pitchBend[UDB_PBSIZE];


	// Flag
	int noFormantFlg;			// noFormantFlg					("N",默认OFF)
	int genderFactor;			// 性别值[-20cent]				("g",-100～100,初始值0)
	int pitchShift;				// pitch微调[10cent]			("t",-240～240,初始值0)
	int pitchShift2;			// pitch微调[1cent]				("T",-2400～2400,初始值0)
	int peekCmp;				// 峰值限制 					("P",0～100,初始值86)
	int lowPassfilter;			// 低通道滤波器					("H",0～99,初始值0)
	int breathiness;			// 呼吸值						("B",0～100,初始值50)
	int brightness;				// 明亮值						("O",-100～100,初始值0)
	int clearness;				// Clear值						("S",-100～100,初始值0)
	int outputVshpFlg;			// 输出Vshp文件					("Z",初始值OFF)
	int reGenerateFrqFlg;		// 重新生成频率表				("V",初始值OFF)
	int disablePitchShift;		// PitchShift关闭				("W",基本值0)
	int ShowOption;				// 文件内容输出					("D",基本值0)

	short* tmpWaveData;
	int tmpSample;
	int tmpSmpFreq;
} UDBOPTION;


typedef struct {
	double freq;
	double dyn;
} FREQDATA;


// Common
double LimitDoubleValue(double value, double min, double max);
int LimitIntValue(int value, int min, int max);
void AddExtStr(char* dstStr, int dstSize, char* extStr);
int CheckFileExist(char* fileName);

// Option
int UDBGetOption(int argc, char* argv[], UDBOPTION* option);
int UDBCnvNoteNum(char* nnStr);
int UDBSetFlgOption(char* flgStr, UDBOPTION* option);
int UDBDecodeBase64(char* str);
int UDBSetPitchBendOption(char* argv[], UDBOPTION* option);
int ShowBPM(UDBOPTION* option);

// Pitch
int UDBStep1(UDBOPTION* option);
HVSPRJ UDBLoadProject(UDBOPTION* option);
int UDBSetTimeStretchPrm(HVSPRJ hVsprj, UDBOPTION* option);
int UDBLoadFrqFile(HVSPRJ hVsprj, UDBOPTION* option);
int UDBCalcAveragePit(HVSPRJ hVsprj, UDBOPTION* option);
int UDBSetPitPrm(HVSPRJ hVsprj, UDBOPTION* option);

// Vol
int UDBStep2(UDBOPTION* option);
int UDBLowPassFilter(UDBOPTION* option);
int UDBVolume(UDBOPTION* option);
int UDBOutputWaveFile(UDBOPTION* option);

#endif 

