/*
 * saVideoARGB.c
 *
 * This is a V4L2 sample application to show ARGB buffer format functionality
 * in Video2. The app puts a moving horizontal bar on the display device
 * in various shades of colors. This application runs RGB565 format with
 * size VGA. This application uses panning facility of the FBDEV driver to
 * move color bars.
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/******************************************************************************
  Header File Inclusion
 ******************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/videodev.h>

//MMN_blank_1_1 
#include <linux/videodev2.h>  /* video4l2 apis */
//MMN_blank_1_2
#include <linux/fb.h>  /* frame buffer */
//MMN_blank_1_3
#include <pthread.h> /* pthread */  

/******************************************************************************
 Macros
 MAX_BUFFER     : Changing the following will result different number of
		  instances of buf_info structure.
 MAXLOOPCOUNT	: Display loop count
 WIDTH		: Width of the output image
 HEIGHT		: Height of the output image
 ******************************************************************************/
#define NUMBUFFERS              2
#define WIDTH			200
#define HEIGHT			200	
#define OMAP_BMP_FILE1		"Lab.rgb"
#define OMAP_BMP_FILE2		"Lab2.rgb"
#define LOOPCOUNT		30

char dev_name[2][20] = { "/dev/video1", "/dev/video2" };
static unsigned int  video1[WIDTH][HEIGHT];
static unsigned int  video2[WIDTH][HEIGHT];

struct buf_info {
	unsigned int length;
	char *start;
};

void fill(void *start, unsigned int w, unsigned int h,unsigned int flag)
{
	int i, j;
	static int count = 0;
	static int direction = 0;

	if(flag == 1) {
		if(count >= 250 || (count <= 0 && direction)) {
			direction = ~direction;
		}
		if(!direction)
			count+=25;
		else
			count-=25;
	}
	unsigned int *ptr = (unsigned int *)start;
	ptr += w* (h-1) ;
	for(i = 0 ; i < h ; i ++) {
		for (j = 0 ; j < w ; j ++) {
			if(flag == 1) {
				*ptr  = ((video2[i][j] & 0x00FFFFFF) |
						count << 24);
			} else if (flag == 0) {
				*ptr = video1[i][j];
			}
			ptr ++;
		}
		ptr -= 2*w;
	}

}

