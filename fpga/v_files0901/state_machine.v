module state_machine(
                CLOCK,
                mreg,
                DONE,
                sSTART,
                file_open,
                
                oSTATE,
                compare,
                look_for
);

input CLOCK;
input [7:0] mreg;
input DONE;
input sSTART;
input file_open;

output [1:0] oSTATE;
output compare;
output [7:0] look_for;
/********************************************************************/
reg [1:0] state, next;
reg [7:0] sync_counter;

initial
begin
        state <= 0;
        next  <= 0;
        
        sync_counter <= 0;
end

/*******************************************************************/
always@(negedge CLOCK)
        state = next;
        
/*always@(*)
begin
        case (state)
            0: 
            1: 
            2: 
            3: 
            default :
        endcase
end*/

always@(posedge CLOCK)
begin
    if(!file_open)
    begin
        sync_counter <= 0;
        next <= 0;
    end
    else
    begin
        case (state)
            0:  begin
                    if(mreg != sync_counter)
                        next <= 1;
                    else
                        next <= 0;
                end
            
            1:  begin
                    if(!sSTART)
                        next <= 1;
                    else
                    begin
                        next <= 2;
                        //sync_counter <= sync_counter + 1;
                    end
                 end
            2:   begin
                     if(!DONE)
                        next <= 2;
                     else
                     begin
                        next <= 0;
                        sync_counter <= sync_counter + 1;
                     end
                 end
            default: next <= 0;
        endcase
    end
end

assign oSTATE = state;
assign compare = (mreg == sync_counter) ? 1 : 0;
assign look_for = sync_counter;

endmodule