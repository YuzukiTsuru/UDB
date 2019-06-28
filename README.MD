# UDB

UTAU DEBUG ENGINE

采用AckieSound的VS变换，还原你的原始声音

并且专治以下疑难杂症：

- 不出声
- 声音不对
- 没有声音
- 格式错误
- 填词错误



### Flag：



```c++
int noFormantFlg;			// noFormantFlg					("N",默认OFF)
int genderFactor;			// 性别值[-20cent]				  ("g",-100～100,初始值0)
int pitchShift;				// pitch微调[10cent]			   ("t",-240～240,初始值0)
int pitchShift2;			// pitch微调[1cent]		       ("T",-2400～2400,初始值0)
int peekCmp;				// 峰值限制 					  ("P",0～100,初始值86)
int lowPassfilter;			// 低通道滤波器					 ("H",0～99,初始值0)
int breathiness;			// 呼吸值						   ("B",0～100,初始值50)
int brightness;				// 明亮值						   ("O",-100～100,初始值0)
int clearness;				// Clear值						("S",-100～100,初始值0)
int outputVshpFlg;			// 输出Vshp文件					  ("Z",初始值OFF)
int reGenerateFrqFlg;		// 重新生成频率表				    ("V",初始值OFF)
int disablePitchShift;		// PitchShift关闭				   ("W",基本值0)
int ShowOption;				// Debug内容					   ("D",基本值0)
```


### Reference

- [vslib](http://ackiesound.ifdef.jp/download.html)

- [vs4u](<http://ackiesound.ifdef.jp/download.html>)

  