/* Thread for video2 streaming */
void * video2_threadfun(void *ptr)
{
	int mode = O_RDWR, ret, j, i;
	int fd, a, numbuffers = NUMBUFFERS, ch_no = *(int*)ptr;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	struct buf_info *buff_info;
	struct v4l2_format fmt;
	struct v4l2_crop crop;

	/* Open the video2 device */
	fd = open((const char *)dev_name[ch_no], mode);
	if (fd <= 0) {
		printf("Cannot open = %s device\n", dev_name[ch_no]);
		exit(0);
	}

	/* Get the format before setting and                          //Set the value and then ioctl()
	 * set only required parameters */
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	
	ret = ioctl(fd , VIDIOC_G_FMT,	&fmt);                       
	if (ret < 0) {
		perror("VIDIOC_G_FMT\n");
		close(fd);
		exit(0);
	}
	/* Set the image size and pixel format*/
	fmt.fmt.pix.width = WIDTH;
	fmt.fmt.pix.height = HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;

	//MMN blank2_1
	ret = ioctl(fd VIDIOC_S_FMT, &fmt);    //finish the pixel format                  
	if (ret < 0) {
		perror("VIDIOC_S_FMT\n");
		close(fd);
		exit(0);
	}
	/* Get the crop before setting and
	 * set only required parameters */
	crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	ret = ioctl(    fd, VIDIOC_G_CROP,    &crop);
	if (ret < 0) {
		perror("VIDIOC_G_CROP\n");
		close(fd);
		exit(0);
	}
	/* Set the Cropping parameters */
	crop.c.left = 0;
	crop.c.top = 0;
	crop.c.width = WIDTH;
	crop.c.height = HEIGHT;
        
        //MMN blank_2_2 
	ret = ioctl( fd  , VIDIOC_S_CROP,   &crop );  //finish the cropping parameters
	if (ret < 0) {
		perror("VIDIOC_S_CROP\n");
		close(fd);
		exit(0);
	}

	/* Get the window parameters before setting
	 * and set only required parameters */
	fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;

	
	ret = ioctl(fd , VIDIOC_G_FMT, &fmt  );                      
	if (ret < 0) {
		perror("VIDIOC_G_FMT 2\n");
		close(fd);
		exit(0);
	}
	/* Set the window size */
	fmt.fmt.win.w.left = 0 ;
	fmt.fmt.win.w.top = 0;
	fmt.fmt.win.w.width = WIDTH;
	fmt.fmt.win.w.height = HEIGHT;
	fmt.fmt.win.global_alpha = 255;

	//MMN blank_2_3
	ret = ioctl(fd, VIDIOC_S_FMT, &fmt);   //finish the the window size 
	if (ret < 0) {
		perror("VIDIOC_S_FMT 2\n");
		close(fd);
		exit(0);
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;                    

	ret = ioctl(fd , VIDIOC_G_FMT, &fmt);
	if (ret < 0) {
		perror("VIDIOC_G_FMT\n");
		close(fd);
		exit(0);
	}

	/* Request the buffers from the driver */
	/* req.memory can be user memory also */
	req.count = numbuffers;
	req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	req.memory = V4L2_MEMORY_MMAP;

	//MMN blank_2_4 REQBUFS
	ret = ioctl(  fd  ,  VIDIOC_REQBUFS  ,&req );    //REQBUFS 
	if (ret < 0) {
		perror("cannot allocate memory\n");
		close(fd);
		exit(0);
	}

	buff_info =
		(struct buf_info *) malloc(sizeof(struct buf_info) *
				req.count);
	if (!buff_info) {
		printf("cannot allocate memory for buff_info\n");
		close(fd);
		exit(0);
	}

	/* Query to get the physical address of the allocated
	 * buffers and get the virtual address of the buffer by
	 * passing the physical address to mmap call of the driver
	 */
	for (i = 0; i < req.count; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.index = i;
	
		//MMN blank_2_5 QUERYBUF
		ret = ioctl(   fd , VIDIOC_QUERYBUF  ,  &buf  );   //QUERYBUF 
		if (ret < 0) {
			perror("QUERYBUF ERROR\n");
			for (j = 0; j < i; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			close(fd);
			exit(0);
		}
		buff_info[i].length = buf.length;
		buff_info[i].start =
			mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
					MAP_SHARED, fd, buf.m.offset);

		if ((unsigned int) buff_info[i].start ==
				MAP_SHARED) {
			printf("Cannot mmap = %d buffer\n", i);
			for (j = 0; j < i; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			close(fd);
			exit(0);
		}
		memset(buff_info[i].start, 0x80, buff_info[i].length);      
		/* Fill the buffer with the RGB data  and Alpha value*/
		fill(buff_info[i].start, fmt.fmt.pix.width,
				fmt.fmt.pix.height, 1);
	}

	memset(&buf, 0, sizeof(buf));

	/* Enqueue buffers */
	for (i = 0; i < req.count; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.index = i;
		buf.memory = V4L2_MEMORY_MMAP;

		//MMN blank_2_6   QBUF
		ret = ioctl(fd, VIDIOC_QBUF , &buf );   //QBUF
		if (ret < 0) {
			perror("QBUF ERROR\n");
			for (j = 0; j < req.count; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			exit(0);
		}
	}

	a = 0;
	/* Start stream on */

	//MMN blank_2_7 STREAMON
	ret = ioctl(   fd , VIDIOC_STREAMON  , &a);   //STREAMON
	if (ret < 0) {
		perror("STREAMON ERROR\n");
		for (i = 0; i < req.count; i++)
			munmap(buff_info[i].start, buff_info[i].length);
		exit(0);
	}

	/* Forever loop for streaming*/
	for(i = 0 ; i < LOOPCOUNT ;i ++) {							//LOOPCOUNT  30
		/* Dqueue the already filled buffers */

		//MMN blank_2_8 DQBUF
		ret = ioctl( fd , VIDIOC_DQBUF   , &buf   );    //DQBUF
		if(ret < 0){
			perror("DQBUF ERROR\n");
			for (j = 0; j < req.count; j++)
				munmap(buff_info[j].start, buff_info[j].length);
			close(fd);
			exit(0);
		}
		/* Everytime fill the buffer with the different alpha value to
		 * view transperency
		 */
		fill(buff_info[buf.index].start, fmt.fmt.pix.width,
				fmt.fmt.pix.height, 1);
		
		/* Enqueue the filled buffer */
                //MMN blank_2_9 QBUF 
		ret = ioctl(  fd , VIDIOC_QBUF , &buf);  //QBUF
		if(ret < 0){
			perror("QBUF ERROR\n");
			for (j = 0; j < req.count; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			close(fd);
			exit(0);
		}
		sleep(1);
	}

	/* Put off streaming
	 * This will never execute as steaming is forever loop
	 */

	//MMN blank_2_10 STREAMOFF
	ret = ioctl(  fd  , VIDIOC_STREAMOFF   , &a);  //STREAMOFF
	if (ret < 0) {
		perror("STREAMOFF ERROR\n");
		for (i = 0; i < req.count; i++)
			munmap(buff_info[i].start, buff_info[i].length);
		close(fd);
		exit(0);
	}
	/* Unmap the buffers */
	for (i = 0; i < req.count; i++)
		munmap(buff_info[i].start, buff_info[i].length);
	close(fd);
	return 0;
}

/* Thread for video1 streaming */
void *video1_threadfun(void *ptr)
{
	int mode = O_RDWR, ret, j, i;
	int fd, a, numbuffers = NUMBUFFERS, ch_no = *(int*)ptr;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	struct buf_info *buff_info;
	struct v4l2_format fmt;
	struct v4l2_crop crop;
	struct v4l2_framebuffer framebuffer;

	/* Open the video1 device */
	fd = open((const char *)dev_name[ch_no], mode);
	if (fd <= 0) {
		printf("Cannot open = %s device\n", dev_name[ch_no]);
		exit(0);
	}

	/* Get the flags related to color keying and alpha blending */
	ret = ioctl (fd, VIDIOC_G_FBUF, &framebuffer);
	if (ret < 0) {
		perror ("VIDIOC_S_FBUF");
		return 0;

	}
	if(framebuffer.capability & V4L2_FBUF_CAP_LOCAL_ALPHA) {
		framebuffer.flags |= V4L2_FBUF_FLAG_LOCAL_ALPHA;

		ret = ioctl (fd, VIDIOC_S_FBUF, &framebuffer);
		if (ret < 0) {
			perror ("VIDIOC_S_FBUF");
			return 0;

		}

	}

	/* Get format before setting and
	 * set only required parameters */
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	
	ret = ioctl(  fd  , VIDIOC_G_FMT,  &fmt ); 
	if (ret < 0) {
		perror("VIDIOC_G_FMT\n");
		close(fd);
		exit(0);
	}
	/* Set the image size pixel format and device type*/
	fmt.fmt.pix.width = WIDTH;
	fmt.fmt.pix.height = HEIGHT;
	/* Video1 does not support ARGB so it takes * RGB32 as unpack RGB24 */
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;

	
	ret = ioctl( fd , VIDIOC_S_FMT,  &fmt  );       
	if (ret < 0) {
		perror("VIDIOC_S_FMT\n");
		close(fd);
		exit(0);
	}

	/* Get the parameters before setting */
	crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	
	ret = ioctl(  fd  , VIDIOC_G_CROP,  &crop  );
	if (ret < 0) {
		perror("VIDIOC_G_CROP\n");
		close(fd);
		exit(0);
	}
	/* Set the Cropping parameters */
	crop.c.left = 0;
	crop.c.top = 0;
	crop.c.width = WIDTH;
	crop.c.height = HEIGHT;

	
	ret = ioctl(  fd  , VIDIOC_S_CROP, &crop   );
	if (ret < 0) {
		perror("VIDIOC_S_CROP\n");
		close(fd);
		exit(0);
	}

	/* Get the window before setting  and
	 * set only required parameters
	 */
	fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;

	
	ret = ioctl(  fd  , VIDIOC_G_FMT, &fmt   );  
	if (ret < 0) {
		perror("VIDIOC_G_FMT 2\n");
		close(fd);
		exit(0);
	}
	/* Set the window size */
	fmt.fmt.win.w.left = 0;
	fmt.fmt.win.w.top = 0;
	fmt.fmt.win.w.width = WIDTH;
	fmt.fmt.win.w.height = HEIGHT;


	ret = ioctl(  fd  , VIDIOC_S_FMT, &fmt   );
	if (ret < 0) {
		perror("VIDIOC_S_FMT 2\n");
		close(fd);
		exit(0);
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	ret = ioctl(  fd  , VIDIOC_G_FMT, &fmt  );
	if (ret < 0) {
		perror("VIDIOC_G_FMT\n");
		close(fd);
		exit(0);
	}

	/* Request buffers from driver
	 * Buffer can be driver allocated type MMAP
	 * or userptr
	 */
	req.count = numbuffers;
	req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	req.memory = V4L2_MEMORY_MMAP;

	
	ret = ioctl(  fd  , VIDIOC_REQBUFS,  &req  ); 
	if (ret < 0) {
		perror("cannot allocate memory\n");
		close(fd);
		exit(0);
	}

	buff_info =
		(struct buf_info *) malloc(sizeof(struct buf_info) *
					   req.count);
	if (!buff_info) {
		printf("cannot allocate memory for buff_info\n");
		close(fd);
		exit(0);
	}
	/* Query the buffer to get the physical address of the allocated
	 * buffers and get the virtual  address of the buffer by
	 * passing the physical address to mmap call of the driver
	 */
	for (i = 0; i < req.count; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.index = i;

		
		ret = ioctl( fd   ,  VIDIOC_QUERYBUF  ,  &buf  ); 
		if (ret < 0) {
			perror("VIDIOC_QUERYBUF\n");
			for (j = 0; j < i; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			close(fd);
			exit(0);
		}
		buff_info[i].length = buf.length;
		buff_info[i].start =
			mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
					MAP_SHARED, fd, buf.m.offset);

		if ((unsigned int) buff_info[i].start ==
				MAP_SHARED) {
			printf("Cannot mmap = %d buffer\n", i);
			for (j = 0; j < i; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			close(fd);
			exit(0);
		}
		memset(buff_info[i].start, 0x80, buff_info[i].length);
		/* Initially fill the buffers */
		fill(buff_info[i].start, fmt.fmt.pix.width,
				fmt.fmt.pix.height, 0);
	}

	memset(&buf, 0, sizeof(buf));

	/* Enqueue buffers */
	for (i = 0; i < req.count; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.index = i;
		buf.memory = V4L2_MEMORY_MMAP;

		
		ret = ioctl(  fd  , VIDIOC_QBUF   ,  &buf  ); 
		if (ret < 0) {
			perror("VIDIOC_QBUF\n");
			for (j = 0; j < req.count; j++)
				munmap(buff_info[j].start,
						buff_info[j].length);
			exit(0);
		}
	}
	/* Start streamon */
	a = 0;

	
	ret = ioctl(  fd  , VIDIOC_STREAMON   , &a);   
	if (ret < 0) {
		perror("VIDIOC_STREAMON\n");
		for (i = 0; i < req.count; i++)
			munmap(buff_info[i].start, buff_info[i].length);
		exit(0);
	}

	/* Forever looop for streaming */
	for(i = 0 ; i < LOOPCOUNT ;i ++) {

				
		ret = ioctl( fd   , VIDIOC_DQBUF  , &buf ); 
		if(ret < 0){
			perror("VIDIOC_DQBUF\n");
			for (j = 0; j < req.count; j++)
				munmap(buff_info[j].start, buff_info[j].length);
			close(fd);
			exit(0);
		}
		/* Filling the buffer without alpha value since video1 does not
		 * support ARGB.   Here the alpha value is not changing hence
		 * this is filling buffer with same value each time.
		 */
		fill(buff_info[buf.index].start, fmt.fmt.pix.width,
				fmt.fmt.pix.height, 0);

		ret = ioctl(  fd  , VIDIOC_QBUF   ,  &buf  );
		if(ret < 0){
			perror("VIDIOC_QBUF\n");
			for (j = 0; j < req.count; j++)
				munmap(buff_info[j].start, buff_info[j].length);
			close(fd);
			exit(0);
		}
		sleep(1);
	}

	/* Streaming off */

	
	ret = ioctl(  fd  , VIDIOC_STREAMOFF   , &buf   );  
	if (ret < 0) {
		perror("VIDIOC_STREAMOFF\n");
		for (i = 0; i < req.count; i++)
			munmap(buff_info[i].start, buff_info[i].length);
		close(fd);
		exit(0);
	}
	/* Unmap all the buffers */
	for (i = 0; i < req.count; i++)
		munmap(buff_info[i].start, buff_info[i].length);

	/* Get the flags related to color keying and alpha blending */
	ret = ioctl (fd, VIDIOC_G_FBUF, &framebuffer);
	if (ret < 0) {
		perror ("VIDIOC_S_FBUF");
		return 0;

	}

	/* Check if device capable of alpha blending */
	if(framebuffer.capability & V4L2_FBUF_CAP_LOCAL_ALPHA) {
		framebuffer.flags &= ~V4L2_FBUF_FLAG_LOCAL_ALPHA;

		ret = ioctl (fd, VIDIOC_S_FBUF, &framebuffer);
		if (ret < 0) {
			perror ("VIDIOC_S_FBUF");
			return 0;

		}

	}

	close(fd);
	return 0;
}

int main()
{
	int ret1, ret2,ch_no1=0, ch_no2=1;
	pthread_t t1, t2;
	int i,j;
	FILE * file_r;
	char red,green,blue;
	unsigned int val;
	char str[250];

	strcpy(str, "echo ""gfx e:0"" > /sys/devices/platform/omapfb/overlays");

	/*  Disable Graphics pipeline for this demo.
	 *  So that it does not create show the past buffer
	 */
	if(system(str)) {
		printf("Failed to dislable graphics pipeline\n");
	}
	/* Open the file for the video1 and store the
	 * RGB values in buffer
	 */
	file_r = fopen(OMAP_BMP_FILE1, "r");
	if(file_r == NULL) {
		printf("Unable to open video1 file\n");
		return -1;
	}
	/* Read the entire image in an array */
	for(i = 0 ; i < HEIGHT ; i++) {
		for(j = 0 ; j < WIDTH ; j++) {
			blue = fgetc (file_r);
			green = fgetc (file_r);
			red = fgetc (file_r);
			val = (0 << 24) |(red << 16) | (green << 8) |
				(blue << 0);
			video1[i][j] = val;
		}

	}
	fclose(file_r);

	/* Open the file for video2 and store the RGB
	 * value in buffer
	 */
	file_r = fopen(OMAP_BMP_FILE2,"r");
	if(file_r == NULL) {
		printf("Unable to open video2 file\n");
		return -1;
	}
	for(i = 0 ; i < HEIGHT ; i++) {
		for(j = 0 ; j < WIDTH ; j++) {
			blue = fgetc (file_r);
			green = fgetc (file_r);
			red = fgetc (file_r);
			val = (0 << 24) | (red << 16) | (green << 8) |
				(blue << 0);
			video2[i][j] = val;
		}
	}
	fclose(file_r);

	/* Two threads are created. one thread displays RGB24 unpacked image
	 * into video1 pipeline and other thread displays ARGB image into
	 * video2 pipeline */
	/* Create threads for Video1 and Video2 streaming*/
	ret1 = pthread_create(&t1, NULL, video1_threadfun, &ch_no1);
	ret2 = pthread_create(&t2, NULL, video2_threadfun, &ch_no2);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	strcpy(str,"echo ""gfx e:1"" > /sys/devices/platform/omapfb/overlays");

		/*  Disable Graphics pipeline for this demo.
		 *  So that it does not create show the past buffer
		 */
		if(system(str)) {
			printf("Failed to dislable graphics pipeline\n");
		}

	return 0;
}

