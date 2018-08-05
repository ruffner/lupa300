/******************** Source
                      Tree   *******************/

module xillydemo
  (
                //custom defined
                DATA_IMAGE,
                SPI_EN,
                SPI_CLK,                                    //20Mhz
                SPI_DAT,
                LINE_VALID,
                FRAME_VALID,
                LUPA_CLK,                                   //80Mhz
                LUPA_RST,
                INT_TIME1,
                INT_TIME2,
                INT_TIME3,
                LED_debug
  ); 
    //custom defined (IO ports and wires and regs)
    input [9:0] DATA_IMAGE;
    output SPI_EN;
    output SPI_CLK;
    output SPI_DAT;
    input LINE_VALID;
    input FRAME_VALID;
    output LUPA_CLK;
    output LUPA_RST;
    output  INT_TIME1;                            //for Slave mode
    output  INT_TIME2;
    output  INT_TIME3;
    output [3:0] LED_debug;

    wire clk_80, clk_20;
    wire FIFO_WR_EN;
    wire grst;
    wire [3:0] debug;
    wire [9:0] col_no;

    //reg [15:0] Reset_Count;
    reg [2:0] frame_count;
    reg flg, rlg;
    reg [7:0] sync_counter;
    
    reg [9:0] line_cntr;
    reg [20:0] pix_cntr;
    
    //reg Reset_N;
    wire fifo_reset;
    wire [1:0] state;
    wire compare;
    wire [7:0] look_for;
    wire hFRAME_VALID;
    reg done, actv;
    reg Pre_FVAL;
    reg [1:0] mFVL;
    reg [2:0] frame_select;
    reg [5:0] multi_frame;
    reg ready_to_fill;
    reg [15:0] delay_count;
    reg [31:0] puls_count;
    
   // Clock and quiesce
   wire    bus_clk;
   wire    quiesce;

   // Memory arrays
   reg [7:0] demoarray[0:31];
   
   reg [7:0] litearray0[0:31];
   reg [7:0] litearray1[0:31];
   reg [7:0] litearray2[0:31];
   reg [7:0] litearray3[0:31];

   // Wires related to /dev/xillybus_mem_8
   wire      user_r_mem_8_rden;
   wire      user_r_mem_8_empty;
   reg [7:0] user_r_mem_8_data;
   wire      user_r_mem_8_eof;
   wire      user_r_mem_8_open;
   wire      user_w_mem_8_wren;
   wire      user_w_mem_8_full;
   wire [7:0] user_w_mem_8_data;
   wire       user_w_mem_8_open;
   wire [4:0] user_mem_8_addr;
   wire       user_mem_8_addr_update;

   // Wires related to /dev/xillybus_read_32
   wire       user_r_read_32_rden;
   wire       user_r_read_32_empty;
   wire [31:0] user_r_read_32_data;
   wire        user_r_read_32_eof;
   wire        user_r_read_32_open;

   // Wires related to /dev/xillybus_read_8
   wire        user_r_read_8_rden;
   wire        user_r_read_8_empty;
   wire [7:0]  user_r_read_8_data;
   wire        user_r_read_8_eof;
   wire        user_r_read_8_open;

   // Wires related to /dev/xillybus_write_32
   wire        user_w_write_32_wren;
   wire        user_w_write_32_full;
   wire [31:0] user_w_write_32_data;
   wire        user_w_write_32_open;

   // Wires related to /dev/xillybus_write_8
   wire        user_w_write_8_wren;
   wire        user_w_write_8_full;
   wire [7:0]  user_w_write_8_data;
   wire        user_w_write_8_open;

   // Wires related to Xillybus Lite
   wire        user_clk;
   wire        user_wren;
   wire [3:0]  user_wstrb;
   wire        user_rden;
   reg [31:0]  user_rd_data;
   wire [31:0] user_wr_data;
   wire [31:0] user_addr;
   wire        user_irq;
   
   //custom logic (some initial conditions)
   initial
   begin
           //Reset_Count <= 0;
           //Reset_N <= 0;        
           frame_count <= 0;
           ready_to_fill <= 0;
           flg <= 0;
           rlg <= 0;
           done <= 0;
           actv <= 0;
           Pre_FVAL <= 0;
           mFVL <= 0;
           sync_counter <= 1;
           frame_select <= 0;
           multi_frame <= 0;
           
           line_cntr <= 0;
           pix_cntr  <= 0;
           
           delay_count <= 0;
           puls_count <= 0;
   end 

   // Note that none of the ARM processor's direct connections to pads is
   // attached in the instantion below. Normally, they should be connected as
   // toplevel ports here, but that confuses Vivado 2013.4 to think that
   // some of these ports are real I/Os, causing an implementation failure.
   // This detachment results in a lot of warnings during synthesis and
   // implementation, but has no practical significance, as these pads are
   // completely unrelated to the FPGA bitstream.

   xillybus xillybus_ins (

    // Ports related to /dev/xillybus_mem_8
    // FPGA to CPU signals:
    .user_r_mem_8_rden(user_r_mem_8_rden),
    .user_r_mem_8_empty(user_r_mem_8_empty),
    .user_r_mem_8_data(user_r_mem_8_data),
    .user_r_mem_8_eof(user_r_mem_8_eof),
    .user_r_mem_8_open(user_r_mem_8_open),

    // CPU to FPGA signals:
    .user_w_mem_8_wren(user_w_mem_8_wren),
    .user_w_mem_8_full(user_w_mem_8_full),
    .user_w_mem_8_data(user_w_mem_8_data),
    .user_w_mem_8_open(user_w_mem_8_open),

    // Address signals:
    .user_mem_8_addr(user_mem_8_addr),
    .user_mem_8_addr_update(user_mem_8_addr_update),


    // Ports related to /dev/xillybus_read_32
    // FPGA to CPU signals:
    .user_r_read_32_rden(user_r_read_32_rden),
    .user_r_read_32_empty(user_r_read_32_empty),
    .user_r_read_32_data(user_r_read_32_data),
    .user_r_read_32_eof(user_r_read_32_eof),
    .user_r_read_32_open(user_r_read_32_open),


    // Ports related to /dev/xillybus_read_8
    // FPGA to CPU signals:
    .user_r_read_8_rden(user_r_read_8_rden),
    .user_r_read_8_empty(user_r_read_8_empty),
    .user_r_read_8_data(user_r_read_8_data),
    .user_r_read_8_eof(user_r_read_8_eof),
    .user_r_read_8_open(user_r_read_8_open),


    // Ports related to /dev/xillybus_write_32
    // CPU to FPGA signals:
    .user_w_write_32_wren(user_w_write_32_wren),
    .user_w_write_32_full(user_w_write_32_full),
    .user_w_write_32_data(user_w_write_32_data),
    .user_w_write_32_open(user_w_write_32_open),


    // Ports related to /dev/xillybus_write_8
    // CPU to FPGA signals:
    .user_w_write_8_wren(user_w_write_8_wren),
    .user_w_write_8_full(user_w_write_8_full),
    .user_w_write_8_data(user_w_write_8_data),
    .user_w_write_8_open(user_w_write_8_open),

    // Xillybus Lite signals:
    .user_clk ( user_clk ),
    .user_wren ( user_wren ),
    .user_wstrb ( user_wstrb ),
    .user_rden ( user_rden ),
    .user_rd_data ( user_rd_data ),
    .user_wr_data ( user_wr_data ),
    .user_addr ( user_addr ),
    .user_irq ( user_irq ),
			  			  
    // General signals
    .bus_clk(bus_clk),
    .quiesce(quiesce)
  );

   assign      user_irq = 0; // No interrupts for now
   
   always @(posedge user_clk)
     begin
	if (user_wstrb[0])
	  litearray0[user_addr[6:2]] <= user_wr_data[7:0];

	if (user_wstrb[1])
	  litearray1[user_addr[6:2]] <= user_wr_data[15:8];

	if (user_wstrb[2])
	  litearray2[user_addr[6:2]] <= user_wr_data[23:16];

	if (user_wstrb[3])
	  litearray3[user_addr[6:2]] <= user_wr_data[31:24];
	
	if (user_rden)
	  user_rd_data <= { litearray3[user_addr[6:2]],
			    litearray2[user_addr[6:2]],
			    litearray1[user_addr[6:2]],
			    litearray0[user_addr[6:2]] };
     end
   
   // A simple inferred RAM
   always @(posedge bus_clk)
     begin
	if (user_w_mem_8_wren)
	  demoarray[user_mem_8_addr] <= user_w_mem_8_data;
	
	if (user_r_mem_8_rden)
	  user_r_mem_8_data <= demoarray[user_mem_8_addr];	  
     end

   assign  user_r_mem_8_empty = 0;
   assign  user_r_mem_8_eof = 0;
   assign  user_w_mem_8_full = 0;

   // 32-bit loopback
   /*fifo_32x512 fifo_32
     (
      .clk(bus_clk),
      .srst(!user_w_write_32_open && !user_r_read_32_open),
      .din(user_w_write_32_data),
      .wr_en(user_w_write_32_wren),
      .rd_en(user_r_read_32_rden),
      .dout(user_r_read_32_data),
      .full(user_w_write_32_full),
      .empty(user_r_read_32_empty)
      );*/
  

   assign  user_r_read_32_eof = 0;
   
   // 8-bit loopback
   /*fifo_8x2048 fifo_8
     (
      .clk(bus_clk),
      .srst(!user_w_write_8_open && !user_r_read_8_open),
      .din(user_w_write_8_data),
      .wr_en(user_w_write_8_wren),
      .rd_en(user_r_read_8_rden),
      .dout(user_r_read_8_data),
      .full(user_w_write_8_full),
      .empty(user_r_read_8_empty)
      );*/

   fifo_8x2048 fifo_8
     (
      .rst(!user_w_write_8_open && !user_r_read_8_open),                     //fifo_reset
      .wr_clk(clk_80),
      .rd_clk(bus_clk),
      .din(user_w_write_8_data),                //~DATA_IMAGE[9:2]
      .wr_en(FIFO_WR_EN),                   //user_w_write_8_wren
      .rd_en(user_r_read_8_rden),
      .dout(user_r_read_8_data),
      .full(user_w_write_8_full),           //????? not sure how to connect yet
      .empty(user_r_read_8_empty)
      );
      
   //assign user_w_write_8_full = 0;       //copy from xillycapture sample code
   //syncronization
