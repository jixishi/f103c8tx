
#include "oled.h"
#include "oledfont.h"
extern I2C_HandleTypeDef hi2c1;
extern SPI_TypeDef  hspi1;
uint8_t OLED_GRAM[144][8];
#if OLED_MODE
void WriteCmd(unsigned char Command)//写命令
{
//    uint8_t pData[1];
//    pData[0] = I2C_Command;
    uint8_t *pData;
    pData = &Command;
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, pData, 1, 100);
}

void WriteDat(unsigned char Data)//写数据
{
//    uint8_t pData[1];
//    pData[0] = I2C_Data;
    uint8_t *pData;
    pData = &Data;
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, 0x40, I2C_MEMADD_SIZE_8BIT, pData, 1, 100);
}
#else
void WriteCmd(unsigned char Command)//写命令
{
//    uint8_t pData[1];
//    pData[0] = I2C_Command;
    uint8_t *pData;
    pData = &Command;
    HAL_SPI_Transmit(&hspi1,pData,sizeof(pData),10);
}

void WriteDat(unsigned char Data)//写数据
{
//    uint8_t pData[1];
//    pData[0] = I2C_Data;
    uint8_t *pData;
    pData = &Data;
    HAL_SPI_Transmit(&hspi1,pData,sizeof(pData),10);
}
#endif
void OLED_Init(void) {
    HAL_Delay(100); //这里的延时很重要
    WriteCmd(0xAE); //display off
    WriteCmd(0x20); //Set Memory Addressing Mode
    WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
    WriteCmd(0xc8); //Set COM Output Scan Direction
    WriteCmd(0x00); //---set low column address
    WriteCmd(0x10); //---set high column address
    WriteCmd(0x40); //--set start line address
    WriteCmd(0x81); //--set contrast control register
    WriteCmd(0xff); //亮度调节 0x00~0xff
    WriteCmd(0xa1); //--set segment re-map 0 to 127
    WriteCmd(0xa6); //--set normal display
    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
    WriteCmd(0x3F); //
    WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    WriteCmd(0xd3); //-set display offset
    WriteCmd(0x00); //-not offset
    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
    WriteCmd(0xf0); //--set divide ratio
    WriteCmd(0xd9); //--set pre-charge period
    WriteCmd(0x22); //
    WriteCmd(0xda); //--set com pins hardware configuration
    WriteCmd(0x12);
    WriteCmd(0xdb); //--set vcomh
    WriteCmd(0x20); //0x20,0.77xVcc
    WriteCmd(0x8d); //--set DC-DC enable
    WriteCmd(0x14); //
    WriteCmd(0xaf); //--turn on oled panel
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{
    WriteCmd(0xb0 + y);
    WriteCmd(((x & 0xf0) >> 4) | 0x10);
    WriteCmd((x & 0x0f) | 0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
    unsigned char m, n;
    for (m = 0; m < 8; m++) {
        WriteCmd(0xb0 + m);//page0-page1
        WriteCmd(0x00);    //low column start address
        WriteCmd(0x10);    //high column start address
        for (n = 0; n < 128; n++) {
            WriteDat(fill_Data);
        }
    }
}


void OLED_CLS(void)//清屏
{
    OLED_Fill(0x00);
}

void OLED_ON(void) {
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}

void OLED_OFF(void) {
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X10);  //关闭电荷泵
    WriteCmd(0XAE);  //OLED休眠
}
//反显函数
void OLED_ColorTurn(uint8_t i)
{
    if(i==0)
    {
        WriteCmd(0xA6);//正常显示
    }
    if(i==1)
    {
        WriteCmd(0xA7);//反色显示
    }
}
//更新显存到OLED
void OLED_Refresh(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
        WriteCmd(0xb0+i); //设置行起始地址
        WriteCmd(0x00);   //设置低列起始地址
        WriteCmd(0x10);   //设置高列起始地址
        for(n=0;n<128;n++)
            WriteDat(OLED_GRAM[n][i]);
    }
}
//屏幕旋转180度
void OLED_DisplayTurn(uint8_t i)
{
    if(i==0)
    {
        WriteCmd(0xC8);//正常显示
        WriteCmd(0xA1);
    }
    if(i==1)
    {
        WriteCmd(0xC0);//反转显示
        WriteCmd(0xA0);
    }
}
void OLED_DrawPoint(uint8_t x,uint8_t y)
{
    uint8_t i,m,n;
    i=y/8;
    m=y%8;
    n=1<<m;
    OLED_GRAM[x][i]|=n;
}

//清除一个点
//x:0~127
//y:0~63
void OLED_ClearPoint(uint8_t x,uint8_t y)
{
    uint8_t i,m,n;
    i=y/8;
    m=y%8;
    n=1<<m;
    OLED_GRAM[x][i]=~OLED_GRAM[x][i];
    OLED_GRAM[x][i]|=n;
    OLED_GRAM[x][i]=~OLED_GRAM[x][i];
}


