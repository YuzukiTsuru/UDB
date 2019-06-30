
//--------------------------------------------------------------
// 设置文件
//--------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <windows.h>

#include "vslib.h"
#include "udb.h"

//--------------------------------------------------------------
//选项设置
//
//引数
//int argc引数的数量
//charvargv[]引数
//设置的收藏选项
//
// 返回值
//    0 :成功1：失败
//
// 
//读取命令行参数，设置选项。
//--------------------------------------------------------------
int UDBGetOption(int argc, char* argv[], UDBOPTION* option)
{

	// 选项初始化
	memset(option, 0, sizeof(UDBOPTION));
	option->peekCmp = 86;
	option->breathiness = 50;

	// 1.输入文件
	strcpy_s(option->inputWaveFile, UDB_MAX_PATH, argv[1]);

	// 2.输出文件
	strcpy_s(option->outputWaveFile, UDB_MAX_PATH, argv[2]);

	// 3.音阶名
	option->noteNum = UDBCnvNoteNum(argv[3]);

	// 4.来自原声长的变化部分（子音速度）
	option->fixSpeed = LimitDoubleValue(atof(argv[4]), 0.0, 400.0);

	// 5.标志
	UDBSetFlgOption(argv[5], option);

	// 6.偏移
	option->offset = atof(argv[6]);

	// 7.要求长
	option->reqLength = atof(argv[7]);

	// 8.gFile
	option->fixedLength = atof(argv[8]);

	// 9.空白
	option->endBlank = atof(argv[9]);

	// 10.音量
	option->volume = LimitDoubleValue(atof(argv[10]), 0.0, 200.0);

	// 11.旁白
	option->modulation = LimitDoubleValue(atof(argv[11]), -400.0, 400.0);

	// 12～13.间距
	if (argc >= 13) {
		UDBSetPitchBendOption(argv, option);
	}

	// 独立频率表文件名设置
	strcpy_s(option->GfrqFile, UDB_MAX_PATH, option->inputWaveFile);
	AddExtStr(option->GfrqFile, UDB_MAX_PATH, (char*)".Gfrq");

	// 标准频率表文件名设置
	strcpy_s(option->frqFile, UDB_MAX_PATH, option->inputWaveFile);
	AddExtStr(option->frqFile, UDB_MAX_PATH, (char*)".frq");

	// 项目文件名设置
	strcpy_s(option->gFile, UDB_MAX_PATH, option->outputWaveFile);
	AddExtStr(option->gFile, UDB_MAX_PATH, (char*)".vshp");

	return 0;
}


//--------------------------------------------------------------
// 把音阶名字符串转换成号码
//
// 引数
//  char *nnStr		音阶名文字列(C0～B9)
//
// 返回
//  (C4=60一个半音単位)
//
// 概要
//  把音阶名字符串转换成号码。
//--------------------------------------------------------------
int UDBCnvNoteNum(char* nnStr)
{
	int noteNum = 0;		// 号码
	int octStrOffset = 1;	// 字符INDEX

							// 检查音阶名文字长度
	int length = strlen(nnStr);
	if (length < 2) {
		return 60;
	}

	// 把音阶名转换成号码
	switch (nnStr[0]) {
	case 'C':
		noteNum = 0;
		break;
	case 'D':
		noteNum = 2;
		break;
	case 'E':
		noteNum = 4;
		break;
	case 'F':
		noteNum = 5;
		break;
	case 'G':
		noteNum = 7;
		break;
	case 'A':
		noteNum = 9;
		break;
	case 'B':
		noteNum = 11;
		break;
	default:
		noteNum = 0;
		break;
	}

	// #加上半个音
	if (nnStr[1] == '#') {
		noteNum++;
		octStrOffset = 2;
	}

	// 转换轴
	if (length > octStrOffset) {
		int oct = nnStr[octStrOffset] - '0';
		if (oct < 0) {
			oct = 0;
		}
		else if (oct > 10) {
			oct = 10;
		}
		noteNum += (oct + 1) * 12;
	}

	return noteNum;
}


//--------------------------------------------------------------
// 标志文字列选项设置
//
// 引数
//  char *flgStr	标志文字列
//
// 返回值
//  0:成功 1:失敗
//
// 概要
//  标志文字列选项设置。
//--------------------------------------------------------------
int UDBSetFlgOption(char* flgStr, UDBOPTION * option)
{
	int length = strlen(flgStr);

	// 设定
	int offset = 0;	// 读出位置
	for (;;) {
		//判断结束
		if (offset == length) {
			break;
		}

		// 标志类别读取
		char flg = flgStr[offset];
		offset++;

		// 计算标志值的字符串长度
		int valueLength = 0;
		for (;;) {
			// 到达终点
			if (offset + valueLength == length) {
				break;
			}
			// 到下一个标志（字母）
			if (isalpha(flgStr[offset + valueLength])) {
				break;
			}
			valueLength++;
		}

		// 将标志值转换为数字
		int valueData = 0;
		if ((valueLength > 0) && (valueLength < 32)) {
			char tmpStr[32];
			memcpy_s(tmpStr, 32, flgStr + offset, valueLength);
			tmpStr[valueLength] = 0;
			valueData = atoi(tmpStr);
			offset += valueLength;
		}

		// 标志设定
		switch (flg) {
		case 'N':
			option->noFormantFlg = 1;
			break;
		case 'g':
			option->genderFactor = LimitIntValue(valueData, -100, 100);
			break;
		case 't':
			option->pitchShift = LimitIntValue(valueData, -240, 240);
			break;
		case 'T':
			option->pitchShift2 = LimitIntValue(valueData, -2400, 2400);
			break;
		case 'P':
			option->peekCmp = LimitIntValue(valueData, 0, 100);
			break;
		case 'H':
			option->lowPassfilter = LimitIntValue(valueData, 0, 99);
			break;
		case 'B':
			option->breathiness = LimitIntValue(valueData, 0, 100);
			break;
		case 'O':
			option->brightness = LimitIntValue(valueData, -100, 100);
			break;
		case 'S':
			option->clearness = LimitIntValue(valueData, -100, 100);
			break;
		case 'Z':
			option->outputVshpFlg = valueData;
			break;
		case 'V':
			option->reGenerateFrqFlg = 1;
			break;
		case 'W':
			option->disablePitchShift = valueData;
			break;
		case 'D':
			option->ShowOption = 1;
			break;
		}
	}
	return 0;
}


