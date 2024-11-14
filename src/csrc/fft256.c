#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "ff.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xtime_l.h"

#define BASEADDR 0xA0010000
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
char bitstream_fft0[7744];

int main()
{
    init_platform();
    platform_init_fs();
    //bitstream

    sd_read_data("0:/fft2560.bin",(u32)(bitstream_fft0),7744);
    FILINFO fileInfo1;
    f_stat("0:/fft2560.bin",&fileInfo1);
    xil_printf("fft2560.bin file size: %d \n\r",fileInfo1.fsize);


    XAxiDma axidma;
    XAxiDma_Config *config;
		int dmastatus;
		//cgra data mems
		int* datas[4];//four datamems

		//fft input datas
		int butterfly_i[512] = {1024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0,0,1024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    0,0,1024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1024,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1024,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1024,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0,0,0,0,0,0,-1024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ,0,0,0,0,0,0};

		int butterfly_o[512] = {0};
		int w[512] = {1024,0,1023,-25,1022,-50,1021,-75,1019,-100,1016,-125,1012,-150,1008,-175,1004,-199,999,-224,993,-248,986,-273,979,-297,972,-321,964,-344,955,-368,946,-391,936,-414,925,-437,914,-460,903,-482,890,-504,878,-526,865,-547,851,-568,837,-589,822,-609,807,-629,791,-649,775,-668,758,-687,741,-706,724,-724,706,-741,687,-758,668,-775,649,-791,629,-807,609,-822,589,-837,568,-851,547,-865,526,-878,504,-890,482,-903,460,-914,437,-925,414,-936,391,-946,368,-955,344,-964,321,-972,297,-979,273,-986,248,-993,224,-999,199,-1004,175,-1008,150,-1012,125,-1016,100,-1019,75,-1021,50,-1022,25,-1023,0,-1023,-25,-1023,-50,-1022,-75,-1021,-100,-1019,-125,-1016,-150,-1012,-175,-1008,-199,-1004,-224,-999,-248,-993,-273,-986,-297,-979,-321,-972,-344,-964,-368,-955,-391,-946,-414,-936,-437,-925,-460,-914,-482,-903,-504,-890,-526,-878,-547,-865,-568,-851,-589,-837,-609,-822,-629,-807,-649,-791,-668,-775,-687,-758,-706,-741,-724,-724,-741,-706,-758,-687,-775,-668,-791,-649,-807,-629,-822,-609,-837,-589,-851,-568,-865,-547,-878,-526,-890,-504,-903,-482,-914,-460,-925,-437,-936,-414,-946,-391,-955,-368,-964,-344,-972,-321,-979,-297,-986,-273,-993,-248,-999,-224,-1004,-199,-1008,-175,-1012,-150,-1016,-125,-1019,-100,-1021,-75,-1022,-50,-1023,-25};

		//fft results
		int fft_result_cgra[512] = {0};
		int fft_result_cpu[512] = {0};

		//fft time
		XTime t_fft_cgra_exestart;
		XTime t_fft_cgra_exeend;
		XTime t_fft_cpu_exestart;
		XTime t_fft_cpu_exeend;


		/*init the dma*/
    config = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if (!config) {
    	xil_printf("No config found for %d\r\n", XPAR_AXIDMA_0_DEVICE_ID);
    	return XST_FAILURE;
    }
    dmastatus = XAxiDma_CfgInitialize(&axidma, config);
    if (dmastatus != XST_SUCCESS) {
    	xil_printf("Initialization failed %d\r\n", dmastatus);
    	return XST_FAILURE;
    }
    XAxiDma_Reset(&axidma);
    while(!XAxiDma_ResetIsDone(&axidma)){printf("DMA is not reset");}

    /*config cgra using fft.bin*/
        print("start conifg cgra using fft0.bin\n\r");
        writecgrareg(0,1);
        print("change cgra state to 1\n\r");
        Xil_DCacheFlush();
        XAxiDma_SimpleTransfer(&axidma, (uintptr_t)bitstream_fft0, 7744, XAXIDMA_DMA_TO_DEVICE);
        print("dma transform fft0 config data to cgra \n\r");

        /*load fft data to cgra*/
        Xil_DCacheFlush();
        datas[0] = butterfly_i; datas[1] = butterfly_o; datas[2] = w;
        writecgrareg(0,2);
        for(int i = 0;i<3;i++){
        xil_printf("load data %d\n\r",i);
          writecgrareg(2<<2,i);//memnum
          writecgrareg(3<<2,0);//startaddr
          writecgrareg(4<<2,0);//addaddr
          XAxiDma_SimpleTransfer(&axidma, (uintptr_t)datas[i], 512*4, XAXIDMA_DMA_TO_DEVICE);
          while(XAxiDma_Busy(&axidma, XAXIDMA_DMA_TO_DEVICE)){};
        }

        /*fft0 exe*/
        xil_printf("start exe fft\n\r");
        writecgrareg(1<<2,0);
        if(readcgrareg(1<<2) == 1){
        	xil_printf("err finished reg is 1 before exe");
        	return;
        }
        writecgrareg(0,3);
        XTime_GetTime(&t_fft_cgra_exestart);
        while(readcgrareg(0)!=0);
        XTime_GetTime(&t_fft_cgra_exeend);
        xil_printf("exe fft finish, finish reg = %d\n\r", readcgrareg(1<<2));


        /*read fft result*/
        writecgrareg(2<<2,1);
        writecgrareg(3<<2,0);
        writecgrareg(4<<2,0);
        writecgrareg(5<<2,512);
        XAxiDma_SimpleTransfer(&axidma, (uintptr_t)fft_result_cgra, 512*4, XAXIDMA_DEVICE_TO_DMA);
        writecgrareg(0,4);
        while(XAxiDma_Busy(&axidma, XAXIDMA_DEVICE_TO_DMA)){};
        for(int i = 0; i<512 ;i ++){
        	xil_printf("data %d = %d\n\r",i,fft_result_cgra[i]);
        }
        xil_printf("f = %ld HZ\n\r",COUNTS_PER_SECOND);
        xil_printf("exe time\n\r");
        xil_printf("exe time = %ld\n\r",t_fft_cgra_exeend-t_fft_cgra_exestart);


    		/*cpu calculate fft*/
        Xil_DCacheFlush();
        XTime_GetTime(&t_fft_cpu_exestart);
        int m = 2;
        for (int k=0;k<256;k+=m){
        	for(int j=0;j<m/2;j++){
        		fftkernel(butterfly_i,w,fft_result_cpu,&m,j,k);
        	}
        }
        XTime_GetTime(&t_fft_cpu_exeend);
        xil_printf("cpu exe time\n\r");
        xil_printf("cpu exe time = %ld\n\r",t_fft_cpu_exeend-t_fft_cpu_exestart);
        xil_printf("\n\r\n\r\n\r");
    cleanup_platform();
    return 0;
}

