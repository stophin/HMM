//ELibrary.h
//now we're using ege library tool
//you may change this
//once changed, associated files should be changed
//Author: Stophin
//2014.01.08
//Ver: 0.01
//
#ifndef _ELIBRARY_H_
#define _ELIBRARY_H_


#ifdef _NANO_MINGW_
#define _NANOC_WINDOWS_
#endif

#ifndef _NANO_LINUX_
#define _NANOC_WINDOWS_
#endif

#ifdef _NANOC_WINDOWS_
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include <math.h>

#define getch _getch
#define scanf scanf_s
#define kbhit _kbhit
#else 

#define _getch getch
#define _tmain main

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <math.h>

void changemode(int dir)
{
	static struct termios oldt, newt;

	if (dir == 1)
	{
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}
	else
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit(void)
{
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);

	select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);
}

int getch(void)
{
	struct termios tm, tm_old;
	int fd = 0, ch;

	if (tcgetattr(fd, &tm) < 0) {//保存现在的终端设置
		return -1;
	}

	tm_old = tm;
	cfmakeraw(&tm);//更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
	if (tcsetattr(fd, TCSANOW, &tm) < 0) {//设置上更改之后的设置
		return -1;
	}

	ch = getchar();
	if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//更改设置为最初的样子
		return -1;
	}

	return ch;
}

#define INT		int
#define VOID	void
#define _TCHAR	char

#define min(x, y) (x > y ? y : x)
#define max(x, y) (x > y ? x : y)

#endif

#define ZERO	1e-10
#define ISZERO(x) (x > -ZERO && x < ZERO)

#define EFTYPE double

#endif	//end of _ELIBRARY_H_
//end of file
