//--------------------------------------------------------------
// Pit变换
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
// Pit变换
//
// 引数
//  UDBOPTION *option	
//
// 返回
//  0:成功 1:失败
//
// 概要
//  (inputWaveFile→tmpWaveData)
//--------------------------------------------------------------
int UDBStep1(UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;

	// 创建项目
	HVSPRJ hVsprj = UDBLoadProject(option);
	if (hVsprj == NULL) { return 1; }

	// 时间伸展设定
	UDBSetTimeStretchPrm(hVsprj, option);

	// 间距设置
	UDBSetPitPrm(hVsprj, option);

	// 语音合成方式设置
	if (option->disablePitchShift == -2)
	{
		// 设定为和音模式
		VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
		vsItemInfo.synthMode = SYNTHMODE_P;
		VslibSetItemInfo(hVsprj, 0, &vsItemInfo);
	}
	else
	{
		if (option->noFormantFlg != 0)
		{
			// 如果没有调整pit，就设置成单音模式
			VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
			vsItemInfo.synthMode = SYNTHMODE_M;
			VslibSetItemInfo(hVsprj, 0, &vsItemInfo);
		}
		else
		{
			// 如果有pit校正的话，就设定为单音pit校正模式
			VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
			vsItemInfo.synthMode = SYNTHMODE_MF;
			VslibSetItemInfo(hVsprj, 0, &vsItemInfo);
		}
	}

	// EQ设置
	int eq1[15] = {0, 0, 0, 0, 0, 0, 0, 20, 60, 100, 120, 120, 80, 0, 0};
	int eq2[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 120, 120};
	VslibSetEQGain(hVsprj, 0, 0, eq1);
	VslibSetEQGain(hVsprj, 0, 1, eq2);

	// 取得控制分数
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	int nCtrlPnt = vsItemInfo.ctrlPntNum;

	// 控制点参数设置
	for (int i = 0; i < nCtrlPnt; i++)
	{
		// 取得控制分数
		VSCPINFOEX vsCpInfo;
		VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);

		// 参数设置
		vsCpInfo.formant = -20 * option->genderFactor;

		// 呼吸设置
		vsCpInfo.breathiness = (option->breathiness - 50) * 200;

		// EQ呼吸设置
		vsCpInfo.eq1 = option->brightness * 100;

		// EQ2明亮度
		vsCpInfo.eq2 = option->clearness * 100;

		// 控制点参数设置
		VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
	}

	// 采样频率、样本数取得
	VslibGetProjectInfo(hVsprj, &vsPrjInfo);
	option->tmpSmpFreq = vsPrjInfo.sampFreq;
	VslibGetMixSample(hVsprj, &option->tmpSample);

	// 获取WAVE数据
	option->tmpWaveData = (short*)calloc(option->tmpSample, sizeof(short));
	VslibGetMixData(hVsprj, option->tmpWaveData, 16, 1, 0, option->tmpSample);

	// 项目文件输出
	if (option->outputVshpFlg == 12345) { VslibSaveProject(hVsprj, option->gFile); }

	// 删除项目
	VslibDeleteProject(hVsprj);

	return 0;
}


