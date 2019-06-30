#include <iostream>
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
// 实际数值范围限制
//
// 引数
//  int value	实数
//  int min		最小值
//  int max		最大值
//
// 返回值
//  范围限制后的实际数值
//
// 摘要
//  最小值以下的最小值，如果是最大值以上的话，将转换为最大值。
//--------------------------------------------------------------
double LimitDoubleValue(double value, double min, double max)
{
	if (value < min) {
		value = min;
	}
	else if (value > max) {
		value = max;
	}
	return value;
}

//--------------------------------------------------------------
// 整数范围限制
// 
// 引数
//  int value	制限対象值
//	int min		最小值
//	int max		最大值
// 
// 返回
//  范围限制后的整数值
// 
// 概要
//  最小值以下的最小值，如果是最大值以上的话，将转换为最大值。
//--------------------------------------------------------------
int LimitIntValue(int value, int min, int max)
{
	if (value < min) {
		value = min;
	}
	else if (value > max) {
		value = max;
	}
	return value;
}


//--------------------------------------------------------------
// 扩展名附加
//
// 引数
//  char *dstStr	文件名
//  int dstSize		文件名缓冲大小
//  char *extStr	扩展名
//
// 返回
//  无
//
// 概要
//  将扩展子添加到文件名。
//--------------------------------------------------------------
void AddExtStr(char* dstStr, int dstSize, char* extStr)
{
	int len = strlen(dstStr);

	//文件名的扩展名“.”转换为“_”
	for (int i = len - 1; i >= 0; i--) {
		if (dstStr[i] == '\\') {
			break;
		}
		if (dstStr[i] == '.') {
			dstStr[i] = '_';
			break;
		}
	}

	// 扩展名付加
	strcat_s(dstStr, dstSize, extStr);
}


//--------------------------------------------------------------
// 确认文件存在
//
// 引数
//  char *fileName	文件名
//
// 返回
//  有文件：1，没有文件：0
//
// 概要
//  如果存在指定的文件的话，返回1。如果文件不存在，则返回0。
//--------------------------------------------------------------
int CheckFileExist(char* fileName)
{
	int existFlg = 0;
	FILE* fp;

	fopen_s(&fp, fileName, "rb");
	if (fp != NULL) {
		fclose(fp);
		existFlg = 1;
	}

	return existFlg;
}


//--------------------------------------------------------------
// 设置控制台颜色
//
// 引数
//  unsigned short color_index 颜色名
//
// 返回
//  无
//
// 概要
//  设置控制台颜色
//--------------------------------------------------------------
void SetColor(unsigned short color_index)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_index);
}