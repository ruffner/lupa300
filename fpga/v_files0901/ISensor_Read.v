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
						
						nREG,
						advalue,
						nRC,
						
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

input [7:0] nREG;
input [15:0] advalue;
output reg [7:0] nRC;

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
reg rgSTART, cfSTART;

reg [19:0] cntr_P;
reg Pre_FVAL;
reg mFVAL;

reg [25:0] exp_ctr;
reg Cstart;
reg int_time1;

reg [15:0] lv_cntr;
reg [31:0] fot_timer;

wire int1_reset, int2_reset, int3_reset;
wire int_fot;

wire [1:0] shoot_ready;
wire fval;
wire rst_p;
//*************** Initialize List ******************
initial
begin
		rst_cntr <= 0;
		rst_n    <= 0;
		rgSTART  <= 0;
		cfSTART <= 0;
		nRC <= 0;
		
		cntr_P <= 0;
		
		int_time1 <= 1;
		Cstart <= 0;
		exp_ctr  <= 0;
		
		lv_cntr <= 0;
		fot_timer <= 0;
		
		mFVAL <= 0;
		Pre_FVAL <= 0;
end
/***************************************************
                  Logic Description
********************************************************************/

spi_upload REG_CONFIG(
							.clock_20				 (iCLOCK_20),
							.start					 (cfSTART),				            //0(bypass) rgSTART cfSTART
							
					        .fval                    (fval),                               //number of registers left over
					        .advalue                 (advalue),                                //4-bit addr + 12-bit value

							.spi_clk				 (SPI_CLK),
							.spi_en					 (SPI_EN),
							.spi_dat				 (SPI_DAT),
							
							.reset_p                 (rst_p),
							.cfg_DONE				 (shoot_ready)
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
		if(rst_cntr < 50000)				//VDDD needs 20000 clock cycles to become stable -- 1V/100uS
		begin
					rst_cntr <= rst_cntr + 1;
					rst_n <= 0;
		end
		else if(rst_cntr < 50050)		//it takes another 50 clock cycles to upload the Default value of all 16 regs
		begin
					rst_cntr <= rst_cntr + 1;
					rst_n <= 1;
		end
		else if(rst_cntr < 50060)		//the 2nd rising edge of the iCLOCK_80 after the rising edge of RST_N 
																	//triggers the rising edge of the core(internal) clock
					rst_cntr = rst_cntr + 1;
		else
		begin
					if(rst_cntr < 3510000000)
						rst_cntr = rst_cntr + 1;
					else
						rst_cntr = 3510000000;
		end
end
//Edit the registers only when FRAME_VALID is low
always@(negedge Frame_Valid)
begin
        if(rst_cntr > 500000)
            rgSTART <= 1;
        else
            rgSTART <= 0;
end
always@(negedge Frame_Valid)
begin
    if(nREG != 0)
        cfSTART = 1;
    else
        cfSTART = 0;
end

always@(posedge iCLOCK_80)
 begin         
       Pre_FVAL    <=    Frame_Valid;
       if({Pre_FVAL, Frame_Valid}==2'b10)              //falling edge
               mFVAL    <=   1;
       else
               mFVAL   <=    0;
 end
 assign fval = ((nRC < nREG)/* && (cfSTART != 0)*/) ? mFVAL : 0;
always@(posedge Frame_Valid)
begin
    if(!cfSTART)
        nRC <= 0;
    else
    begin
        if(nRC < 16)
            nRC <= nRC + 1;
        else
            nRC <= 16;
    end
end


assign RST_N = rst_n /*& (~rst_p)*/;                                         //Master Mode
//assign RSR_N = rst_n & (~rgSTART) | shoot_ready[0];
//assign debug = {rst_n, rgSTART, shoot_ready};
//Hold the RST_N low for a minimum of 50nS, the SEQUENCER will be reset

//*******************************************************************************
/*always@(posedge iCLOCK_80)
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
end*/
//********************* Row Overhead Elimination ***********************************
	
always@(negedge Line_Valid)
begin
		if((shoot_ready!=2'b11)/* || (Qt_ins==0)*/)
		          Cstart = 0;                                     //Start to count
		else
		          Cstart = 1;
end

/****************** Readout time longer than Intergration time *********************/

/****************** Readout time shorter than Intergration time *******************************/
always@(posedge iCLOCK_80)
begin
        if(Cstart)
        begin
                if(exp_ctr < 321560 + 624)
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
                if(exp_ctr >= 127200)
                        int_time1 = 1;
                else
                        int_time1 = 0;
        end
end
assign fot_now3 =  (exp_ctr > 321560) ? 0 : 1;
assign fot_now2 =  (exp_ctr > 321560) ? 0 : 1;
assign fot_now1 = ((Cstart == 1) && ((exp_ctr > 321560) || (exp_ctr<= 30160))) ? 0 : 1;

assign INT_TIME1 = fot_now1;
assign INT_TIME2 = fot_now2;
assign INT_TIME3 = fot_now3;
/*************************************************************************************/
/*always@(posedge iCLOCK_80)
   begin
           
           Pre_LVAL    <=    Line_Valid;
           if({Pre_LVAL,Line_Valid}==2'b10)              //falling edge
                   mLVAL    <=   1;
           else
                   mLVAL   <=    0;
   end

always@(posedge iCLOCK_80)
begin
    if(!Cstart)
    begin
        fot_timer <= 0;
        lv_cntr <= 0;
    end
    else if(mLVAL)
    begin
        if(lv_cntr < 699)
        begin
            fot_timer <= 0;
            lv_cntr <= lv_cntr + 1;;
        end
        else if(lv_cntr == 700)
        begin
            fot_timer <= fot_timer + 1;
            lv_cntr <= lv_cntr + 1;
        end
        else                                //if case an Line_Valid falling edge happens during FOT
        begin
            fot_timer <= fot_timer + 1;
            lv_cntr <= lv_cntr;
        end    
    end
    else
    begin
        if(lv_cntr == 700)
        begin
            if(fot_timer < 624)
            begin
                fot_timer <= fot_timer + 1;
                lv_cntr <= lv_cntr;
            end
            else
            begin
                fot_timer <= 0;
                lv_cntr <= 0;
            end
        end
        else
        begin
            fot_timer <= 0;
            lv_cntr <= lv_cntr;
        end
    end
end
assign int_fot = (fot_timer > 0) && (fot_timer < 624) && Cstart;

assign int1_reset = ((Cstart == 1) && (lv_cntr < 2) && (lv_cntr > 0)) ? 0 : 1;
assign int2_reset = 1;
assign int3_reset = 1;
assign INT_TIME1 = int1_reset & (~int_fot);
assign INT_TIME2 = ~int_fot;
assign INT_TIME3 = ~int_fot;*/
endmodule
