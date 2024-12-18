`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/12/16 15:37:19
// Design Name: 
// Module Name: data_source_serial
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


module data_source_serial(
    clk,
    rst_n,
    trigger,
    data_out_valid,
    data_out
    );
    
parameter CHANNEL_ID = 1;
parameter DATA_WIDTH = 16;
parameter START = -512;
parameter END = 512;
parameter STRIDE = 1;
parameter CONSTANT = 1;

input clk;
input rst_n;
input trigger;
output reg data_out_valid;
output reg [DATA_WIDTH-1:0] data_out;

reg [9:0] data; // data from 0-1023

always @(posedge clk or negedge rst_n) begin
    if(!rst_n || (!trigger)) 
        data <= START;
    else if(data == END)
        data <= START;
    else begin
        data <= data + STRIDE; 
    end
end

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)
        data_out <= {DATA_WIDTH{1'b0}};
    else if(CONSTANT == 1)
        data_out <= CHANNEL_ID;
    else
        data_out <= data;
end

always @(posedge clk or negedge rst_n) begin
    if(!rst_n || (!trigger)) begin
        data_out_valid <= 1'b0;
    end 
    else begin
        data_out_valid <= 1'b1; 
    end
end

endmodule