//--------------------------------------------------------------
// 创建项目
//
// 引数
//  UDBOPTION *option	选项
//
// 返回
//  项目指向
//
// 概要
//  读取独自的频率表，并返回项目方向。
//  如果没有单独的频率表的情况下制作。
//  如果失败了返回NULL。
//--------------------------------------------------------------
HVSPRJ UDBLoadProject(UDBOPTION* option)
{
	int sta;

	// WAVE文件存在检查
	if (CheckFileExist(option->inputWaveFile) == 0)
	{
		SetColor(12);
		printf("UDB:哇！我没有在音源里找到这个发音，我没办法生成了啦QAQ,缺少的发音是:%s\n", option->inputWaveFile);
		SetColor(15);
		if (option->ShowOption == 1) { system("pause"); }
		return NULL;
	}

	// 标准频率表存在检查
	int existFrqFlg = CheckFileExist(option->frqFile);

	// 独立频率表存在检查
	int existVsfrqFlg = CheckFileExist(option->GfrqFile);

	// 创建项目
	HVSPRJ hVsprj = NULL;

	if (option->reGenerateFrqFlg != 0) { existVsfrqFlg = 0; }
	else
	{
		if (existVsfrqFlg != 0)
		{
			// 读取自己的频率表
			sta = VslibOpenProject(&hVsprj, option->GfrqFile);
			if (sta != VSERR_NOERR)
			{
				SetColor(12);
				printf("UDB:我在读取我自己的Gfrq的时候发现好像频率表错误了，Master检查下吧！或者删了频率表我重新生成一个OwO\n");
				SetColor(15);
				if (option->ShowOption == 1) { system("pause"); }
				existVsfrqFlg = 0;
			}
		}
	}

	if (existVsfrqFlg == 0)
	{
		// 创建独自的频率表（Gfrq）
		SetColor(11);
		printf("UDB:创建UDB自己用的一个小频率表OwO\n");
		SetColor(15);

		// 创建项目
		sta = VslibCreateProject(&hVsprj);
		if (sta != VSERR_NOERR)
		{
			SetColor(12);
			printf("UDB:哇，居然初始化错误，我也不知道什么原因，好难受\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
			return NULL;
		}

		// 输入文件到项目中
		int itemNum;
		sta = VslibAddItemEx(hVsprj, option->inputWaveFile, &itemNum, 36, 48, 0);
		if (sta != VSERR_NOERR)
		{
			SetColor(12);
			printf("UDB:文件输入错误了QAQ\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
			return NULL;
		}

		// 使用标准频率表来设置频率
		if (existFrqFlg != 0) { UDBLoadFrqFile(hVsprj, option); }

		// 独立频率表保存
		sta = VslibSaveProject(hVsprj, option->GfrqFile);
		if (sta != VSERR_NOERR)
		{
			SetColor(12);
			printf("UDB:哇。。保存频率表失败惹，Master帮我检查下呗QAQ(可能是文件夹没有写入权限导致的）\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
		}
		else
		{
			SetColor(11);
			printf("UDB:耶！保存频率表成功了OwO\n");
			SetColor(15);
		}
	}

	return hVsprj;
}


//--------------------------------------------------------------
// 使用标准频率表来设置频率
//
// 引数
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// 返回
//  0:成功 1:失败
//
// 概要
//  利用标准频率表设置间距解析值。
//--------------------------------------------------------------
int UDBLoadFrqFile(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;
	int retVal = 0;

	FILE* fpFrq;
	fopen_s(&fpFrq, option->frqFile, "rb");

	if (fpFrq != NULL)
	{
		// 数据间隔[样本]
		int itvSmp = 256;
		fseek(fpFrq, 8, SEEK_SET);
		fread(&itvSmp, sizeof(int), 1, fpFrq);

		// 键频率
		double keyFreq = 440.0;
		fseek(fpFrq, 12, SEEK_SET);
		fread(&keyFreq, sizeof(double), 1, fpFrq);

		// 数据数
		int nData = 0;
		fseek(fpFrq, 36, SEEK_SET);
		fread(&nData, sizeof(int), 1, fpFrq);

		// 频率数据
		FREQDATA* freqData = (FREQDATA*)calloc(nData, sizeof(FREQDATA));
		if (freqData == nullptr)
		{
			SetColor(12);
			printf("读取频率表错误！终止。QAQ\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
			return -1;
		}
		else
		{
			fseek(fpFrq, 40, SEEK_SET);
			fread(freqData, sizeof(FREQDATA), nData, fpFrq);
		}

		// 取得控制分数
		VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
		int nCtrlPnt = vsItemInfo.ctrlPntNum;

		// 抽样频率取得
		VslibGetProjectInfo(hVsprj, &vsPrjInfo);
		int sampFreq = vsPrjInfo.sampFreq;
		double cpSec = 1.0 / vsItemInfo.ctrlPntPs; // 控制点的秒数

		// 频率设置
		for (int i = 0; i < nCtrlPnt; i++)
		{
			// 在标准频率表上计算点
			double smp = i * cpSec * sampFreq;
			int pnt = (int)(smp / itvSmp);

			// 频率设置
			double freq = 0.0;
			if ((pnt >= 0) && (pnt < nData)) { freq = freqData[pnt].freq; }

			if (freq > 55.0)
			{
				// 间距
				// 设定编辑前间距和无间距的标志
				VSCPINFOEX vsCpInfo;
				VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
				vsCpInfo.pitOrg = (int)(log(freq / 440.0) / log(2.0) * 1200.0) + 6900;
				vsCpInfo.pitFlgEdit = 1;
				VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			}
			else
			{
				// 无间距
				// 设定编辑前间距和无间距的标志
				VSCPINFOEX vsCpInfo;
				VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
				vsCpInfo.pitOrg = (int)(log(keyFreq / 440.0) / log(2.0) * 1200.0) + 6900;
				vsCpInfo.pitFlgEdit = 0;
				VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			}
		}

		// 频率数据释放
		free(freqData);
		freqData = NULL;

		// 关闭文件
		fclose(fpFrq);
	}
	else { retVal = 1; }

	return retVal;
}


//--------------------------------------------------------------
// 时间伸展设定
//
// 引数
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// 返回
//  0:成功
//
// 概要
//  时间伸展设定。
//--------------------------------------------------------------
int UDBSetTimeStretchPrm(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;

	// 抽样频率取得
	VslibGetProjectInfo(hVsprj, &vsPrjInfo);
	int sampFreq = vsPrjInfo.sampFreq;

	// 获取项目信息
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	double orgSmp = vsItemInfo.sampleOrg; // 源样本数
	double offsetSmp = option->offset * 0.001 * sampFreq; // 偏移的样本数
	double fixedSmp = option->fixedLength * 0.001 * sampFreq; // 子音部的样本数
	double endBlankSmp = option->endBlank * 0.001 * sampFreq; // blank的样本数
	double reqSmp = option->reqLength * 0.001 * sampFreq; // 要求长度的样本数

	// blank以负（从偏移的相对值）转换为从结尾处的空白
	if (endBlankSmp < 0) { endBlankSmp = orgSmp - offsetSmp + endBlankSmp; }

	// 子音部的伸展倍率
	double fixStretch = pow(2.0, 1.0 - option->fixSpeed / 100.0);

	// 添加定时控制点
	if (reqSmp >= fixedSmp * fixStretch)
	{
		// 要求长超过子音部
		VslibSetTimeCtrlPnt(hVsprj, 0, 1, (int)orgSmp, (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)(orgSmp - endBlankSmp), (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)(offsetSmp + fixedSmp), (int)(fixedSmp * fixStretch));
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)offsetSmp, 0);
	}
	else
	{
		// 要求长未满子音部
		VslibSetTimeCtrlPnt(hVsprj, 0, 1, (int)orgSmp, (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)(offsetSmp + reqSmp / fixStretch), (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)offsetSmp, 0);
	}

	return 0;
}


//--------------------------------------------------------------
// 母音部分的间距平均值计算
//
// 引数
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// 返回
//  0:成功
//
// 概要
//  计算母音部分的间距平均值计算。
//--------------------------------------------------------------
int UDBCalcAveragePit(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSITEMINFO vsItemInfo;

	// 取得控制分数
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	int nCtrlPnt = vsItemInfo.ctrlPntNum;

	// 计算控制点的秒数
	double ctrpntSec = 1.0 / vsItemInfo.ctrlPntPs;

	// 计算母音区间的控制点号码
	int startPnt = (int)((option->offset + option->fixedLength) * 0.001 / ctrpntSec);
	int endPnt = (int)(nCtrlPnt - option->endBlank * 0.001 / ctrpntSec);
	startPnt = LimitIntValue(startPnt, 0, nCtrlPnt);
	endPnt = LimitIntValue(endPnt, 0, nCtrlPnt);

	// 母音的间距平均值计算
	int pitSum = 0;
	int count = 0;
	for (int i = startPnt; i < endPnt; i++)
	{
		VSCPINFOEX vsCpInfo;
		VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		if (vsCpInfo.pitFlgEdit == 1)
		{
			// 只有间距的部分计数
			pitSum += vsCpInfo.pitOrg;
			count++;
		}
	}
	int pitAverage = 6000;
	if (count != 0) { pitAverage = pitSum / count; }

	// 母音的间距平均值计算（第二次）
	pitSum = 0;
	count = 0;
	for (int i = startPnt; i < endPnt; i++)
	{
		VSCPINFOEX vsCpInfo;
		VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		if (vsCpInfo.pitFlgEdit == 1)
		{
			// 只有间距的部分计数
			if (abs(vsCpInfo.pitOrg - pitAverage) < 200)
			{
				pitSum += vsCpInfo.pitOrg;
				count++;
			}
		}
	}
	pitAverage = 6000;
	if (count != 0) { pitAverage = pitSum / count; }

	return pitAverage;
}


//--------------------------------------------------------------
// 间距设置
//
// 引数
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// 返回
//  0:成功
//
// 概要
//  设定间距参数。
//--------------------------------------------------------------
int UDBSetPitPrm(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;

	// 获取抽样频率
	VslibGetProjectInfo(hVsprj, &vsPrjInfo);
	int sampFreq = vsPrjInfo.sampFreq;

	// 取得控制分数
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	int nCtrlPnt = vsItemInfo.ctrlPntNum;

	// 计算控制点的秒数
	double ctrpntSec = 1.0 / vsItemInfo.ctrlPntPs;

	// 计算间距平均值
	int pitAverage = UDBCalcAveragePit(hVsprj, option);

	// 计算整体的间距
	int globalShift = 10 * option->pitchShift + option->pitchShift2;

	// 计算间距[秒单位]的间距
	double pbitvSec = 256.0 / sampFreq;
	if (option->tempo > 0.0)
	{
		pbitvSec = 60.0 / option->tempo / 96; // 4分音符1/96秒
	}

	if ((option->disablePitchShift == -1) || (option->disablePitchShift == -2))
	{
		for (int i = 0; i < nCtrlPnt; i++)
		{
			// 间距设置
			VSCPINFOEX vsCpInfo;
			VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			vsCpInfo.pitEdit = vsCpInfo.pitOrg;
			VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		}
	}
	else
	{
		// 编辑后的间距参数设置
		for (int i = 0; i < nCtrlPnt; i++)
		{
			// 编辑后的位置（秒单位）计算
			double edtSec;
			VslibGetStretchEditSec(hVsprj, 0, i * ctrpntSec, &edtSec);

			// 间距计算
			int bendPnt = (int)(edtSec / pbitvSec);
			int bend = 0;
			if ((bendPnt >= 0) && (bendPnt < option->nPitchBend)) { bend = option->pitchBend[bendPnt]; }

			// 间距设置
			VSCPINFOEX vsCpInfo;
			VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			int modPit = (int)((vsCpInfo.pitOrg - pitAverage) * option->modulation / 100.0);
			vsCpInfo.pitEdit = option->noteNum * 100 + modPit + bend + globalShift;
			VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		}


		// 编辑后的间距参数（UDB）设置
		// 输出样本数取得
		int mixSample;
		VslibGetMixSample(hVsprj, &mixSample);

		// 计算间距的数据数
		double mixSec = (double)mixSample / sampFreq;
		int nPitchBend = (int)(mixSec / pbitvSec) + 1;

		for (int i = 0; i < nPitchBend; i++)
		{
			// 计算对应于间距数据位置的控制点号码
			double orgSec;
			double edtSec = LimitDoubleValue(i * pbitvSec, 0.0, mixSec);
			VslibGetStretchOrgSec(hVsprj, 0, edtSec, &orgSec);
			int pnt = LimitIntValue((int)(orgSec / ctrpntSec), 0, nCtrlPnt - 1);

			// 取得编辑前的间距
			VSCPINFOEX vsCpInfo;
			VslibGetCtrlPntInfoEx(hVsprj, 0, pnt, &vsCpInfo);
			int pitOrg = vsCpInfo.pitOrg;

			// 计算编辑后的间距
			int modPit = (int)((pitOrg - pitAverage) * option->modulation / 100.0);
			int pitEdt = option->noteNum * 100 + modPit + globalShift + option->pitchBend[i];

			// 设置间距变更
			option->pitchBend[i] = pitEdt - pitOrg;
		}

		// 设定间距数据
		VslibSetPitchArray(hVsprj, 0, option->pitchBend, nPitchBend, pbitvSec * sampFreq);
	}

	return 0;
}
