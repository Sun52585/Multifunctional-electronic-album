/*
	NAME
       open and possibly create a file or device

SYNOPSIS
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>   //for open

       int open(const char *pathname, int flags);
       int open(const char *pathname, int flags, mode_t mode);

	   file  descriptor:文件描述符
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>  //for close
#include <sys/mman.h>
#include <linux/input.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pthread.h>
int *plcd = NULL;

char wo[] = {
	0x04,0x40,0x0E,0x50,0x78,0x48,0x08,0x48,0x08,0x40,0xFF,0xFE,0x08,0x40,0x08,0x44,
0x0A,0x44,0x0C,0x48,0x18,0x30,0x68,0x22,0x08,0x52,0x08,0x8A,0x2B,0x06,0x10,0x02
};

//初始化串口
int init_serial(const char *file, int baudrate)
{ 
	/*设置串口
	  波特率:19200
	  数据位:8
	  校验位:不要
	  停止位:1
	  数据流控制:无
	  */
#if 0
	struct termios
	{
		tcflag_t c_iflag;       /* input mode flags */
		tcflag_t c_oflag;       /* output mode flags */
		tcflag_t c_cflag;       /* control mode flags */
		tcflag_t c_lflag;       /* local mode flags */
		cc_t c_line;            /* line discipline */
		cc_t c_cc[NCCS];        /* control characters */
		speed_t c_ispeed;       /* input speed */
		speed_t c_ospeed;       /* output speed */
#define _HAVE_STRUCT_TERMIOS_C_ISPEED 1
#define _HAVE_STRUCT_TERMIOS_C_OSPEED 1
	};
#endif
	
	int fd;
	
	fd = open(file, O_RDWR);
	if (fd == -1)
	{
		perror("open device error:");
		return -1;
	}

	struct termios myserial;
	//清空结构体
	memset(&myserial, 0, sizeof (myserial));
	//O_RDWR               
	myserial.c_cflag |= (CLOCAL | CREAD);
	//设置控制模式状态，本地连接，接受使能
	//设置 数据位
	myserial.c_cflag &= ~CSIZE;   //清空数据位
	myserial.c_cflag &= ~CRTSCTS; //无硬件流控制
	myserial.c_cflag |= CS8;      //数据位:8

	myserial.c_cflag &= ~CSTOPB;//   //1位停止位
	myserial.c_cflag &= ~PARENB;  //不要校验
	//myserial.c_iflag |= IGNPAR;   //不要校验
	//myserial.c_oflag = 0;  //输入模式
	//myserial.c_lflag = 0;  //不激活终端模式

	switch (baudrate)
	{
		case 9600:
			cfsetospeed(&myserial, B9600);  //设置波特率
			cfsetispeed(&myserial, B9600);
			break;
		case 115200:
			cfsetospeed(&myserial, B115200);  //设置波特率
			cfsetispeed(&myserial, B115200);
			break;
		case 19200:
			cfsetospeed(&myserial, B19200);  //设置波特率
			cfsetispeed(&myserial, B19200);
			break;
	}
		/* 刷新输出队列,清楚正接受的数据 */
	tcflush(fd, TCIFLUSH);

	/* 改变配置 */
	tcsetattr(fd, TCSANOW, &myserial);

	return fd;
}

void lcd_draw_point(int x0, int y0, int color)
{
	*(plcd + y0*800 + x0) = color;
}

//在顶点为x0,y0的位置开始显示一个颜色矩形，矩形的宽//度为w, 高度为h,颜色为color
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
	
	//1、跳过前54个byte
	lseek(fd, 54, SEEK_SET);
	char ch[w*h*3];
	read(fd, ch, sizeof(ch));
	close(fd);
	
	//lcd 的像素点占4byte， 24位bmp 3byte
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
		}
	}
}

//操作触摸屏，获取触摸的x坐标和y坐标
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
			
		}
	}
	
	close(fd);
}

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
	
	//内存映射，吧一个文件或者设备映射到一个进程的///地址空间中（内存），在进程中操作这块映射的内///存就相当于操作 被映射的文件
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
	
	//如何把整个屏幕全部显示白色？
	//循环
	lcd_clean_screen(0xFFFFFFFF);
	
	//如何显示一个矩形
	lcd_draw_rect(50, 50, 100, 100, 0x00ff0000);
	
	//显示BMP图片
	lcd_draw_bmp("music.bmp", 100, 100, 240, 240);
	
	//初始化串口
	int com_fd = init_serial("/dev/s3c2410_serial3", 9600);
	
	monitor_touch();
	
	close(fd);
	
	return 0;
}