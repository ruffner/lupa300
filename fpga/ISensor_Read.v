`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    16:46:23 01/14/2016 
// Design Name: 
// Module Name:    ISensor_Read 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module ISensor_Read(
						iCLOCK_80,
						iCLOCK_20,
						
						//
						SPI_EN,
						SPI_CLK,
						SPI_DAT,
						
						Line_Valid,
						Frame_Valid,
						INT_TIME1,
						INT_TIME2,
						INT_TIME3,
						
						RST_N,
						Qt_ins,
						debug,
    );
//*************** Port Declaration ****************
input iCLOCK_80;								//from top module, because the PLL is there
input iCLOCK_20;


output SPI_EN;									//Low Active
output SPI_CLK;								//Lower than 20Mhz
output SPI_DAT;								//UPLOAD

input Line_Valid;
input Frame_Valid;
output INT_TIME1;								//NC in Master mode
output INT_TIME2;								//NC in Master mode
output INT_TIME3;								//NC in Master mode

output RST_N;
input  Qt_ins;
output [3:0] debug;
//*************** Reg List *************************
reg rst_n;
reg [31:0] rst_cntr;
reg rgSTART;

reg [19:0] cntr_P;
reg Pre_FVAL;
reg mFVAL, mLVAL;

reg [25:0] exp_ctr;
reg Cstart;
reg int_time1;

wire fot_now1, fot_now2, fot_now3;
wire fot_end;

wire [1:0] shoot_ready;
//*************** Initialize List ******************
initial
begin
		rst_cntr <= 0;
		rst_n    <= 0;
		rgSTART  <= 0;
		
		cntr_P <= 0;
		
		int_time1 <= 1;
		Cstart <= 0;
		exp_ctr  <= 0;
end
/***************************************************
                  Logic Description
********************************************************************/

spi_upload REG_CONFIG(
							.clock_20				(iCLOCK_20),
							.start					(rgSTART),				            //0(bypass) rgSTART
							.nrg					(10),								//edit 16, the other 10 regs remain the default

							.spi_clk				(SPI_CLK),
							.spi_en					(SPI_EN),
							.spi_dat				(SPI_DAT),
							
							.cfg_DONE				(shoot_ready)
					 );					 
/************************** Readout Timing **************************/
always@(posedge iCLOCK_80 or negedge Frame_Valid)
begin
		if(!Frame_Valid)
				cntr_P = 0;
		else if(Frame_Valid && Line_Valid)
				cntr_P = cntr_P + 1;
		else
				cntr_P = cntr_P;
end

//////////////////////////////////////////////////////////////////////
//LUPA300 RESET CIRCUITRY
always@(posedge iCLOCK_80)
begin
		if(rst_cntr < 40000)				//VDDD needs 20000 clock cycles to become stable -- 1V/100uS
		begin
					rst_cntr <= rst_cntr + 1;
					rst_n <= 0;
		end
		else if(rst_cntr < 40050)		//it takes another 50 clock cycles to upload the Default value of all 16 regs
		begin
					rst_cntr <= rst_cntr + 1;
					rst_n <= 1;
		end
		else if(rst_cntr < 40060)		//the 2nd rising edge of the iCLOCK_80 after the rising edge of RST_N 
																	//triggers the rising edge of the core(internal) clock
					rst_cntr = rst_cntr + 1;
		else
		begin
					if(rst_cntr < 41000)
						rst_cntr = rst_cntr + 1;
					else
						rst_cntr = 41000;
		end
end
//(negedge Frame_Valid) Edit the registers only when FRAME_VALID is low
always@(negedge Frame_Valid)
begin
        if(rst_cntr>40055)
                rgSTART <= 1;
        else
                rgSTART <= 0;
end
assign RST_N = rst_n/* & (!rgSTART) | (shoot_ready==2'b11)*/;
assign debug = {rst_n, rgSTART, shoot_ready};
//Hold the RST_N low for a minimum of 50nS, the SEQUENCER will be reset
always@(posedge iCLOCK_80)
begin
		if(!rst_n)
		begin
				Pre_FVAL <= 0;
				mFVAL    <= 0;
				mLVAL		<= 0;
		end
		else
		begin
				Pre_FVAL	<=	Frame_Valid;
				if({Pre_FVAL,Frame_Valid}==2'b01)
						mFVAL	<=	1;
				else if({Pre_FVAL,Frame_Valid}==2'b10)
						mFVAL	<=	0;
				mLVAL	<=	Line_Valid;
		end
end
//********************* Row Overhead Elimination ***********************************
	
always@(negedge Line_Valid)
begin
		if((shoot_ready!=2'b11)/* || (Qt_ins==0)*/)
		          Cstart = 0;                                     //Start to count
		else
		          Cstart = 1;
end

/****************** Readout time longer than Intergration time *********************/
/*always@(posedge iCLOCK_80)
begin
		if(Cstart)
		begin
		       if(exp_ctr < 323184)
			         exp_ctr = exp_ctr + 1;
			   else
			         exp_ctr = 0;      
	    end
		else
			exp_ctr = 0;
end
assign fot_now = ((exp_ctr>0)&&(exp_ctr<=624)) ? 0 : 1;
always@(negedge Line_Valid or posedge Cstart)
begin
		if(Cstart)
		begin
		      if((exp_ctr > 624) && (exp_ctr <= 243640))
		              int_time1 = 0;
		      else
		              int_time1 = 1;
		end
		else
		      int_time1 = 1;
end
*/
/****************** Readout time shorter than Intergration time *******************************/
always@(posedge iCLOCK_80)
begin
        if(Cstart)
        begin
                if(exp_ctr < 450000)
                        exp_ctr = exp_ctr + 1;
                else
                        exp_ctr = 0;
        end
        else
                        exp_ctr = 0;
end
always@(negedge Line_Valid)
begin
        if(!Cstart)
                int_time1 = 1;
        else
        begin
                if((exp_ctr >= 652) && (exp_ctr < 450000-624))              //172
                        int_time1 = 1;
                else
                        int_time1 = 0;
        end
end
assign fot_now3 =  ((exp_ctr > 450000-624) || ((exp_ctr > 430000) && (exp_ctr < 430300))) ? 0 : 1;
assign fot_now2 =  ((exp_ctr > 450000-624) || ((exp_ctr > 400000) && (exp_ctr < 400300))) ? 0 : 1;
assign fot_now1 = ((exp_ctr > 450000-624) || (exp_ctr < 652)) ? 0 : 1;
assign fot_end = (exp_ctr<=300) ? 1 : 0;
//assign fot_end = (exp_ctr < 672) ? 1 : 0;
/********************************************************************************************/

assign INT_TIME1 = int_time1 & fot_now1 | (!Cstart)/* | fot_end*/;
assign INT_TIME2 = fot_now2;
assign INT_TIME3 = fot_now3;
endmodule