//--------------------------------------------------------------
// Base64解码
//
// 引数
//  char *str	字符串
//
// 返回
//  代码值
//
// 概要
//  将2位数的BASE64字列解码，返回- 2048～2047的值。
//--------------------------------------------------------------
int UDBDecodeBase64(char* str)
{
	int value = 0;

	for (int i = 0; i < 2; i++) {
		int c = str[i];
		int v = 0;

		if ((c >= 'A') && (c <= 'Z')) {
			v = c - 'A';
		}
		else if ((c >= 'a') && (c <= 'z')) {
			v = c - 'a' + 26;
		}
		else if ((c >= '0') && (c <= '9')) {
			v = c - '0' + 52;
		}
		else if (c == '+') {
			v = 62;
		}
		else if (c == '/') {
			v = 63;
		}

		value = value * 64 + v;
	}

	if (value >= 2048) {
		value -= 4096;
	}

	return value;
}


//--------------------------------------------------------------
// 间距控制选项设置
//
// 引数
//  int		argc		引数值
//  char	*argv[]		引数
//  UDBOPTION *option	选项
//
// 返回
//  0:成功 1:失败
//
// 概要
//  读取命令线引数，设置间距选项。
//--------------------------------------------------------------
int UDBSetPitchBendOption(char* argv[], UDBOPTION * option)
{
	char* pitchBendStr = argv[12];
	option->tempo = 0.0;

	// 节奏读取
	if (pitchBendStr[0] == '!') {
		option->tempo = atof(pitchBendStr + 1);
		pitchBendStr = argv[13];
	}

	// 读取间距的数据
	int length = strlen(pitchBendStr);	// 文字长度
	int nData = 0;		// 数据数
	int offset = 0;		// 读取位置
	int lastData = 0;	// 上次数据

	for (;;) {
		// 结束判断
		if (offset >= length) {
			break;
		}

		if (pitchBendStr[offset] == '#') {
			// 计算文字长
			int i = offset + 1;
			int offset2 = i;
			for (;;) {
				if (i == length) {
					break;
				}
				if (pitchBendStr[i] == '#') {
					offset2 = i;
					break;
				}
				i++;
			}
			int repNumLength = offset2 - offset - 1;

			// 算出，数据设置
			int repNum = 0;
			if (repNumLength < 8) {
				char tmp[8];
				memcpy_s(tmp, 8, pitchBendStr + offset + 1, repNumLength);
				tmp[repNumLength] = NULL;
				repNum = atoi(tmp);
				for (int i = 0; i < repNum; i++) {
					if (nData + i < UDB_PBSIZE) {
						option->pitchBend[nData + i] = lastData;
					}
					else {
						break;
					}
				}
			}

			nData += repNum;
			offset = offset2 + 1;
		}
		else {
			// 直接指定
			if (length >= offset + 2) {
				lastData = UDBDecodeBase64(pitchBendStr + offset);
				if (nData < UDB_PBSIZE) {
					option->pitchBend[nData] = lastData;
				}
				nData++;
				offset += 2;
			}
			else {
				break;
			}
		}
	}


	// 内容表示（debug）
	if (option->ShowOption == 1) {
		SetColor(9);
		printf("\n\n");
		printf("输入文件:%s\n", option->inputWaveFile);
		printf("输出文件:%s\n", option->outputWaveFile);
		printf("输出音阶:%d\n", option->noteNum);
		printf("offset:%f\n", option->offset);
		printf("要求长:%f\n", option->reqLength);
		printf("子音:%f\n", option->fixedLength);
		printf("空白:%f\n", option->endBlank);
		printf("子音速度:%f\n", option->fixSpeed);
		printf("音量:%f\n", option->volume);
		printf("调幅度:%f\n", option->modulation);
		printf("\n");
		printf("\n");
		SetColor(15);
	}

	// 数据数设置
	if (nData > UDB_PBSIZE) {
		nData = UDB_PBSIZE;
	}
	option->nPitchBend = nData;

	return 0;
}
//--------------------------------------------------------------
// 显示BPM
//
// 引数
//  UDBOPTION *option	选项
//
// 返回
//  0
//
// 概要
//  读取命令线引数，输出BPM
//--------------------------------------------------------------
int ShowBPM(UDBOPTION * option) {
	printf("\nBPM:%f\n", option->fixSpeed);
	return 0;
}