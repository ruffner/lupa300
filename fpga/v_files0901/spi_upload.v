`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//				Address			|				Default Value
//				0000				|				000000101001		SEQUENCER [10:0]
//			   0001				|				000000000000		START_X	 [7:0]
//			   0010				|				000000000000		START_Y	 [8:0]
//			   0011				|				000010100000		NB_PIX	 [7:0]
//			   0100				|				000000000010		RES1_LENGTH [11:0]
//			   0101				|				000000000000		RES2_TIMER	[11:0]
//			   0110				|				000000000000		RES3_TIMER	[11:0]
//			   0111				|				000111100001		FT_TIMER		[11:0]
//			   1000				|				000001001010		VCAL			[7:0]
//			   1001				|				000001101011		VBLACK		[7:0]
//			   1010				|				000001010101		VOFFSET		[7:0]
//			   1011				|				000011110000		ANA_IN_ADC	[11:0]
//			   1100				|				111110110000		PGA_SETTING	[11:0]
//			   1101				|				101011011111		CALIB_ADC<11:0>
//			   1110				|				011011011011		CALIB_ADC<23:12>
//			   1111				|				000011011011		CALIB_ADC<8:0>
//////////////////////////////////////////////////////////////////////////////////
module spi_upload(
		clock_20,
		start,
		
		fval,
		advalue,

		spi_clk,
		spi_en,
	   spi_dat,
		
		reset_p,
		cfg_DONE
    );

input clock_20;				     //20Mhz
input start;					 //Trigger start
input fval;
input [15:0] advalue;

output reg spi_clk;
output reg spi_en;
output reg spi_dat;

output reg reset_p;
output reg [1:0] cfg_DONE;

/****************** Reg List *******************/
reg [5:0] state, next;
reg [3:0] addr;
reg [11:0] dat;
//***************** Logic Description ***********
initial
begin
		state <= 0;
		next  <= 0;
		
		//addr <= 4'b0000;
		//dat  <= 12'h028;								//default value at 0000, 12'h029, SLAVE MODE 12'h028
		//addr <= 4'b0100;
		//dat <= 12'h1A5;                               //intergration time = 0.5ms
		//addr <= 4'b0111;                              //integration time = 8.3ms fps = 120
		//dat  <= 12'h3E1;
		addr <= 0;
		dat  <= 0;
		reset_p <= 0;
		cfg_DONE <= 2'b00;
end


always@(negedge clock_20)
		state <= next;
		
always@(*)
begin
		case (state)
			0: begin
					spi_en  <= 0;
					spi_clk <= 0;
					spi_dat <= 1'bz;
				end
			
			1: begin
			         spi_en <= 0;
                     spi_clk <= 0;
                     spi_dat <= 1'bz;
			   end
			2: begin
			         spi_en  <= 0;
			         spi_clk <= 0;
			         spi_dat <= 1'bz;
			   end
			3: begin                                 //give some time to update the advalue
                    spi_en  <= 0;
                    spi_clk <= 0;
                    spi_dat <= 1'bz;
                    
                    /*if(advalue[15:12] == 4'b0000)
                        reset_p <= 1;
                    else
                        reset_p <= 0;*/
               end
			4: begin
			         addr <= advalue[15:12];
			         dat  <= advalue[11:0];
			         
			         spi_en <= 0;
			         spi_clk <= 0;
			   end
			5: begin spi_dat <= addr[3]; spi_en <= 0; spi_clk <= 0; end
			6: begin spi_clk <= 1; spi_en <= 0; end
			7: begin
					spi_clk <= 0;
					spi_dat <= addr[2];
					spi_en <= 0;
				end
			8: begin spi_clk <= 1; spi_en <= 0; end
			9: begin
					spi_clk <= 0;
					spi_dat <= addr[1];
					spi_en <= 0;
				end
			10: begin spi_clk <= 1; spi_en <= 0; end 
			11: begin
					spi_clk <= 0;
					spi_dat <= addr[0];
					spi_en <= 0;
				end
			12: begin spi_clk <= 1; spi_en <= 0; end
			
		  13: begin
					spi_clk <= 0;
					spi_dat <= dat[11];
					spi_en <= 0;
				end
		  14: begin spi_clk <= 1; spi_en <= 0; end
		  15: begin
					spi_clk <= 0;
					spi_dat <= dat[10];
					spi_en <= 0;
				end
		  16: begin spi_clk <= 1; spi_en <= 0; end
		  17: begin
					spi_clk <= 0;
					spi_dat <= dat[9];
					spi_en <= 0;
				end
		  18: begin spi_clk <= 1; spi_en <= 0; end
		  19: begin
					spi_clk <= 0;
					spi_dat <= dat[8];
					spi_en <= 0;
				end
		  20: begin spi_clk <= 1; spi_en <= 0; end
		  21: begin
					spi_clk <= 0;
					spi_dat <= dat[7];
					spi_en <= 0;
				end
		  22: begin spi_clk <= 1; spi_en <= 0; end
		  23: begin
					spi_clk <= 0;
					spi_dat <= dat[6];
					spi_en <= 0;
				end
		  24: begin spi_clk <= 1;  spi_en <= 0; end
		  25: begin
					spi_clk <= 0;
					spi_dat <= dat[5];
					 spi_en <= 0;
				end
		  26: begin spi_clk <= 1;  spi_en <= 0; end
		  27: begin
					spi_clk <= 0;
					spi_dat <= dat[4];
					 spi_en <= 0;
				end
		  28: begin spi_clk <= 1; spi_en <= 0; end
		  29: begin
					spi_clk <= 0;
					spi_dat <= dat[3];
					 spi_en <= 0;
				end
		  30: begin spi_clk <= 1;  spi_en <= 0; end
		  31: begin
					spi_clk <= 0;
					spi_dat <= dat[2];
					spi_en <= 0;
				end
		  32: begin spi_clk <= 1;  spi_en <= 0; end
		  33: begin
					spi_clk <= 0;
					spi_dat <= dat[1];
					spi_en <= 0;
				end
		  34: begin spi_clk <= 1;  spi_en <= 0; end
		  35: begin
					spi_clk <= 0;
					spi_dat <= dat[0];
					 spi_en <= 0;
				end
		  36:begin  spi_clk <= 1;  spi_en <= 0; end
		  
		  37: begin
					spi_clk <= 0;
					spi_en <= 1;
					cfg_DONE[1] <= 1;
				end
		
		  
		  38: begin
					spi_en  <= 0;
					spi_clk <= 0;
					spi_dat <= 1'bz;
					
					//reset_p <= 0;
				end
				
		  39: begin
		                  cfg_DONE[0] <= 1;
		                  spi_en <= 0;
		                  spi_dat <= 1'bz;
		                  spi_clk <= 0;
		      end
		  endcase
end

always@(posedge clock_20 or posedge fval)
begin
		if(fval)
		begin
		      if(state != 39)
		          next <= next;
		      else
		          next<= 3;
		end
		else
		begin
            if(!start)
                next <= 0;
            else
            begin
                    case (state)
                        0: next <= 1;
                        1: next <= 2;
                       39: next <= 39;	
                      default: next <= next + 1;
                    endcase
            end
		end
end

endmodule