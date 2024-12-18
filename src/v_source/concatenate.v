`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/12/16 13:36:40
// Design Name: 
// Module Name: concatenate
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


module concatenate(
    clk,
    rst_n,
    in_data_valid,
    in_channel1_data,
    in_channel2_data,
    in_channel3_data,
    in_channel4_data,
    in_channel5_data,
    in_channel6_data,
    in_channel7_data,
    in_channel8_data,
    in_channel9_data,
    in_channel10_data,
    
    out_channel1_data1_valid,
    out_channel1_data1,
    out_channel1_data2_valid,
    out_channel1_data2,
    out_channel2_data1_valid,
    out_channel2_data1,
    out_channel2_data2_valid,
    out_channel2_data2
    );
    
parameter DATA_WIDTH = 16;

input clk;
input rst_n;
input in_data_valid;
input [DATA_WIDTH-1:0] in_channel1_data;
input [DATA_WIDTH-1:0] in_channel2_data;
input [DATA_WIDTH-1:0] in_channel3_data;
input [DATA_WIDTH-1:0] in_channel4_data;
input [DATA_WIDTH-1:0] in_channel5_data;
input [DATA_WIDTH-1:0] in_channel6_data;
input [DATA_WIDTH-1:0] in_channel7_data;
input [DATA_WIDTH-1:0] in_channel8_data;
input [DATA_WIDTH-1:0] in_channel9_data;
input [DATA_WIDTH-1:0] in_channel10_data;
    
output out_channel1_data1_valid;
output reg [DATA_WIDTH-1:0] out_channel1_data1;
output out_channel1_data2_valid;
output reg [DATA_WIDTH-1:0] out_channel1_data2;
output out_channel2_data1_valid;
output reg [DATA_WIDTH-1:0] out_channel2_data1;
output out_channel2_data2_valid;
output reg [DATA_WIDTH-1:0] out_channel2_data2;

reg in_data_valid_reg;
reg [DATA_WIDTH-1:0] in_channel1_data_reg;
reg [DATA_WIDTH-1:0] in_channel2_data_reg;
reg [DATA_WIDTH-1:0] in_channel3_data_reg;
reg [DATA_WIDTH-1:0] in_channel4_data_reg;
reg [DATA_WIDTH-1:0] in_channel5_data_reg;
reg [DATA_WIDTH-1:0] in_channel6_data_reg;
reg [DATA_WIDTH-1:0] in_channel7_data_reg;
reg [DATA_WIDTH-1:0] in_channel8_data_reg;
reg [DATA_WIDTH-1:0] in_channel9_data_reg;
reg [DATA_WIDTH-1:0] in_channel10_data_reg;
reg [3:0] out_channel1_data1_sel [0:26];
reg [3:0] out_channel1_data2_sel [0:26];
reg [3:0] out_channel2_data1_sel [0:27];
reg [3:0] out_channel2_data2_sel [0:27];
wire [3:0] out_chn1_data1_sel;
wire [3:0] out_chn1_data2_sel;
wire [3:0] out_chn2_data1_sel;
wire [3:0] out_chn2_data2_sel;
reg [4:0] chn_sel_addr1;
reg [4:0] chn_sel_addr2;

// register the input data
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        in_data_valid_reg <= 1'b0;
        in_channel1_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel2_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel3_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel4_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel5_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel6_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel7_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel8_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel9_data_reg <= {DATA_WIDTH{1'b0}};
        in_channel10_data_reg <= {DATA_WIDTH{1'b0}};
    end
    else begin
        in_data_valid_reg <= in_data_valid;
        in_channel1_data_reg <= in_channel1_data;
        in_channel2_data_reg <= in_channel2_data;
        in_channel3_data_reg <= in_channel3_data;
        in_channel4_data_reg <= in_channel4_data;
        in_channel5_data_reg <= in_channel5_data;
        in_channel6_data_reg <= in_channel6_data;
        in_channel7_data_reg <= in_channel7_data;
        in_channel8_data_reg <= in_channel8_data;
        in_channel9_data_reg <= in_channel9_data;
        in_channel10_data_reg <= in_channel10_data;
    end
end

// generate output data valid
assign out_channel1_data1_valid = (chn_sel_addr1 < 5'd27)? in_data_valid_reg : 1'b0;
assign out_channel1_data2_valid = (chn_sel_addr1 < 5'd27)? in_data_valid_reg : 1'b0;
assign out_channel2_data1_valid = (chn_sel_addr2 < 5'd28)? in_data_valid_reg : 1'b0;
assign out_channel2_data2_valid = (chn_sel_addr2 < 5'd28)? in_data_valid_reg : 1'b0;

// generate output data
always @(posedge clk or negedge rst_n) begin
    if(!rst_n || (!in_data_valid_reg)) begin 
        chn_sel_addr1 <= 5'd0;
        chn_sel_addr2 <= 5'd0;
    end
    else begin
        chn_sel_addr1 <= chn_sel_addr1 + 5'd1;
        chn_sel_addr2 <= chn_sel_addr2 + 5'd1;
    end
end
assign out_chn1_data1_sel = out_channel1_data1_sel[chn_sel_addr1];
assign out_chn1_data2_sel = out_channel1_data2_sel[chn_sel_addr1];
assign out_chn2_data1_sel = out_channel2_data1_sel[chn_sel_addr2];
assign out_chn2_data2_sel = out_channel2_data2_sel[chn_sel_addr2];

always@(*) begin
    case (out_chn1_data1_sel)
        4'd1: out_channel1_data1 = in_channel1_data_reg;
        4'd2: out_channel1_data1 = in_channel2_data_reg;
        4'd3: out_channel1_data1 = in_channel3_data_reg;
        default: out_channel1_data1 = in_channel1_data_reg;
    endcase
end

always@(*) begin
    case (out_chn1_data2_sel)
        4'd1: out_channel1_data2 = in_channel1_data_reg;
        4'd2: out_channel1_data2 = in_channel2_data_reg;
        4'd3: out_channel1_data2 = in_channel3_data_reg;
        4'd4: out_channel1_data2 = in_channel4_data_reg;
        4'd5: out_channel1_data2 = in_channel5_data_reg;
        4'd6: out_channel1_data2 = in_channel6_data_reg;
        4'd7: out_channel1_data2 = in_channel7_data_reg;
        4'd8: out_channel1_data2 = in_channel8_data_reg;
        4'd9: out_channel1_data2 = in_channel9_data_reg;
        4'd10: out_channel1_data2 = in_channel10_data_reg;
        default: out_channel1_data2 = in_channel1_data_reg;
    endcase
end

always@(*) begin
    case (out_chn2_data1_sel)
        4'd4: out_channel2_data1 = in_channel4_data_reg;
        4'd5: out_channel2_data1 = in_channel5_data_reg;
        4'd6: out_channel2_data1 = in_channel6_data_reg;
        4'd7: out_channel2_data1 = in_channel7_data_reg;
        4'd8: out_channel2_data1 = in_channel8_data_reg;
        4'd9: out_channel2_data1 = in_channel9_data_reg;
        4'd10: out_channel2_data1 = in_channel10_data_reg;
        default: out_channel2_data1 = in_channel4_data_reg;
    endcase
end

always@(*) begin
    case (out_chn2_data2_sel)
        4'd4: out_channel2_data2 = in_channel4_data_reg;
        4'd5: out_channel2_data2 = in_channel5_data_reg;
        4'd6: out_channel2_data2 = in_channel6_data_reg;
        4'd7: out_channel2_data2 = in_channel7_data_reg;
        4'd8: out_channel2_data2 = in_channel8_data_reg;
        4'd9: out_channel2_data2 = in_channel9_data_reg;
        4'd10: out_channel2_data2 = in_channel10_data_reg;
        default: out_channel2_data2 = in_channel4_data_reg;
    endcase
end

// some default values
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        out_channel1_data1_sel[0] <= 4'd0;
        out_channel1_data1_sel[1] <= 4'd0;
        out_channel1_data1_sel[2] <= 4'd0;
        out_channel1_data1_sel[3] <= 4'd0;
        out_channel1_data1_sel[4] <= 4'd0;
        out_channel1_data1_sel[5] <= 4'd0;
        out_channel1_data1_sel[6] <= 4'd0;
        out_channel1_data1_sel[7] <= 4'd0;
        out_channel1_data1_sel[8] <= 4'd0;
        out_channel1_data1_sel[9] <= 4'd0;
        out_channel1_data1_sel[10] <= 4'd0;
        out_channel1_data1_sel[11] <= 4'd0;
        out_channel1_data1_sel[12] <= 4'd0;
        out_channel1_data1_sel[13] <= 4'd0;
        out_channel1_data1_sel[14] <= 4'd0;
        out_channel1_data1_sel[15] <= 4'd0;
        out_channel1_data1_sel[16] <= 4'd0;
        out_channel1_data1_sel[17] <= 4'd0;     
        out_channel1_data1_sel[18] <= 4'd0;
        out_channel1_data1_sel[19] <= 4'd0;
        out_channel1_data1_sel[20] <= 4'd0;
        out_channel1_data1_sel[21] <= 4'd0;     
        out_channel1_data1_sel[22] <= 4'd0;
        out_channel1_data1_sel[23] <= 4'd0;
        out_channel1_data1_sel[24] <= 4'd0;
        out_channel1_data1_sel[25] <= 4'd0;  
        out_channel1_data1_sel[26] <= 4'd0;        

        out_channel1_data2_sel[0] <= 4'd0;
        out_channel1_data2_sel[1] <= 4'd0;
        out_channel1_data2_sel[2] <= 4'd0;
        out_channel1_data2_sel[3] <= 4'd0;
        out_channel1_data2_sel[4] <= 4'd0;
        out_channel1_data2_sel[5] <= 4'd0;
        out_channel1_data2_sel[6] <= 4'd0;
        out_channel1_data2_sel[7] <= 4'd0;
        out_channel1_data2_sel[8] <= 4'd0;
        out_channel1_data2_sel[9] <= 4'd0;
        out_channel1_data2_sel[10] <= 4'd0;
        out_channel1_data2_sel[11] <= 4'd0;
        out_channel1_data2_sel[12] <= 4'd0;
        out_channel1_data2_sel[13] <= 4'd0;
        out_channel1_data2_sel[14] <= 4'd0;
        out_channel1_data2_sel[15] <= 4'd0;
        out_channel1_data2_sel[16] <= 4'd0;
        out_channel1_data2_sel[17] <= 4'd0;     
        out_channel1_data2_sel[18] <= 4'd0;
        out_channel1_data2_sel[19] <= 4'd0;
        out_channel1_data2_sel[20] <= 4'd0;
        out_channel1_data2_sel[21] <= 4'd0;     
        out_channel1_data2_sel[22] <= 4'd0;
        out_channel1_data2_sel[23] <= 4'd0;
        out_channel1_data2_sel[24] <= 4'd0;
        out_channel1_data2_sel[25] <= 4'd0;  
        out_channel1_data2_sel[26] <= 4'd0;        

        out_channel2_data1_sel[0] <= 4'd0;
        out_channel2_data1_sel[1] <= 4'd0;
        out_channel2_data1_sel[2] <= 4'd0;
        out_channel2_data1_sel[3] <= 4'd0;
        out_channel2_data1_sel[4] <= 4'd0;
        out_channel2_data1_sel[5] <= 4'd0;
        out_channel2_data1_sel[6] <= 4'd0;
        out_channel2_data1_sel[7] <= 4'd0;
        out_channel2_data1_sel[8] <= 4'd0;
        out_channel2_data1_sel[9] <= 4'd0;
        out_channel2_data1_sel[10] <= 4'd0;
        out_channel2_data1_sel[11] <= 4'd0;
        out_channel2_data1_sel[12] <= 4'd0;
        out_channel2_data1_sel[13] <= 4'd0;
        out_channel2_data1_sel[14] <= 4'd0;
        out_channel2_data1_sel[15] <= 4'd0;
        out_channel2_data1_sel[16] <= 4'd0;
        out_channel2_data1_sel[17] <= 4'd0;     
        out_channel2_data1_sel[18] <= 4'd0;
        out_channel2_data1_sel[19] <= 4'd0;
        out_channel2_data1_sel[20] <= 4'd0;
        out_channel2_data1_sel[21] <= 4'd0;     
        out_channel2_data1_sel[22] <= 4'd0;
        out_channel2_data1_sel[23] <= 4'd0;
        out_channel2_data1_sel[24] <= 4'd0;
        out_channel2_data1_sel[25] <= 4'd0;  
        out_channel2_data1_sel[26] <= 4'd0;        

        out_channel2_data2_sel[0] <= 4'd0;
        out_channel2_data2_sel[1] <= 4'd0;
        out_channel2_data2_sel[2] <= 4'd0;
        out_channel2_data2_sel[3] <= 4'd0;
        out_channel2_data2_sel[4] <= 4'd0;
        out_channel2_data2_sel[5] <= 4'd0;
        out_channel2_data2_sel[6] <= 4'd0;
        out_channel2_data2_sel[7] <= 4'd0;
        out_channel2_data2_sel[8] <= 4'd0;
        out_channel2_data2_sel[9] <= 4'd0;
        out_channel2_data2_sel[10] <= 4'd0;
        out_channel2_data2_sel[11] <= 4'd0;
        out_channel2_data2_sel[12] <= 4'd0;
        out_channel2_data2_sel[13] <= 4'd0;
        out_channel2_data2_sel[14] <= 4'd0;
        out_channel2_data2_sel[15] <= 4'd0;
        out_channel2_data2_sel[16] <= 4'd0;
        out_channel2_data2_sel[17] <= 4'd0;     
        out_channel2_data2_sel[18] <= 4'd0;
        out_channel2_data2_sel[19] <= 4'd0;
        out_channel2_data2_sel[20] <= 4'd0;
        out_channel2_data2_sel[21] <= 4'd0;     
        out_channel2_data2_sel[22] <= 4'd0;
        out_channel2_data2_sel[23] <= 4'd0;
        out_channel2_data2_sel[24] <= 4'd0;
        out_channel2_data2_sel[25] <= 4'd0;  
        out_channel2_data2_sel[26] <= 4'd0;        
    end 
    else begin
        out_channel1_data1_sel[0] <= 4'd1;
        out_channel1_data1_sel[1] <= 4'd1;
        out_channel1_data1_sel[2] <= 4'd1;
        out_channel1_data1_sel[3] <= 4'd1;
        out_channel1_data1_sel[4] <= 4'd1;
        out_channel1_data1_sel[5] <= 4'd1;
        out_channel1_data1_sel[6] <= 4'd1;
        out_channel1_data1_sel[7] <= 4'd1;
        out_channel1_data1_sel[8] <= 4'd1;
        out_channel1_data1_sel[9] <= 4'd1;
        out_channel1_data1_sel[10] <= 4'd2;
        out_channel1_data1_sel[11] <= 4'd2;
        out_channel1_data1_sel[12] <= 4'd2;
        out_channel1_data1_sel[13] <= 4'd2;
        out_channel1_data1_sel[14] <= 4'd2;
        out_channel1_data1_sel[15] <= 4'd2;
        out_channel1_data1_sel[16] <= 4'd2;
        out_channel1_data1_sel[17] <= 4'd2;     
        out_channel1_data1_sel[18] <= 4'd2;
        out_channel1_data1_sel[19] <= 4'd3;
        out_channel1_data1_sel[20] <= 4'd3;
        out_channel1_data1_sel[21] <= 4'd3;     
        out_channel1_data1_sel[22] <= 4'd3;
        out_channel1_data1_sel[23] <= 4'd3;
        out_channel1_data1_sel[24] <= 4'd3;
        out_channel1_data1_sel[25] <= 4'd3;  
        out_channel1_data1_sel[26] <= 4'd3;        

        out_channel1_data2_sel[0] <= 4'd1;
        out_channel1_data2_sel[1] <= 4'd2;
        out_channel1_data2_sel[2] <= 4'd3;
        out_channel1_data2_sel[3] <= 4'd4;
        out_channel1_data2_sel[4] <= 4'd5;
        out_channel1_data2_sel[5] <= 4'd6;
        out_channel1_data2_sel[6] <= 4'd7;
        out_channel1_data2_sel[7] <= 4'd8;
        out_channel1_data2_sel[8] <= 4'd9;
        out_channel1_data2_sel[9] <= 4'd10;
        out_channel1_data2_sel[10] <= 4'd2;
        out_channel1_data2_sel[11] <= 4'd3;
        out_channel1_data2_sel[12] <= 4'd4;
        out_channel1_data2_sel[13] <= 4'd5;
        out_channel1_data2_sel[14] <= 4'd6;
        out_channel1_data2_sel[15] <= 4'd7;
        out_channel1_data2_sel[16] <= 4'd8;
        out_channel1_data2_sel[17] <= 4'd9;     
        out_channel1_data2_sel[18] <= 4'd10;
        out_channel1_data2_sel[19] <= 4'd3;
        out_channel1_data2_sel[20] <= 4'd4;
        out_channel1_data2_sel[21] <= 4'd5;     
        out_channel1_data2_sel[22] <= 4'd6;
        out_channel1_data2_sel[23] <= 4'd7;
        out_channel1_data2_sel[24] <= 4'd8;
        out_channel1_data2_sel[25] <= 4'd9;  
        out_channel1_data2_sel[26] <= 4'd10;        

        out_channel2_data1_sel[0] <= 4'd4;
        out_channel2_data1_sel[1] <= 4'd4;
        out_channel2_data1_sel[2] <= 4'd4;
        out_channel2_data1_sel[3] <= 4'd4;
        out_channel2_data1_sel[4] <= 4'd4;
        out_channel2_data1_sel[5] <= 4'd4;
        out_channel2_data1_sel[6] <= 4'd4;
        out_channel2_data1_sel[7] <= 4'd5;
        out_channel2_data1_sel[8] <= 4'd5;
        out_channel2_data1_sel[9] <= 4'd5;
        out_channel2_data1_sel[10] <= 4'd5;
        out_channel2_data1_sel[11] <= 4'd5;
        out_channel2_data1_sel[12] <= 4'd5;
        out_channel2_data1_sel[13] <= 4'd6;
        out_channel2_data1_sel[14] <= 4'd6;
        out_channel2_data1_sel[15] <= 4'd6;
        out_channel2_data1_sel[16] <= 4'd6;
        out_channel2_data1_sel[17] <= 4'd6;     
        out_channel2_data1_sel[18] <= 4'd7;
        out_channel2_data1_sel[19] <= 4'd7;
        out_channel2_data1_sel[20] <= 4'd7;
        out_channel2_data1_sel[21] <= 4'd7;     
        out_channel2_data1_sel[22] <= 4'd8;
        out_channel2_data1_sel[23] <= 4'd8;
        out_channel2_data1_sel[24] <= 4'd8;
        out_channel2_data1_sel[25] <= 4'd9;  
        out_channel2_data1_sel[26] <= 4'd9;       
        out_channel2_data1_sel[27] <= 4'd10;  

        out_channel2_data2_sel[0] <= 4'd4;
        out_channel2_data2_sel[1] <= 4'd5;
        out_channel2_data2_sel[2] <= 4'd6;
        out_channel2_data2_sel[3] <= 4'd7;
        out_channel2_data2_sel[4] <= 4'd8;
        out_channel2_data2_sel[5] <= 4'd9;
        out_channel2_data2_sel[6] <= 4'd10;
        out_channel2_data2_sel[7] <= 4'd5;
        out_channel2_data2_sel[8] <= 4'd6;
        out_channel2_data2_sel[9] <= 4'd7;
        out_channel2_data2_sel[10] <= 4'd8;
        out_channel2_data2_sel[11] <= 4'd9;
        out_channel2_data2_sel[12] <= 4'd10;
        out_channel2_data2_sel[13] <= 4'd6;
        out_channel2_data2_sel[14] <= 4'd7;
        out_channel2_data2_sel[15] <= 4'd8;
        out_channel2_data2_sel[16] <= 4'd9;
        out_channel2_data2_sel[17] <= 4'd10;     
        out_channel2_data2_sel[18] <= 4'd7;
        out_channel2_data2_sel[19] <= 4'd8;
        out_channel2_data2_sel[20] <= 4'd9;
        out_channel2_data2_sel[21] <= 4'd10;     
        out_channel2_data2_sel[22] <= 4'd8;
        out_channel2_data2_sel[23] <= 4'd9;
        out_channel2_data2_sel[24] <= 4'd10;
        out_channel2_data2_sel[25] <= 4'd9;  
        out_channel2_data2_sel[26] <= 4'd10;         
        out_channel2_data2_sel[27] <= 4'd10;  
    end
end
endmodule
