`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/12/16 17:26:25
// Design Name: 
// Module Name: data_source_10_parallel_5MHz_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module data_source_10_parallel_5MHz_tb(

    );
    
    parameter DATA_WIDTH = 32;
    parameter START = -512;
    parameter END = 512;
    parameter STRIDE = 1;

    reg clk_5MHz;
    reg clk_200MHz;
    reg rst_n;
    reg trigger;
    wire [DATA_WIDTH-1:0] out_channel1_data1;
    wire [DATA_WIDTH-1:0] out_channel1_data2;
    wire [DATA_WIDTH-1:0] out_channel2_data1;
    wire [DATA_WIDTH-1:0] out_channel2_data2;
    wire out_channel1_data1_valid;
    wire out_channel1_data2_valid;
    wire out_channel2_data1_valid;
    wire out_channel2_data2_valid;
    
    data_source_10_parallel_5MHz #(DATA_WIDTH, START, END, STRIDE) u_data_source_5MHz(
        .clk_5MHz(clk_5MHz),
        .clk_200MHz(clk_200MHz),
        .rst_n(rst_n),
        .trigger(trigger),
        
        .out_channel1_data1_valid(out_channel1_data1_valid),
        .out_channel1_data1(out_channel1_data1),
        .out_channel1_data2_valid(out_channel1_data2_valid),
        .out_channel1_data2(out_channel1_data2),
        .out_channel2_data1_valid(out_channel2_data1_valid),
        .out_channel2_data1(out_channel2_data1),
        .out_channel2_data2_valid(out_channel2_data2_valid),
        .out_channel2_data2(out_channel2_data2)
    );
    
    always #(100) clk_5MHz = ~clk_5MHz;
    always #(2.5) clk_200MHz = ~clk_200MHz;
    
    initial begin
        clk_5MHz = 0;
        clk_200MHz = 0;
        trigger = 0;
        rst_n = 1;
        #5;
        rst_n = 0;
        #100;
        rst_n = 1;
        #100;
        trigger = 1;
    end
endmodule
