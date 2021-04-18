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


void pause() { system("pause"); }


//--------------------------------------------------------------
// 主要内容
//--------------------------------------------------------------
int main(int argc, char* argv[])
{
	// 看代码就懂
	if (argc <= 1)
	{
		SetColor(14);
		printf(" -------------------------------\n");
		printf("|        UDB ver 0.0.3.1        |\n");
		printf("| Copyright(C) 2020 GloomyGhost |\n");
		printf("| Copyright(C) 2020 AckieSound  |\n");
		printf(" -------------------------------\n");
		printf("|    请问是在UTAU内运行了吗？   |\n");
		printf("|    UDB只能在UTAU里调用哦OwO   |\n");
		printf(" -------------------------------\n");
		SetColor(15);
		pause();
		return -1;
	}


	// 开始时间获取
	ULONGLONG time_start = GetTickCount64();


	// 选项设置
	UDBOPTION option;
	if (UDBGetOption(argc, argv, &option) != 0) { return 0; }

	// 间距，时间，伸展
	if (UDBStep1(&option) != 0) { return 0; }

	// 过滤器、音量
	if (UDBStep2(&option) != 0) { return 0; }


	// 完成时间获取
	ULONGLONG time_end = GetTickCount64();

	// 输出变量
	SetColor(6);
	printf("参数个数：%d\n", argc);
	SetColor(15);

	if (argc != 14 && argc != 13)
	{
		SetColor(4);
		printf("输入错误，请检查相关参数\n");
		SetColor(15);
		for (int i = 0; i < argc; i++) { printf(" %s ", argv[i]); }
		pause();
	}
	else
	{
		SetColor(3);
		printf("参数正确，正在合成OwO\n");
		SetColor(15);
	}

	// 输出音频信息
	SetColor(10);
	printf("\n输入音源文件：%s", argv[1]);
	printf("\n输出音源文件：%s", argv[2]);
	printf("\n音高        ：%s", argv[3]);
	printf("\nFLAG        ：%s", argv[5]);
	printf("\nOTO         ：%s %s %s %s", argv[6], argv[7], argv[8], argv[9]);
	SetColor(15);

	// 表示时间
	SetColor(11);
	printf("\n");
	printf("\nUDB:处理完成OwO，时长:%I64ums\n", time_end - time_start);
	printf("\n");
	SetColor(15);
	return 0;
}
