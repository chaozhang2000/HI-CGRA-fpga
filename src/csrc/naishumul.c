#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "ff.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xtime_l.h"
#include "unistd.h"
#include <math.h>

#define FFT_LEN 1024
#define LOG2_LEN 10
#define BASEADDR 0xA0010000
#define TIMES 10
#define PI 3.1415926
int readcgrareg(uintptr_t addr){
    volatile int* addr1= (volatile int *)(addr + BASEADDR);
	return *addr1;
}
void writecgrareg(uintptr_t addr,int data){
    volatile int* addr1= (volatile int *)(addr + BASEADDR);
	*addr1 = data;

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

int init_DMA(XAxiDma* axidma,XAxiDma_Config *config);
void config_CGRA(XAxiDma* axidma,char *bitstream,int bitstream_size);
char bitstream_mul0[3872];
void main(){
    init_platform();
    platform_init_fs();
	/*read bitstream from sd*/
    sd_read_data("0:/test.bin",(u32)(bitstream_mul0),3872);
    FILINFO fileInfo2;
    f_stat("0:/test.bin",&fileInfo2);
    xil_printf("naishumul.bin file size: %d \n\r",fileInfo2.fsize);

    Xil_DCacheFlush();
    XAxiDma axidma;
    XAxiDma_Config *config;
		/*init the dma*/
    if(init_DMA(&axidma,config) == XST_FAILURE)return ;

    Xil_DCacheFlush();

    /*config cgra using conv3.bin*/
    config_CGRA(&axidma,bitstream_mul0,3872);

    writecgrareg(6<<2,1);

    cleanup_platform();
    return ;
}
int init_DMA(XAxiDma* axidma,XAxiDma_Config *config){
	int dmastatus;
    config = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if (!config) {
    	xil_printf("No config found for %d\r\n", XPAR_AXIDMA_0_DEVICE_ID);
    	return XST_FAILURE;
    }
    dmastatus = XAxiDma_CfgInitialize(axidma, config);
    if (dmastatus != XST_SUCCESS) {
    	xil_printf("Initialization failed %d\r\n", dmastatus);
    	return XST_FAILURE;
    }
    XAxiDma_Reset(axidma);
    while(!XAxiDma_ResetIsDone(axidma)){printf("DMA is not reset");}
    return 0;
}
void config_CGRA(XAxiDma* axidma,char *bitstream,int bitstream_size){
    print("start conifg cgra\n\r");
    writecgrareg(0,1);
    print("change cgra state to config state\n\r");
    Xil_DCacheFlush();
    XAxiDma_SimpleTransfer(axidma, (uintptr_t)bitstream, bitstream_size, XAXIDMA_DMA_TO_DEVICE);
    while(XAxiDma_Busy(axidma, XAXIDMA_DMA_TO_DEVICE)){};
    print("config cgra finish \n\r");
}


