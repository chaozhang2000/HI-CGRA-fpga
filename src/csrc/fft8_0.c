#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "ff.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xtime_l.h"

#define BASEADDR 0xA0010000

#define BUFFER 0x10000000
#define TIMES 10
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
void fftkernel(int *in,int *w,int *out,int* m,int j, int k){
	int p = k+j;
	int q = k + j + ((*m)>>1);
	int w_rindex = j*2;
	int w_iindex = j*2+1;
	int p_rindex = p*2;
	int p_iindex = p*2+1;
	int q_rindex = q*2;
	int q_iindex = q*2+1;

	int inpr = in[p_rindex];
	int inpi = in[p_iindex];
	int inqr = in[q_rindex];
	int inqi = in[q_iindex];
	int wr = w[w_rindex];
	int wi = w[w_iindex];
    out[p_rindex] = inpr + ((inqr *wr - inqi * wi)>>TIMES);
		out[p_iindex] = inpi + ((inqr* wi + wr*inqi)>>TIMES);
    out[q_rindex] = inpr + ((- inqr *wr + inqi * wi)>>TIMES);
		out[q_iindex] = inpi+(( - inqr* wi - wr*inqi)>>TIMES);
}
int main()
{
    init_platform();

    platform_init_fs();
    char bitstream[7744];
    sd_read_data("0:/fft.bin",(u32)(bitstream),7744);

    FILINFO fileInfo1;
    f_stat("0:/fft.bin",&fileInfo1);
    xil_printf("fft.bin file size: %d \n\r",fileInfo1.fsize);

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
    XAxiDma_SimpleTransfer(&axidma, (uintptr_t)bitstream, 7744, XAXIDMA_DMA_TO_DEVICE);
    print("dma transform config data to cgra \n\r");

    //load data
		int butterfly_i[16] = {1024,0,1024,0,1024,0,1024,0,-1024,0,-1024,0,-1024,0,-1024,0};
		int butterfly_o[16] = {0};
		int w[16] = {1024,0,724,-724,0,-1023,-724,-724,0,0,0,0,0,0,0,0};
		int m[16] = {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    Xil_DCacheFlush();
    int* datas[4]; datas[0] = butterfly_i; datas[1] = butterfly_o; datas[2] = w; datas[3] = m;
    writecgrareg(0,2);
    for(int i = 0;i<4;i++){
    xil_printf("load data %d\n\r",i);
      writecgrareg(2<<2,i);//memnum
      writecgrareg(3<<2,0);//startaddr
      writecgrareg(4<<2,0);//addaddr
      XAxiDma_SimpleTransfer(&axidma, (uintptr_t)datas[i], 16*4, XAXIDMA_DMA_TO_DEVICE);
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
    XTime t_exestart;
    XTime_GetTime(&t_exestart);
    while(readcgrareg(0)!=0);
    XTime t_exeend;
    XTime_GetTime(&t_exeend);
    xil_printf("exe finish, finish reg = %d\n\r", readcgrareg(1<<2));


    //readdata
    writecgrareg(2<<2,1);
    writecgrareg(3<<2,0);
    writecgrareg(4<<2,0);
    writecgrareg(5<<2,16);

    XAxiDma_SimpleTransfer(&axidma, (uintptr_t)BUFFER, 16*4, XAXIDMA_DEVICE_TO_DMA);
    int *buffer = (int*)BUFFER;
    writecgrareg(0,4);
    while(XAxiDma_Busy(&axidma, XAXIDMA_DEVICE_TO_DMA)){};
    for(int i = 0; i<16 ;i ++){
    	xil_printf("data %d = %d\n\r",i,buffer[i]);
    }
    xil_printf("f = %ld HZ\n\r",COUNTS_PER_SECOND);
    xil_printf("exe time\n\r");
    xil_printf("exe time = %ld\n\r",t_exeend-t_exestart);


    Xil_DCacheFlush();
    XTime_GetTime(&t_exestart);

    for (int k=0;k<8;k+=m[0]){
    	for(int j=0;j<m[0]/2;j++){
    		fftkernel(butterfly_i,w,butterfly_o,m,j,k);
    	}
    }
    XTime_GetTime(&t_exeend);
    xil_printf("cpu exe time\n\r");
    xil_printf("cpu exe time = %ld\n\r",t_exeend-t_exestart);

    cleanup_platform();
    return 0;
}