//画线
//x:0~128
//y:0~64
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
    uint8_t i,k,k1,k2,y0;
    if((x1<0)||(x2>128)||(y1<0)||(y2>64)||(x1>x2)||(y1>y2))return;
    if(x1==x2)    //画竖线
    {
        for(i=0;i<(y2-y1);i++)
        {
            OLED_DrawPoint(x1,y1+i);
        }
    }
    else if(y1==y2)   //画横线
    {
        for(i=0;i<(x2-x1);i++)
        {
            OLED_DrawPoint(x1+i,y1);
        }
    }
    else      //画斜线
    {
        k1=y2-y1;
        k2=x2-x1;
        k=k1*10/k2;
        for(i=0;i<(x2-x1);i++)
        {
            OLED_DrawPoint(x1+i,y1+i*k/10);
        }
    }
}
//x,y:圆心坐标
//r:圆的半径
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r)
{
    int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)
    {
        OLED_DrawPoint(x + a, y - b);
        OLED_DrawPoint(x - a, y - b);
        OLED_DrawPoint(x - a, y + b);
        OLED_DrawPoint(x + a, y + b);

        OLED_DrawPoint(x + b, y + a);
        OLED_DrawPoint(x + b, y - a);
        OLED_DrawPoint(x - b, y - a);
        OLED_DrawPoint(x - b, y + a);

        a++;
        num = (a * a + b * b) - r*r;//计算画的点离圆心的距离
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//size:选择字体 12/16/24
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1)
{
    uint8_t i,m,temp,size2,chr1;
    uint8_t y0=y;
    size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
    chr1=chr-' ';  //计算偏移后的值
    for(i=0;i<size2;i++)
    {
        if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字体
        else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字体
        else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字体
        else return;
        for(m=0;m<8;m++)           //写入数据
        {
            if(temp&0x80)OLED_DrawPoint(x,y);
            else OLED_ClearPoint(x,y);
            temp<<=1;
            y++;
            if((y-y0)==size1)
            {
                y=y0;
                x++;
                break;
            }
        }
    }
}
//显示浮点数
//x,y:起点坐标
//size1:字体大小
//*chr:字符串起始地址
void OLED_ShowFloat(uint8_t x,uint8_t y,float num,uint8_t size1,uint8_t * chr)
{
    char temp[10];
    sprintf((char *)temp,chr,num);
    OLED_ShowString(x,y,temp,size1);
}
//显示字符串
//x,y:起点坐标  
//size1:字体大小 
//*chr:字符串起始地址 
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1)
{
    while((*chr>=' ')&&(*chr<='~'))//判断是不是非法字符!
    {
        OLED_ShowChar(x,y,*chr,size1);
        x+=size1/2;
        if(x>128-size1)  //换行
        {
            x=0;
            y+=2;
        }
        chr++;
    }
}

//m^n
uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)
    {
        result*=m;
    }
    return result;
}

//显示2个数字
//x,y :起点坐标
//len :数字的位数
//size:字体大小
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1)
{
    uint8_t t,temp;
    for(t=0;t<len;t++)
    {
        temp=(num/OLED_Pow(10,len-t-1))%10;
        if(temp==0)
        {
            OLED_ShowChar(x+(size1/2)*t,y,'0',size1);
        }
        else
        {
            OLED_ShowChar(x+(size1/2)*t,y,temp+'0',size1);
        }
    }
}

//显示汉字
//x,y:起点坐标
//num:汉字对应的序号
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1)
{
    uint8_t i,m,n=0,temp,chr1;
    uint8_t x0=x,y0=y;
    uint8_t size3=size1/8;
    while(size3--)
    {
        chr1=num*size1/8+n;
        n++;
        for(i=0;i<size1;i++)
        {
            if(size1==16)
            {temp=Hzk1[chr1][i];}//调用16*16字体
            else if(size1==24)
            {temp=Hzk2[chr1][i];}//调用24*24字体
            else if(size1==32)
            {temp=Hzk3[chr1][i];}//调用32*32字体
            else if(size1==64)
            {temp=Hzk4[chr1][i];}//调用64*64字体
            else return;

            for(m=0;m<8;m++)
            {
                if(temp&0x01)OLED_DrawPoint(x,y);
                else OLED_ClearPoint(x,y);
                temp>>=1;
                y++;
            }
            x++;
            if((x-x0)==size1)
            {x=x0;y0=y0+8;}
            y=y0;
        }
    }
}

//num 显示汉字的个数
//space 每一遍显示的间隔
void OLED_ScrollDisplay(uint8_t num,uint8_t space)
{
    uint8_t i,n,t=0,m=0,r;
    while(1)
    {
        if(m==0)
        {
            OLED_ShowChinese(128,24,t,16); //写入一个汉字保存在OLED_GRAM[][]数组中
            t++;
        }
        if(t==num)
        {
            for(r=0;r<16*space;r++)      //显示间隔
            {
                for(i=0;i<144;i++)
                {
                    for(n=0;n<8;n++)
                    {
                        OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh();
            }
            t=0;
        }
        m++;
        if(m==16){m=0;}
        for(i=0;i<144;i++)   //实现左移
        {
            for(n=0;n<8;n++)
            {
                OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

//配置写入数据的起始位置
void OLED_WR_BP(uint8_t x,uint8_t y)
{
    WriteCmd(0xb0+y);//设置行起始地址
    WriteCmd(((x&0xf0)>>4)|0x10);
    WriteCmd((x&0x0f)|0x01);
}

//x0,y0：起点坐标
//x1,y1：终点坐标
//BMP[]：要写入的图片数组
void OLED_ShowPicture(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[])
{
    uint32_t j=0;
    uint8_t x=0,y=0;
    if(y%8==0)y=0;
    else y+=1;
    for(y=y0;y<y1;y++)
    {
        OLED_WR_BP(x0,y);
        for(x=x0;x<x1;x++)
        {
            WriteDat(BMP[j]);
            j++;
        }
    }
}