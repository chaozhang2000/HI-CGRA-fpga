`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/12/16 17:24:56
// Design Name: 
// Module Name: data_source_10_parallel_5MHz
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


module data_source_10_parallel_5MHz(
    clk_200MHz,
    rst_n,
    trigger,
    
    out_channel1_data1_valid,
    out_channel1_data1,
    out_channel1_data2_valid,
    out_channel1_data2,
    out_channel2_data1_valid,
    out_channel2_data1,
    out_channel2_data2_valid,
    out_channel2_data2
    );
    
parameter DATA_WIDTH = 32;
parameter START = -512;
parameter END = 512;
parameter STRIDE = 1;
    
input clk_200MHz;
input rst_n;
input trigger;
output [DATA_WIDTH-1:0] out_channel1_data1;
output [DATA_WIDTH-1:0] out_channel1_data2;
output [DATA_WIDTH-1:0] out_channel2_data1;
output [DATA_WIDTH-1:0] out_channel2_data2;
output out_channel1_data1_valid;
output out_channel1_data2_valid;
output out_channel2_data1_valid;
output out_channel2_data2_valid;

reg clk_5MHz;
reg [5:0] counter;  // 6位计数器，可以计数到63（足够40）
    always @(posedge clk_200MHz or negedge rst_n) begin
        if (!rst_n) begin
            counter <= 0;
            clk_5MHz <= 0;
        end else begin
            counter <= counter + 1;
            if (counter == 19) begin  // 当计数器达到39时翻转输出时钟
                clk_5MHz <= ~clk_5MHz;
                counter <= 0;  // 重置计数器
            end
        end
    end
wire [DATA_WIDTH-1:0] channel_data [0:9]; // 10 channels
wire channel_data_valid [0:9];
wire in_data_valid;
genvar i;

generate
    for( i = 0; i < 10; i = i + 1 )begin
        data_source_serial #(i+1, DATA_WIDTH, START+i, END+i, STRIDE, 1) u_data_source_channel(
            .clk(clk_5MHz),
            .rst_n(rst_n),
            .trigger(trigger),
            .data_out_valid(channel_data_valid[i]),
            .data_out(channel_data[i])
        );
    end
endgenerate
assign in_data_valid = channel_data_valid[0] || channel_data_valid[1] || channel_data_valid[2] || channel_data_valid[3] || channel_data_valid[4] || channel_data_valid[5] || channel_data_valid[6] || channel_data_valid[7] || channel_data_valid[8] || channel_data_valid[9]; 

concatenate #(DATA_WIDTH) u_concatenate(
    .clk(clk_200MHz),
    .rst_n(rst_n),
    .in_data_valid(in_data_valid),
    .in_channel1_data(channel_data[0]),
    .in_channel2_data(channel_data[1]),
    .in_channel3_data(channel_data[2]),
    .in_channel4_data(channel_data[3]),
    .in_channel5_data(channel_data[4]),
    .in_channel6_data(channel_data[5]),
    .in_channel7_data(channel_data[6]),
    .in_channel8_data(channel_data[7]),
    .in_channel9_data(channel_data[8]),
    .in_channel10_data(channel_data[9]),
    
    .out_channel1_data1_valid(out_channel1_data1_valid),
    .out_channel1_data1(out_channel1_data1),
    .out_channel1_data2_valid(out_channel1_data2_valid),
    .out_channel1_data2(out_channel1_data2),
    .out_channel2_data1_valid(out_channel2_data1_valid),
    .out_channel2_data1(out_channel2_data1),
    .out_channel2_data2_valid(out_channel2_data2_valid),
    .out_channel2_data2(out_channel2_data2)
);

endmodule
