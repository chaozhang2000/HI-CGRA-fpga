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
int init_DMA(XAxiDma* axidma,XAxiDma_Config *config);
void config_CGRA(XAxiDma* axidma,char *bitstream,int bitstream_size);
void load_data_to_Datamem(XAxiDma* axidma,int* data, int memID,int startaddr,int size);
void read_result_from_Datamem(XAxiDma* axidma,int* result, int memID,int startaddr,int size);
void rotation_factor(int *r,int N,int n,int k);
char bitstream_fft0[7744];
char bitstream_fft1[7744];
char bitstream_conv3[7744];
//fft input datas
int butterfly_i[FFT_LEN*2] = {1024,0,-1024,0,1024,0,-1024,0,1024,0,-1024,0,1024,0,-1024,0};
int butterfly_o[FFT_LEN*2] = {0};
int w[FFT_LEN] = {1024,0,724,-724,0,-1023,-724,-724,0,0,0,0,0,0,0,0};
int m[LOG2_LEN] = {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//fft results
int fft_result_cgra[FFT_LEN*2] = {0};
int fft_result_cpu[FFT_LEN*2] = {0};
void main(){
    init_platform();
    platform_init_fs();

    //bitstream
    sd_read_data("0:/fft10240.bin",(u32)(bitstream_fft0),7744);
    FILINFO fileInfo1;
    f_stat("0:/fft10240.bin",&fileInfo1);
    xil_printf("fft10240.bin file size: %d \n\r",fileInfo1.fsize);

    sd_read_data("0:/fft10241.bin",(u32)(bitstream_fft1),7744);
    FILINFO fileInfo3;
    f_stat("0:/fft10241.bin",&fileInfo3);
    xil_printf("fft10241.bin file size: %d \n\r",fileInfo3.fsize);

	/*read bitstream from sd*/
    sd_read_data("0:/conv3.bin",(u32)(bitstream_conv3),7744);
    FILINFO fileInfo2;
    f_stat("0:/conv3.bin",&fileInfo2);
    xil_printf("conv3.bin file size: %d \n\r",fileInfo2.fsize);

    Xil_DCacheFlush();
    XAxiDma axidma;
    XAxiDma_Config *config;
		//cgra data mems
		int* datas[4];//four datamems



		//fft time
		XTime t_fft_cgra_exestart;
		XTime t_fft_cgra_exeend;
		XTime t_fft_cpu_exestart;
		XTime t_fft_cpu_exeend;

		//conv3 input datas
    int conv3_data1[200] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    int conv3_data2[200] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    int conv3_data3[200] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

		//conv3 results
		int conv3_result_cgra[200] = {0};
		int conv3_result_cpu[200] = {0};

		//conv3 time
    XTime t_conv3_cgra_exestart;
    XTime t_conv3_cgra_exeend;
    XTime t_conv3_cpu_exestart;
    XTime t_conv3_cpu_exeend;

		/*init the dma*/
    if(init_DMA(&axidma,config) == XST_FAILURE)return ;

    for(int i=1;i<LOG2_LEN;i++){
    	m[i] = m[i-1] << 1;
    }
		for(int i = 0; i<FFT_LEN/2; i++){
    	rotation_factor(&(w[i*2]),FFT_LEN,1,i);
		}
    Xil_DCacheFlush();


    /*config cgra using conv3.bin*/
    config_CGRA(&axidma,bitstream_conv3,7744);
       /*load conv3 data to cgra*/
       Xil_DCacheFlush();
       datas[0] = conv3_data1; datas[1] = conv3_data2; datas[2] = conv3_data3;
       for(int i = 0;i<3;i++){
       	load_data_to_Datamem(&axidma,datas[i], i, 0, 200*4);
       }
       /*conv3 exe*/
       xil_printf("start exe conv3\n\r");
       writecgrareg(1<<2,0);
       if(readcgrareg(1<<2) == 1){
       	xil_printf("err finished reg is 1 before exe");
       	return;
       }
       writecgrareg(0,3);
       XTime_GetTime(&t_conv3_cgra_exestart);
       while(readcgrareg(0)!=0);
       XTime_GetTime(&t_conv3_cgra_exeend);
       xil_printf("exe conv3 finish, finish reg = %d\n\r", readcgrareg(1<<2));

   		/*read conv3 result*/
       read_result_from_Datamem(&axidma,conv3_result_cgra,3,0,200*4);
       Xil_DCacheFlush();

       xil_printf("Superflex result:\n\r");
       for(int i = 0; i<200 ;i ++){
       	xil_printf("data %d = %d\n\r",i,conv3_result_cgra[i]);
       }
       //xil_printf("f = %ld HZ\n\r",COUNTS_PER_SECOND);
       //xil_printf("cpuf = %ld Hz\n\r",XPAR_CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ);
       //xil_printf("exe time\n\r");
       xil_printf("Superflex exeution time = %ld ns\n\r",(t_conv3_cgra_exeend-t_conv3_cgra_exestart)*10);


   		/*cpu calculate conv3*/
       Xil_DCacheFlush();
       XTime_GetTime(&t_conv3_cpu_exestart);
       int kernel[3][3]= {{1,2,3},{4,5,6},{7,8,9}};
       for (int i=1;i<200-1;i++){
       	int sum = 0;
       	for(int j=0;j<3;j++){
       		for(int k = 0; k < 3; k++){
       			sum += datas[j][i+k-1] * kernel[j][k];
       		}
       	}
       	conv3_result_cpu[i] = sum;
       	//kernel(data1,data2,data3,data4,i);
       }
       XTime_GetTime(&t_conv3_cpu_exeend);
       //xil_printf("cpu exe time\n\r");
       xil_printf("CPU exeution time = %ld\n\r",(t_conv3_cpu_exeend-t_conv3_cpu_exestart)*10);
       for(int k = 0;k<200;k++){
       	if(conv3_result_cpu[k]!=conv3_result_cgra[k]){
       		xil_printf("conv3 result error!!!!\n\r");
       		break;
       	}
       }



 /*config cgra using fft.bin*/
       char* bitstream_fft;
       int fftlen = FFT_LEN;
       int log2len = LOG2_LEN;
       for(int lay = 0; lay< log2len;lay++){
    	   bitstream_fft = (lay %2 == 0) ? bitstream_fft0 : bitstream_fft1;
    	   config_CGRA(&axidma,bitstream_fft,7744);
           int m1 = 1 << (lay + 1);
    	   writecgrareg(0,1);
           //writecgrareg(16<<2,fftlen);???????????????
           writecgrareg(17<<2,m1/2);
           writecgrareg(13<<2,m1);
           writecgrareg(25<<2,0);
           writecgrareg(26<<2,0);
            /*load fft data to cgra*/
    	   if(lay == 0){
            Xil_DCacheFlush();
            datas[0] = butterfly_i; datas[1] = butterfly_o; datas[2] = w; datas[3] = m;
            writecgrareg(0,2);
            load_data_to_Datamem(&axidma,datas[0], 0, 0, 2*fftlen*4);
            load_data_to_Datamem(&axidma,datas[2], 2, 0, 1*fftlen*4);
            load_data_to_Datamem(&axidma,datas[3], 3, 0, 1*log2len*4);
    	   }else{
    		   load_data_to_Datamem(&axidma,&(datas[3][lay]), 3, 0, 1*4);
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
            while(readcgrareg(0)!=0){;}
            XTime_GetTime(&t_fft_cgra_exeend);
            if(lay == log2len -1){
            	read_result_from_Datamem(&axidma,fft_result_cgra,(lay%2 == 0)?1:0,0,2*fftlen*4);
            }
       }

            for(int i = 0; i<fftlen*2 ;i ++){
            	xil_printf("data %d = %d\n\r",i,fft_result_cgra[i]);
            }
            //xil_printf("f = %ld HZ\n\r",COUNTS_PER_SECOND);
            //xil_printf("exe time\n\r");
            //xil_printf("exe time = %ld\n\r",t_fft_cgra_exeend-t_fft_cgra_exestart);
            xil_printf("Superflex exeution time = %ld ns\n\r",(t_fft_cgra_exeend-t_fft_cgra_exestart)*10);

        		/*cpu calculate fft*/
            Xil_DCacheFlush();
            XTime_GetTime(&t_fft_cpu_exestart);
            for (int k=0;k<FFT_LEN;k+=m[0]){
            	for(int j=0;j<m[0]/2;j++){
            		fftkernel(butterfly_i,w,fft_result_cpu,m,j,k);
            	}
            }
            XTime_GetTime(&t_fft_cpu_exeend);
            //xil_printf("cpu exe time\n\r");
            xil_printf("CPU exeution time = %ld ns\n\r",(t_fft_cpu_exeend-t_fft_cpu_exestart)*10);
            xil_printf("\n\r\n\r\n\r");


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
void load_data_to_Datamem(XAxiDma* axidma,int* data, int memID,int startaddr,int size){

	writecgrareg(0,2);
	print("change cgra state to load_data state\n\r");
	writecgrareg(2<<2,memID);
	writecgrareg(3<<2,startaddr);
	writecgrareg(4<<2,0);
	XAxiDma_SimpleTransfer(axidma, (uintptr_t)data, size, XAXIDMA_DMA_TO_DEVICE);
    while(XAxiDma_Busy(axidma, XAXIDMA_DMA_TO_DEVICE)){;}


}
void read_result_from_Datamem(XAxiDma* axidma,int* result, int memID,int startaddr,int size){

	writecgrareg(2<<2,memID);
	writecgrareg(3<<2,startaddr);
	        writecgrareg(4<<2,0);
	        writecgrareg(5<<2,size/4);
	        XAxiDma_SimpleTransfer(axidma, (uintptr_t)result, size, XAXIDMA_DEVICE_TO_DMA);
	        writecgrareg(0,4);
	        while(XAxiDma_Busy(axidma, XAXIDMA_DEVICE_TO_DMA)){};

}
void rotation_factor(int *r,int N,int n,int k)       //旋转因子
{
    *r = (int)(cos(2* PI * n * k /N)*pow(2,TIMES));
    *(r+1) =(int)(- sin(2* PI * n * k /N)*pow(2,TIMES));
}

