/*NAME
open and possibly create a file or device
SYNOPSIS
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
       int open(const char *pathname, int flags);
       int open(const char *pathname, int flags, mode_t mode);
	   file  descriptor:文件描述符
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>	//for open
#include <sys/mman.h>
#include <linux/input.h>
#include<termios.h>
#include<unistd.h>	//for close
#include "hanziku.h"  //提取的字模存放在"hanziku.h"头文件
int *plcd = NULL;

void lcd_draw_point(int x0, int y0, int color)
{
	*(plcd + y0*800 + x0) = color;
}

//在顶点为x0,y0的位置开始显示一个颜色矩形，矩形的宽度为w, 高度为h,颜色为color
void lcd_draw_rect(int x0, int y0, int w, int h, int color)
{
	int x, y;
	for (y = y0; y < y0+h; y++)
	{
		for (x = x0; x < x0+w; x++)
			lcd_draw_point(x, y, color);
	}
}

//把lcd清成color颜色
void lcd_clean_screen(int color)
{
	int x, y;
	for (y = 0; y < 480; y++)
	{
		for (x = 0; x < 800; x++)
			lcd_draw_point(x, y, color);
	}
}

void delayms(int xms){
	int i,j;
	for( i = 0; i < xms; i++)
		for( j = 110; j > 0; j--);
}


//显示一个字
void lcd_draw_word(unsigned char *ch, int color, int len,  int w, int x, int y)
{
	int i, j;

	int high;

	//32 * 64
	// 一个字的显示：画点（一个一个像素点）
	// 八个点用一个 unsigned char
	// 已知：数组长度：len => 像素点的个数：len * 8
	// 已知：字的宽度=> 高度：len*8/w
	high = len*8 / w; //64
	int flag;
	flag = w/8; // 4
	for (i = 0; i < len; i++)
	{
		for (j = 7; j >= 0; j--)
		{
			if ((ch[i] >> j) & 1 == 1)
			{
				if (i%flag == 0)
					lcd_draw_point(7-j+x, i/flag+y, color);
				else
					lcd_draw_point(8*(i%flag)+7-j+x, i/flag+y, color);
			}
		}
	}
}

//显示一张24位的bmp图片，图片的宽度必须为4的倍数
void lcd_draw_bmp(char *name, int x0, int y0, int w, int h)
{
	int fd;
	fd = open(name, O_RDWR);
	if (-1 == fd)
	{
		printf("open %s error!\n", name);

		return ;
	}

	//跳过前54个byte
	lseek(fd, 54, SEEK_SET);
	char ch[w*h*3];
	read(fd, ch, sizeof(ch));
	close(fd);

	//lcd 的像素点占4byte，24位bmp 3byte
	int color;
	char r, g, b;
	//位运算
	int x, y, i = 0;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			b = ch[i];
			g = ch[i+1];
			r = ch[i+2];
			i += 3;
			color = (r << 16) | (g << 8) | b;
			lcd_draw_point(x+x0, h-y+y0, color);
			delayms(1);
		}

	}
}
//void lcd_draw_bmpop（）函数左右颠倒显示一张24位的bmp图片，图片的宽度必须为4的倍数
void lcd_draw_bmpop(char *name, int x0, int y0, int w, int h)
{
	int fd;
	fd = open(name, O_RDWR);
	if (-1 == fd)
	{
		printf("open %s error!\n", name);

		return ;
	}
	//跳过前54个byte
	lseek(fd, 54, SEEK_SET);
	char ch[w*h*3];
	read(fd, ch, sizeof(ch));
	close(fd);
	//lcd 的像素点占4byte， 24位bmp 3byte
	int color;
	char r, g, b;
	//位运算
	int x, y, k,i = 0;
	for (y = 0; y < h; y++)
	{
		for (x = w; x >0; x--)
		{
			b = ch[i];
			g = ch[i+1];
			r = ch[i+2];
			i += 3;
			color = (r << 16) | (g << 8) | b;
			lcd_draw_point(x+x0, h-y+y0,color);

		}
	}
}
//void select_interface()函数显示五个特效选择界面和退出选择按钮
void select_interface()
{
        lcd_clean_screen(0xFFFFFFFF);
        //在长方形框里显示“聚焦显示”这四个字
        lcd_draw_rect(45, 220,70,40, 0x66666600);
        lcd_draw_word(ju, 0xFFFFFFFF, 32, 16, 48, 232);
        lcd_draw_word(jiao,0xFFFFFFFF, 32, 16,64, 232);
        lcd_draw_word(zhu, 0xFFFFFFFF, 32, 16,80, 232);
        lcd_draw_word(xiansi,0xFFFFFFFF, 32, 16,96,232);
        //在长方形框里显示“反向显示”这四个字
        lcd_draw_rect(205, 140,70,40, 0x66666600);
        lcd_draw_word(fan, 0xFFFFFFFF, 32,16,208,152);
        lcd_draw_word(xiangagain,0xFFFFFFFF,32,16,224,152);
        lcd_draw_word(xian, 0xFFFFFFFF,32,16,240,152);
        lcd_draw_word(shi,0xFFFFFFFF,32,16,256,152);
         //在长方形框里显示“聚中显示”这四个字
        lcd_draw_rect(365, 140,70,40, 0x66666600);
        lcd_draw_word(ju, 0xFFFFFFFF, 32,16,368,152);
        lcd_draw_word(zhong,0xFFFFFFFF,32,16,384,152);
        lcd_draw_word(xian, 0xFFFFFFFF,32,16,400,152);
        lcd_draw_word(shi,0xFFFFFFFF,32,16,416,152);
         //在长方形框里显示“逆序显示”这四个字
        lcd_draw_rect(525, 140,70,40, 0x66666600);
        lcd_draw_word(ni, 0xFFFFFFFF, 32,16,528,152);
        lcd_draw_word(xu,0xFFFFFFFF,32,16,544,152);
        lcd_draw_word(xian, 0xFFFFFFFF,32,16,560,152);
        lcd_draw_word(shi,0xFFFFFFFF,32,16,576,152);
         //在长方形框里显示“放大逐现”这四个字
        lcd_draw_rect(685, 220,70,40, 0x66666600);
        lcd_draw_word(fang, 0xFFFFFFFF, 32, 16,688,232);
        lcd_draw_word(da,0xFFFFFFFF, 32, 16,704,232);
        lcd_draw_word(zhu, 0xFFFFFFFF, 32, 16,720, 232);
        lcd_draw_word(xiansi,0xFFFFFFFF, 32, 16,736,232);
         //在长方形框里显示“退出”这两个字
        lcd_draw_rect(370,400,60,40, 0x0000ff00);
        lcd_draw_word(tui,0xFFFFFFFF, 32, 16,384,412);
        lcd_draw_word(chu,0xFFFFFFFF, 32, 16,400,412);
}
 //int monitor_touch_judge_entry()函数根据是否触摸到“进入”这个长方形框区域判断是否进入特效选择界面
int monitor_touch_judge_entry()
{
	int fd;
	fd = open("/dev/event0", O_RDWR);
	if (-1 == fd)
		return ;
	struct input_event ev;
	int x, y;
	while (1)
	{
		read(fd, &ev, sizeof(ev));
		//如何判断该事件是一个触摸事件
		if ((ev.type == EV_ABS) && (ev.code == ABS_X))
		{
			x = ev.value;
		}
		else if ((ev.type == EV_ABS) && (ev.code == ABS_Y))
		{
			y = ev.value;
		}

		//手指弹起，如触摸到“进入”这个长方形框区域则返回“1”
		if ((ev.type == EV_ABS) && (ev.code == ABS_PRESSURE) && (ev.value == 0))
		{
			printf("x: %d, y: %d\n", x, y);
                if((x >= 640&& x <700) && (y >=40 &&y<80))
                 return 1;
		}
	}

	close(fd);
}
//void login_interface()函数显示登录界面
void login_interface()
{       
         //显示背景图片和“电子相册登录”这六个字
        lcd_draw_bmp("jiemian.bmp", 1, 1, 796, 477);
        lcd_draw_word(dian, 0xFFFFFFFF, 441, 56, 100, 100);
	    lcd_draw_word(zi, 0xFFFFFFFF, 441, 56, 209, 100);
	    lcd_draw_word(xiang, 0xFFFFFFFF, 441, 56, 317,100);
	    lcd_draw_word(ce, 0xFFFFFFFF, 441, 56, 427, 100);
	    lcd_draw_word(deng, 0xFFFFFFFF, 441, 56, 535, 100);
	    lcd_draw_word(lu, 0xFFFFFFFF, 441, 56, 644, 100);
         //在长方形框里显示“进入”这两个字
        lcd_draw_rect(640,400,60,40, 0x0000ff00);
        lcd_draw_word(jin, 0xFFFFFFFF, 32,16,654,412);
        lcd_draw_word(ru,0xFFFFFFFF, 32,16,670,412);
        delayms(3000000);
}
//void draw_bmp(int x, int y)函数根据触摸的（x，y）坐标判断做什么动作：播放哪一种特效或者退出该界面
void draw_bmp(int x, int y){
//效果一：聚焦逐现
        if((x >=50&& x <110) && (y >= 220 && y <260))
        {
        lcd_clean_screen(0xFFFFFFFF);
	    lcd_draw_bmp("zs.bmp", 1, 1, 796, 477);
        delayms(3000000);
        lcd_draw_bmp("zsone.bmp", 100, 60, 600, 360);
        delayms(3000000);
        lcd_draw_bmp("zstwo.bmp", 200, 120, 400, 240);
        delayms(3000000);
        lcd_draw_bmp("zsthree.bmp", 300, 140, 200, 200);
        delayms(3000000);
        lcd_draw_bmpop("zs.bmp", 1, 1, 796, 477);
        delayms(3000000);
	    }
        //效果二:反向显示
        if((x >= 210&& x < 270) && (y >= 300 && y <340))
        {
        lcd_clean_screen(0xFFFFFFFF);
	    lcd_draw_bmp("boat.bmp", 2,15, 796, 150);
        delayms(3000000);
        lcd_draw_bmp("sun.bmp", 2,165, 796, 150);
        delayms(3000000);
        lcd_draw_bmp("leaf.bmp", 2,315, 796, 150);
        delayms(3000000);
        lcd_draw_bmpop("boat.bmp",2,15, 796, 150);
        delayms(3000000);
        lcd_draw_bmpop("sun.bmp", 2,165, 796, 150);
        delayms(3000000);
        lcd_draw_bmpop("leaf.bmp",2,315, 796, 150);
        delayms(3000000);
	    }
        //效果三：聚中显示
         if((x >= 370&& x <430) && (y >= 300 && y <340))
        {
        lcd_clean_screen(0xFFFFFFFF);
	    lcd_draw_bmp("flower.bmp", 0,0, 320,220);
        lcd_draw_bmp("flower.bmp",480,0,320,220);
        lcd_draw_bmp("flower.bmp",0,252,320,220);
        lcd_draw_bmp("flower.bmp",480,252,320,220);
        delayms(3000000);

        lcd_draw_bmp("flower.bmp", 15,4, 320,220);
        lcd_draw_bmp("flower.bmp",465,4,320,220);
        lcd_draw_bmp("flower.bmp",15,248,320,220);
        lcd_draw_bmp("flower.bmp",465,248,320,220);
        delayms(3000000);

        lcd_draw_bmp("flower.bmp", 30,8, 320,220);
        lcd_draw_bmp("flower.bmp",450,8,320,220);
        lcd_draw_bmp("flower.bmp",30,244,320,220);
        lcd_draw_bmp("flower.bmp",450,244,320,220);
        delayms(3000000);

        lcd_draw_bmp("flower.bmp", 45,12, 320,220);
        lcd_draw_bmp("flower.bmp",435,12,320,220);
        lcd_draw_bmp("flower.bmp",45,240,320,220);
        lcd_draw_bmp("flower.bmp",435,240,320,220);
	    delayms(3000000);

        lcd_draw_bmp("flower.bmp", 60,13, 320,220);
        lcd_draw_bmp("flower.bmp",420,13,320,220);
        lcd_draw_bmp("flower.bmp",60,239,320,220);
        lcd_draw_bmp("flower.bmp",420,239,320,220);
	    delayms(3000000);

        lcd_draw_bmp("flower.bmp", 75,14, 320,220);
        lcd_draw_bmp("flower.bmp",405,14,320,220);
        lcd_draw_bmp("flower.bmp",75,238,320,220);
        lcd_draw_bmp("flower.bmp",405,238,320,220);
	    delayms(6000000);

        lcd_draw_rect(200, 100, 400, 240, 0xFFFFFFFF);
        lcd_draw_bmp("flower.bmp",240,110,320,220);
        delayms(3000000);
	    }
        //效果四：逆序显示
        if((x >= 530&& x < 590) && (y >= 300 && y <340))
        {
        lcd_clean_screen(0xFFFFFFFF);
	    lcd_draw_bmp("starone.bmp", 20, 1, 240, 478);
        delayms(3000000);
        lcd_draw_bmp("startwo.bmp", 280, 1, 240, 478);
        delayms(3000000);
        lcd_draw_bmp("starthree.bmp",540, 1, 240, 478);
        delayms(3000000);
        lcd_draw_bmp("starone.bmp",540, 1, 240, 478);
        delayms(3000000);
        lcd_draw_bmp("starthree.bmp", 280, 1, 240, 478);
        delayms(3000000);
        lcd_draw_bmp("startwo.bmp", 20, 1, 240, 478);
        delayms(3000000);
	     }
         //效果五：放大逐现
        if((x >= 690&& x < 750) && (y >= 220 && y <260))
        {
        lcd_clean_screen(0xFFFFFFFF);
	    lcd_draw_bmp("riverone.bmp", 20, 10, 360, 220);
        lcd_draw_bmp("riverone.bmp", 420, 10, 360, 220);
        lcd_draw_bmp("riverone.bmp", 20, 250, 360, 220);
        lcd_draw_bmp("riverone.bmp", 420, 250, 360, 220);
        delayms(2000000);

        lcd_draw_bmp("rivertwo.bmp",350, 150, 100, 180);
        delayms(3000000);
        lcd_draw_bmp("riverthree.bmp",300, 140, 200, 200);
        delayms(3000000);
        lcd_draw_bmp("riverfour.bmp",200, 120, 400, 240);
        delayms(3000000);
        lcd_draw_bmp("riverfive.bmp",100, 60, 600, 360);
        delayms(3000000);
        lcd_draw_bmp("riversix.bmp",1,1,796,478);
        delayms(3000000);
        lcd_clean_screen(0xFFFFFFFF);
        lcd_draw_bmp("riverone.bmp", 20, 10, 360, 220);
        lcd_draw_bmpop("riverone.bmp", 420, 10, 360, 220);
        lcd_draw_bmpop("riverone.bmp", 20, 250, 360, 220);
        lcd_draw_bmp("riverone.bmp", 420, 250, 360, 220);
        delayms(3000000);
	     }
        
        //退出选择按钮
        if((x >= 370&& x <430) && (y >=40 &&y<80))
        {
                   login_interface();
             if(monitor_touch_judge_entry()==1)
             select_interface();
         }
}
//void monitor_touch()函数接收到触摸的（x,y）坐标点后，再调用void draw_bmp(int x, int y)判断触摸区域进行功能选择
void monitor_touch()
{
	int fd;
	fd = open("/dev/event0", O_RDWR);
	if (-1 == fd)
		return ;
struct input_event ev;
	int x, y;
	while (1)
	{
		read(fd, &ev, sizeof(ev));
		//如何判断该事件是一个触摸事件
		if ((ev.type == EV_ABS) && (ev.code == ABS_X))
		{
			x = ev.value;
		}
		else if ((ev.type == EV_ABS) && (ev.code == ABS_Y))
		{
			y = ev.value;
		}
		//手指弹起
		if ((ev.type == EV_ABS) && (ev.code == ABS_PRESSURE) && (ev.value == 0))
		{
			printf("x: %d, y: %d\n", x, y);
                       //播放特效
                        draw_bmp(x, y);
                        //播放完特效回到特效选择界面
                        select_interface();
		}
	}

	close(fd);
}
//主函数 
int main(int argc, char *argv[])
{
	int fd;
	//O_RDWR: 可读可写
	fd = open("/dev/fb0", O_RDWR);
	// if(fd == -1)
	//if (fd = -1)
	if (-1 == fd)
	{
		printf("open dev/fb0 error !\n");
		return 0;
	}
	//内存映射，把一个文件或者设备映射到一个进程的地址空间中（内存），在进程中操作这块映射的内存就相当于操作被映射的文件
	//plcd 存储了映射的地址空间的首地址
	plcd = mmap(NULL, 800*480*4,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0);
	//如何把lcd上的第n个像素点 显示成红色？
	//*(plcd + n) = 0x00ff0000;
	//如何把坐标为(x, y)的像素点，显示成红色？
	//*(plcd + y*800+x) = 0x00ff0000;
	//如何把整个屏幕全部显示白色？//循环
	lcd_clean_screen(0xFFFFFFFF);
	//如何显示一个矩形
	//lcd_draw_rect(50, 50, 100, 100, 0x00ff0000);
	//初始化串口
	//int com_fd = init_serial("/dev/s3c2410_serial3", 9600);
	//lcd_draw_word(wo, 0x00ff0000, 32, 16, 100, 100);

        //清屏
        lcd_clean_screen(0xFFFFFFFF);
        //显示登录界面
        login_interface();
        //登录进去显示五个特效选择界面和退出按钮
        if(monitor_touch_judge_entry()==1)
        select_interface();
        //显示触摸位置做相应动作
monitor_touch();
}

