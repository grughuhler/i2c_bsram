// Single-port RAM, synchronous read via inference and direct instantiation

// Copyright (C) 2024 Grug Huhler YouTube Channel
//
// This source file may be used and distributed without
// restriction provided that this copyright statement is not
// removed from the file and that any derivative work contains
// the original copyright notice and the associated disclaimer.
//
// This source file is free software; you can redistribute it
// and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any
// later version.
//
// This source is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more
// details. 
// 
// You should have received a copy of the GNU Lesser General 
// Public License along with this source; if not, download it
// from <http://www.opencores.org/lgpl.shtml>

module bsram (
	      input 	   clk,
	      input 	   rst,
	      input 	   ce, 
	      input 	   wre,
	      input [13:0] addr,
	      input [7:0]  data_in,
	      output [7:0] data_out
	       );

// Define one of DIRECT, FROMDOC, or GENERIC
`define DIRECT

`ifdef GENERIC
   reg [7:0] 		   mem[16383:0];
   reg [13:0] 		   read_addr;

   always @(posedge clk) begin
      if (wre)
	mem[addr] <= data_in;
      read_addr <= addr;
   end

   assign data_out = mem[read_addr];

`elsif FROMDOC
   reg [7:0] 		   mem[16383:0];
   reg [7:0] 		   data_out_reg;

   always @(posedge clk or posedge rst)
     if (rst)
       data_out_reg <= 0;
     else
       if (ce & !wre)
	 data_out_reg <= mem[addr];

   always @(posedge clk)
     if (ce & wre)
       mem[addr] <= data_in;
   
   assign data_out = data_out_reg;

`elsif DIRECT

   Gowin_SP gmem (
		  .dout(data_out),
		  .clk(clk),
		  .oce(1'b1),
		  .ce(ce),
		  .reset(rst),
		  .wre(wre),
		  .ad(addr),
		  .din(data_in)
		  );

`endif
endmodule // bsram

   
