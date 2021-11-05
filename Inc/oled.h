#define OLED_ADDRESS 0x78
#define OLED_MODE 1
#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include <stdio.h>
void OLED_Init(void);//初始化
void WriteCmd(unsigned char I2C_Command);//写命令
void WriteDat(unsigned char I2C_Data);//写数据
void OLED_SetPos(unsigned char x, unsigned char y);//设置起始点坐标 BMP图使用
void OLED_Fill(unsigned char fill_Data);//全屏填充
void OLED_CLS(void);//清屏
void OLED_ON(void);//OLED唤醒
void OLED_OFF(void);//OLED休眠
uint32_t OLED_Pow(uint8_t m,uint8_t n);//m^n
void OLED_ColorTurn(uint8_t i);//反显函数
void OLED_DisplayTurn(uint8_t i);//屏幕旋转180度
void OLED_Refresh(void);//更新显存到OLED
//x:0~127
//y:0~63
void OLED_DrawPoint(uint8_t x,uint8_t y);//画点存于缓冲区
//x:0~127
//y:0~63
void OLED_ClearPoint(uint8_t x,uint8_t y);//从缓冲区清除一个点

//x:0~128
//y:0~64
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);//画线
//x,y:圆心坐标
//r:圆的半径
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r);
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//size:选择字体 12/16/24
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1);
//显示浮点数
//x,y:起点坐标
//size1:字体大小
//*chr:字符串格式化形式

void OLED_ShowFloat(uint8_t x,uint8_t y,float num,uint8_t size1,uint8_t * chr);
//显示字符串
//x,y:起点坐标
//size1:字体大小
//*chr:字符串起始地址
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1);
//显示2个数字
//x,y :起点坐标
//len :数字的位数
//size:字体大小
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1);
//显示汉字
//x,y:起点坐标
//num:汉字对应的序号
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1);
//num 显示汉字的个数
//space 每一遍显示的间隔
void OLED_ScrollDisplay(uint8_t num,uint8_t space);
//配置写入数据的起始位置
void OLED_WR_BP(uint8_t x,uint8_t y);
//x0,y0：起点坐标
//x1,y1：终点坐标
//BMP[]：要写入的图片数组
void OLED_ShowPicture(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[]);