/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "ff.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_exception.h"

#define BASEADDR 0xA0010000

#define BUFFER 0x10000000

int readcgrareg(uintptr_t addr){
    volatile int* addr1= (volatile int *)(addr + BASEADDR);
	return *addr1;
}
void writecgrareg(uintptr_t addr,int data){
    volatile int* addr1= (volatile int *)(addr + BASEADDR);
	*addr1 = data;
	int rdata = readcgrareg(addr);
	if(rdata != data){
		xil_printf("write addr%d failed,write %d, read %d\n\r",addr,data,rdata);
	}
}
static FATFS fatfs;
int platform_init_fs(){
	FRESULT status;
	TCHAR *Path = "0:/";

	xil_printf("mounting\r\n");
	status = f_mount(&fatfs,Path,0);
	if(status != FR_OK){
		xil_printf("Volume is not FAT formated; formating FAT\r\n");
		return -1;
	}
	xil_printf("mount success\r\n");
	return 0;
}
int sd_write_data(char *file_name, u32 src_addr,u32 byte_len){
	FIL fil;
	UINT bw;

	f_open(&fil,file_name,FA_CREATE_ALWAYS | FA_WRITE);
	f_lseek(&fil,0);

	f_write(&fil,(void*)src_addr,byte_len,&bw);
	f_close(&fil);
	return 0;
}
int sd_read_data(char *file_name,u32 src_addr,u32 byte_len){
	FIL fil;
	UINT br;
	f_open(&fil,file_name,FA_READ);
	f_lseek(&fil,0);
	f_read(&fil,(void*)src_addr,byte_len,&br);
	f_close(&fil);
	return 0;
}
int main()
{
    init_platform();

    platform_init_fs();
    char bitstream[7744];
    sd_read_data("0:/bit.bin",(u32)(bitstream),7744);

    FILINFO fileInfo1;
    f_stat("0:/bit.bin",&fileInfo1);
    xil_printf("bit.bin file size: %d \n\r",fileInfo1.fsize);

    XAxiDma axidma;
    XAxiDma_Config *config;
    int status;
    config = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if (!config) {
    	xil_printf("No config found for %d\r\n", XPAR_AXIDMA_0_DEVICE_ID);
    	return XST_FAILURE;
    }
    status = XAxiDma_CfgInitialize(&axidma, config);
    if (status != XST_SUCCESS) {
    	xil_printf("Initialization failed %d\r\n", status);
    	return XST_FAILURE;
    }
    XAxiDma_Reset(&axidma);
    while(!XAxiDma_ResetIsDone(&axidma)){printf("DMA is not reset");}
    /*
    DIR dir;
    FILINFO fileInfo;
    FRESULT res = f_opendir(&dir,"0:/");
    if(res == FR_OK){
    	for(;;){
    		res = f_readdir(&dir,&fileInfo);
    		if(res != FR_OK || fileInfo.fname[0] == 0) break;
    		xil_printf("%s\r\n",fileInfo.fname);
    	}
    }
    f_closedir(&dir);
    */

    /*
    for(int i =0; i < 20000; i++){
    	xil_printf("%d\n\r",i);
    }*/
    // config

    print("start conifg cgra\n\r");
    writecgrareg(0,1);
    print("change cgra state to 1\n\r");
    Xil_DCacheFlush();
    XAxiDma_SimpleTransfer(&axidma, (uintptr_t)bitstream, 7744 * 4, XAXIDMA_DMA_TO_DEVICE);
    print("dma transform config data to cgra \n\r");

    //load data
    int data1[20] ={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    int data2[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    int data3[20] ={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    int* data4 = (int *)BUFFER;
    Xil_DCacheFlush();
    int* datas[4]; datas[0] = data1; datas[1] = data2; datas[2] = data3; datas[3] = data4;
    writecgrareg(0,2);
    for(int i = 0;i<3;i++){
    xil_printf("load data %d\n\r",i);
      writecgrareg(2<<2,i);//memnum
      writecgrareg(3<<2,0);//startaddr
      writecgrareg(4<<2,0);//addaddr
      XAxiDma_SimpleTransfer(&axidma, (uintptr_t)datas[i], 20*4, XAXIDMA_DMA_TO_DEVICE);
      while(XAxiDma_Busy(&axidma, XAXIDMA_DMA_TO_DEVICE)){};

    }

    //exe
    xil_printf("start exe\n\r");
    writecgrareg(1<<2,0);
    if(readcgrareg(1<<2) == 1){
    	xil_printf("err finished reg is 1 before exe");
    	return;
    }
    writecgrareg(0,3);
    while(readcgrareg(0)!=0);
    xil_printf("exe finish, finish reg = %d\n\r", readcgrareg(1<<2));


    //readdata
    writecgrareg(2<<2,3);
    writecgrareg(3<<2,0);
    writecgrareg(4<<2,0);
    writecgrareg(5<<2,20);

    XAxiDma_SimpleTransfer(&axidma, (uintptr_t)BUFFER, 20*4, XAXIDMA_DEVICE_TO_DMA);
    writecgrareg(0,4);
    while(XAxiDma_Busy(&axidma, XAXIDMA_DEVICE_TO_DMA)){};
    for(int i = 0; i<20 ;i ++){
    	xil_printf("data %d = %d\n\r",i,data4[i]);
    }

    cleanup_platform();
    return 0;
}