/********************************** Test Use ****************************************/   
    /*fifo_8x2048 fifo_8
        (
         .rst(fifo_reset),
         .wr_clk(clk_80),
         .rd_clk(bus_clk),
         .din(col_no[7:0]),
         .wr_en(ready_to_fill),
         .rd_en(user_r_read_8_rden),
         .dout(user_r_read_8_data),
         .full(user_w_write_8_full),
         .empty(user_r_read_8_empty)
         );*/
         
   fifo_32x512 fifo_32
              (
               .rst(fifo_reset),
               .wr_clk(clk_80),
               .rd_clk(bus_clk),
               .din(~DATA_IMAGE[9:2]),
               .wr_en(FIFO_WR_EN),
               .rd_en(user_r_read_32_rden),
               .dout(user_r_read_32_data),
               .full(user_w_write_32_full),
               .empty(user_r_read_32_empty)
               );      
         
   /*always@(posedge clk_80)
   begin
        if(!ready_to_fill)
                pix_cntr = 0;
        else
        begin
                pix_cntr = pix_cntr + 1;
                if(pix_cntr == 307201)
                        sync_counter = sync_counter + 1;
                else
                        sync_counter = sync_counter;
        end
   end
   assign col_no = pix_cntr % 640;*/
   
   always@(negedge FRAME_VALID)
   begin
           if(frame_count==1)
                sync_counter = sync_counter + 1;
           else
                sync_counter = sync_counter;
   end
   /**************************** End of Test ****************************************/
   
  assign fifo_reset = !grst/*!user_r_read_32_open*/;
 
   
        
   assign  user_r_read_8_eof = 0;
   
   //custom logic starts here
   /*******************************
   ********************************
   ********************************
   ********************************
   ********************************
   *******************************/
   clk_wiz_0  CLK_MANAGE(// Clock in ports
                                   .clk_in1        (bus_clk),             //100Mhz from system bus
                                // Clock out ports
                                   .clk_out1       (clk_80),             //80Mhz
                                   .clk_out2       (clk_20)               //SPI clock below 20Mhz
    );
   
   ISensor_Read LUPA300(
                            .iCLOCK_80      (clk_80),
                            .iCLOCK_20      (clk_20),
                            
                            .SPI_EN         (SPI_EN),
                            .SPI_CLK        (SPI_CLK),
                            .SPI_DAT        (SPI_DAT),
                            
                            .Line_Valid     (LINE_VALID),
                            .Frame_Valid    (FRAME_VALID),
                            .INT_TIME1      (INT_TIME1),
                            .INT_TIME2      (INT_TIME2),
                            .INT_TIME3      (INT_TIME3),
                            
                            .RST_N          (grst),
                            .Qt_ins         (ready_to_fill),
                            .debug          (debug)
                        );
                            
                            
   assign LUPA_CLK = clk_80;
   
   assign LUPA_RST = grst;               //Reset_N  
     
   
   assign FIFO_WR_EN = (state==2) & LINE_VALID &  hFRAME_VALID;      //Master Mode
  //assign FIFO_WR_EN = (frame_count==4) & LINE_VALID & FRAME_VALID & (line_cntr>0) & (line_cntr<=480);
  
  always@(posedge LINE_VALID or negedge FRAME_VALID)
  begin
        if(!FRAME_VALID)
                 line_cntr = 0;
        else
        begin
                if(line_cntr <= 480)
                    line_cntr = line_cntr + 1;
                else
                    line_cntr = line_cntr;
        end    
  end
  
   
   always@(posedge clk_80)
   begin
        if(!fifo_reset)
                flg = 0;
        else
        begin
                if(user_w_write_32_full)
                        flg = 1;
                else
                        flg = flg;
        end
   end
   always@(posedge flg)
   begin
        rlg = 1;
   end
   /***************************** Synchronization ************************************
   ************** only write data to FIFO after read() is called ********************
   ************** write a 0xEE to 3rd address in memory array *********************/
   always@(posedge clk_80)
   begin
            if(demoarray[3]==sync_counter)
                   ready_to_fill = 1;
            else
                   ready_to_fill = 0;
   end
   /*************** State Machine for synchronization *****************************/
   state_machine SYNCHRONIZE(
                   .CLOCK                (clk_80),
                   .mreg                 (demoarray[3]),
                   .DONE                 (done),
                   .sSTART               (actv),
                   .file_open            (user_r_read_32_open),
                   
                   .oSTATE               (state),
                   .compare              (compare),
                   .look_for             (look_for)
   );
   
   always@(posedge clk_80)
   begin
          if(state == 2'b01)
          begin
                if(mFVL == 1)
                        actv = 1;
                else
                        actv =0;
          end
          else
                actv = 0;
   end
   
   always@(posedge clk_80)
   begin
           
           Pre_FVAL    <=    hFRAME_VALID;
           if({Pre_FVAL,hFRAME_VALID}==2'b01)                //rising edge
                   mFVL    <=    1;
           else if({Pre_FVAL, hFRAME_VALID}==2'b10)              //falling edge
                   mFVL    <=    2;
           else
                   mFVL   <=    0;
   end
   
   always@(posedge clk_80)
   begin
        if(state == 2'b10)
        begin
                if(mFVL == 2)
                        done <= 1;
                else
                        done <= 0;
        end
        else
                done <= 0;
   end
   
      
   /***********************************************************************************/
   always@(posedge FRAME_VALID)
        frame_select = frame_select + 1;
        
    assign hFRAME_VALID = (frame_select[1:0] == 2'b00) ? FRAME_VALID : 0;
   
   
   always@(posedge clk_20)
        puls_count = puls_count + 1;
   //assign LED_debug = (debug==4'b1111) ? puls_count[16]: 0;
   //assign LED_debug = {state[1], user_r_read_32_empty, compare};
   assign LED_debug = look_for[3:0];
endmodule
