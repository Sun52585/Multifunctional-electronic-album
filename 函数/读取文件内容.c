/*

1. 打开或关闭一个文件
		
	NAME
       open,  creat   open and possibly create a file or  device

	SYNOPSIS
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

		open用来打开一个文件(或创建一个文件，并打开它)
       int open(const char *pathname, int flags);
       int open(const char *pathname, int flags, mode_t mode);
		pathname: 要打开或创建的文件名(带路径名)
		flags: 打开标志。告诉系统，是以何种方式打开这个文件
				O_RDONLY: read only
				O_WRONLY: write only
				O_RDWR: read/write
					　以上三个标志选其一。
				O_APPEND: append 追加方式
				O_CREAT: create 创建标志(如果文件不存在则创建)
       			O_EXCL: 该标志一般和O_CREAT配合使用，用来测试文件是否存在。
						指定O_EXCL | O_CREAT ，如果文件存在，则open失败，并且
						errno == EEXIST
       			O_NONBLOCK: non block() 非阻塞方式打开文件。
       					该标志如果设置，则为非阻塞方式
       					该标志如果不设置，则为阻塞方式
       			O_TRUNC: truncate ,截短标志。
       					假如文件存在，并且是一个普通文件，而且打开方式
       					为O_RDWR/O_WRONLY,则文件内容会被清空。
       			....
       			以上标志，用"或"来组合

       	当第二个参数指定了O_CREAT(创建标志),
		那么第三个参数mode指定创建的文件的权限
		mode: 用来指定新创建的文件的权限。有两种方式指定:
			(1) user: S_IRUSR S_IWUSR S_IXUSR  -> S_IRWXU == S_IRUSR  | S_IWUSR  |S_IXUSR
			      group: S_IRGRP S_IWGRP S_IXGRP -> S_IRWXG == S_IRGRP | S_IWGRP | S_IXGRP
			      other: S_IROTH S_IWOTH S_IXOTH -> S_IRWXO == S_IROTH | S_IWOTH | S_IXOTH
			(2) 0660

			返回值:
				如果成功返回文件描述符(>0,后续所有对文件的操作都必须
				通过它，因为它代表这个文件。)
				失败返回-1,  并且errno被设置


       int creat(const char *pathname, mode_t mode);
       <=> open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);


       =====
	NAME
       close - close a file descriptor

	SYNOPSIS
       #include <unistd.h>

		关闭一个文件描述符
       int close(int fd);
		
2. 读/写一个文件
	NAME
     read - read from a file

SYNOPSIS
       #include <unistd.h>

       		read用来从fd指定的文件中读取nbyte个字节数据到buf指向
       		的空间中去。
       ssize_t read(int fd, void *buf, size_t nbyte);
       	fd:文件描述符(open返回值),表示从哪个文件里面读
       	buf:一个指针，指向一段内存，用来保存从文件里面读到的数据
       	nbyte:读多少字节数据.
       	返回值:
       		> 0 : 成功读取到了字节数，文件偏移量(光标)随之增加
       		= 0: 到达文件尾了(文件结束啦)
       		< 0(-1) :出错了。同时errno被设置
	===
	NAME  b 
    write - write on a file

	SYNOPSIS
       #include <unistd.h>

		write用来把buf指向的内存空间中至多nbyte个字节数据写到
		fd指定的文件中去。
       ssize_t write(int fd, const void *buf, size_t nbyte);
       		fd:文件描述符(open返回值),表示写到哪个文件中去
       		buf:指针，指向要写到文件中去的数据
       		nbyte:写多少字节
       	返回值:
       		> 0 :成功写到文件中去的字节数，文件偏移量随之增加
       		=0: 什么也没写
       		-1:出错，同时errno被设置

3. 定位
	lseek
	
NAME
       lseek - reposition read/write file offset

SYNOPSIS
       #include <sys/types.h>
       #include <unistd.h>

		lseek用来给指定文件定位光标.(设置偏移量)
       off_t lseek(int fd, off_t offset, int whence);
       	fd:文件描述符，要定位的文件
       	offset:要设置的偏移量
       	whence: 定位方式,有三种:
       		SEEK_SET: 基于文件开头定位
       				新的光标位置为 = 文件开头　+　offset(不能< 0)
       		SEEK_CUR:基于当前光标位置定位
       				新的光标位置为= 当前位置　+ offset(可正可负)
       		SEEK_END:基于文件末尾定位
       				新的光标位置为=文件末尾　+ offset(可正可负)
       	返回值:
       		成功返回新的光标位置相对于文件开头的偏移量(以字节为单位)
       		失败返回-1
 4. mmap/munmap

 NAME
       mmap, munmap - map or unmap files or devices into memory
       		mmamp把一个文件或设备映射到内存
       		munmap解映射

SYNOPSIS
       #include <sys/mman.h>

       void *mmap(void *addr, size_t length, int prot, int flags,  int fd, off_t offset);
       	addr:把文件内容映射到内存哪个地址。一般为NULL,让操作系统自行分配。
       	length:要映射的文件内容的长度。向上取PAGE_SIZE(4K)的整数倍
       	prot:映射的内存区域的权限。(应与文件打开的权限一致，因为操作
       		此区域内存实际上就是操作文件内容)
       		PROT_EXEC:可执行
       		PROT_READ: 可读
       		PROT_WRITE:可写
       		PROT_NONE:没访问权限
       	 flags:映射标志。决定对映射部分的操作是否对其他进程可见。
       	 	MAP_SHARED: 共享。对其他进程可见，内存操作直接应用到文件中去。
       	 	MAP_PRIVATE:私有。对其他进程不可见，内存操作不应用到文件中去。
       	 fd:文件描述符。要映射的文件
       	 offset:偏移量。表示从文件的哪个位置开始映射。
       	 			offset必须为PAGE_SIZE(4k)的整数倍。
		返回值:
			成功返回映射内存区域的首地址。
			如果失败返回MAP_FAILED,同时errno被设置。


       
       int munmap(void *addr, size_t length);
       		解映射。与mmap相反的操作
       	进程退出，会自动unmap
       	但是关闭文件时，不会自动unmap
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>

int main()
{  
	int fd;
	int r;

	fd = open("/home/csgec/20151217.txt", O_RDWR | O_CREAT, 0660);
	if (fd < 0)
	{
		//if (errno == EEXIST)
		//{
		//	printf("file exists\n");
		//}
		perror("open failed:");
		return -1;
	}

	#if 1
	char str[10] ;

	r = read(fd, str, 10);
	if (r > 0)
	{
		str[r] = '\0';
		printf("r = %d :%s\n",r, str);
	}


	r = read(fd, str, 10);
	printf("r = %d\n", r);

	#endif

	//r = write(fd, "hello", 5);
	//printf("r = %d\n", r);


	//r = lseek(fd, 0, SEEK_END);
	//printf("r = %d\n" ,r);
	close(fd);

	return 0;
}
