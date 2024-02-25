/*

	ODDF - Open Digital Design Framework
	Copyright Advantest Corporation
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

/*

	FIFO module that contains a memory with one clock cycle of output
	delay.

*/

#pragma once

#include "../bus.h"
#include "../inout.h"

namespace dfx {
namespace modules {

template<typename T, int addressWidth> class BusFifo {


private:

	using InternalT = typename types::TypeTraits<T>::internalType;

public:

	using AddressT = ufix<addressWidth>;
	using WordCountT = ufix<addressWidth + 1>;

	struct {

		inout::input_bus<T> Data;
		inout::input_node<bool> Write;
		inout::output_node<bool> Ready;

	} Input;

	struct {

		inout::output_bus<InternalT> Data;
		inout::input_node<bool> Read;
		inout::output_node<bool> Ready;

	} Output;

	struct {

		inout::input_node<bool> Reset;

		inout::output_node<dynfix> Level;

	} Control;

	BusFifo(int busWidth, int depth)
	{
		namespace b = blocks;

		// check for correct design parameters
		if (depth < 1) {
			throw dfx::design_error(string_printf("Fifo depth '%d' must be greater than 0.", depth));
		}

		if (depth > (1 << addressWidth)) {
			throw dfx::design_error(string_printf("Fifo depth '%d' must be smaller than 2**'addressWidth %d'.", depth, addressWidth));
		}

		Input.Data.create(busWidth);

		/*
			The following is a line-by-line translation of fifo.sv.

			Settings are:

			pc_flipflop_based = false
			pc_asynchronous = false
			pc_read_mode = 2'b01 ('read ahead')
			pc_read_ram_reg_out = true
			pc_almost_full_limit = 0
			pc_almost_empty_limit = 0
			pc_illegal_access_chk = 0
			*/

		/*
			// ----- write domain -----
			input  var logic                      wr_clk         ,   // write clock
			input  var logic                      wr_rst_n       ,   // write reset (asynchronous active low)
			input  var logic                      wr_sel         ,   // write select, put new data if not full
			input  var logic                      wr_clear       ,   // write synchronous clear, also to read side
			input  var logic [pc_data_length-1:0] wr_data        ,   // write data
			output var logic                      wr_full        ,   // write full flag
			output var logic [c_wcnt_length -1:0] wr_free_words  ,   // write minimum free words to be written

			// ----- read domain -----
			input  var logic                      rd_clk         ,   // read clock
			input  var logic                      rd_rst_n       ,   // read reset (asynchronous active low)
			input  var logic                      rd_sel         ,   // read select, get new data if not empty
			input  var logic                      rd_clear       ,   // read synchronous clear, also to write side
			output var logic [pc_data_length-1:0] rd_data        ,   // read data
			output var logic                      rd_empty       ,   // read empty flag
			output var logic [c_wcnt_length -1:0] rd_valid_words ,   // read minimum valid words to be read

			*/

		node<bool> wr_sel = Input.Write;
		node<bool> wr_clear = Control.Reset;
		bus<InternalT> wr_data = Input.Data;
		forward_node<bool> wr_full;
		forward_node<WordCountT> wr_free_words;

		node<bool> rd_sel = Output.Read;
		node<bool> rd_clear = Control.Reset;
		bus<InternalT> rd_data;
		node<bool> rd_empty;
		forward_node<WordCountT> rd_valid_words;

		// write/read addresses, sector flags and their pointer aggregates
		forward_node<AddressT> wr_addr;
		node<dynfix> wr_addr_s;     // synchronized to read clock domain
		node<dynfix> wr_addr_nxt;
		forward_node<bool> wr_sector;
		node<bool> wr_sector_s;   // synchronized to read clock domain
		node<bool> wr_sector_nxt;
		node<dynfix> wr_free_words_nxt;

		forward_node<AddressT> rd_addr;
		node<dynfix> rd_addr_s;     // synchronized to write clock domain
		node<dynfix> rd_addr_nxt;
		forward_node<bool> rd_sector;
		node<bool> rd_sector_s;   // synchronized to write clock domain
		node<bool> rd_sector_nxt;
		node<dynfix> rd_valid_words_nxt;

		// fifo buffer and output registers
		forward_bus<T> rd_data_buf(busWidth);
		forward_bus<T> rd_data_ff(busWidth);

		// control signals
		node<bool> wr_awake;
		forward_node<bool> wr_awake_s;
		node<bool> wr_buf_ena;
		node<bool> wr_full_nxt;

		node<bool> rd_awake;
		forward_node<bool> rd_awake_s;
		node<bool> rd_buf_ena;
		forward_node<bool> rd_data_ff_vld;
		node<bool> rd_data_ff_vld_nxt;
		forward_node<bool> rd_data_buf_vld;
		node<bool> rd_data_buf_vld_nxt;
		node<bool> rd_buf2ff_sel;       // read select from FIFO buffer to read output register

		//---------------------------------------------------------------------------
		// write domain: address/sector <-> pointer conversion & synchronization
		//---------------------------------------------------------------------------

		rd_sector_s = rd_sector;
		rd_addr_s = rd_addr;

		//---------------------------------------------------------------------------
		// write domain: FIFO write controller
		//---------------------------------------------------------------------------

		// protection of internal write enable against erroneous external write requests
		wr_buf_ena = wr_sel && !wr_full;

		// determination of next pointer value
		wr_addr_nxt = b::Decide(
			wr_buf_ena, b::Decide(
				wr_addr >= depth - 1, 0,
				b::FloorCast<AddressT>(wr_addr + 1)),
			wr_addr);

		wr_sector_nxt = b::Decide(
			wr_buf_ena, b::Decide(
				wr_addr >= depth - 1, !wr_sector,
				wr_sector),
			wr_sector);

		// determination of next available words, full & almost full flags
		wr_full_nxt = (wr_addr_nxt == rd_addr_s) && (wr_sector_nxt != rd_sector_s);

		wr_free_words_nxt = b::FloorCast<WordCountT>(b::Decide(
			wr_addr_nxt == rd_addr_s, b::Decide(                       // border values when effective pointers are identical
				wr_sector_nxt != rd_sector_s, dynfix(0),               // full when sector (wrap around) bits are different
				dynfix(depth)),                                        // empty when sector (wrap around) bits are equal
			depth - b::Decide(
				wr_addr_nxt >= rd_addr_s, wr_addr_nxt - rd_addr_s,     // neither full nor empty
				depth - (rd_addr_s - wr_addr_nxt))));

		//---------------------------------------------------------------------------

		wr_addr <<= b::Delay(b::Decide(
			rd_awake_s, wr_addr_nxt, AddressT(0)));

		wr_sector <<= b::Delay(b::Decide(
			rd_awake_s, wr_sector_nxt, false));

		wr_full <<= b::Delay(b::Decide(
			rd_awake_s, wr_full_nxt, false));

		wr_free_words <<= b::Delay(b::Decide(
			rd_awake_s, wr_free_words_nxt, WordCountT(depth)));

		//---------------------------------------------------------------------------
		// cross domain: awake flag generation
		//---------------------------------------------------------------------------

		wr_awake = !wr_clear;
		rd_awake = !rd_clear;

		rd_awake_s <<= wr_awake && rd_awake;
		wr_awake_s <<= wr_awake && rd_awake;

		//---------------------------------------------------------------------------
		// read domain: address/sector <-> pointer conversion & synchronization
		//---------------------------------------------------------------------------

		wr_sector_s = wr_sector;
		wr_addr_s = wr_addr;

		//---------------------------------------------------------------------------
		// read domain: FIFO read controller
		//---------------------------------------------------------------------------

		// check for FIFO buffer empty
		node<bool> fifo_empty = (rd_addr == wr_addr_s) && (rd_sector == wr_sector_s);

		// determination of internal read selects
		/*
		var logic [3:0] rd_ctrl_in;
		rd_ctrl_in = {~fifo_empty, rd_data_buf_vld, rd_data_ff_vld, rd_sel};
		casez (rd_ctrl_in)
		4'b00??: rd_ctrl_out = 5'b00;
		4'b010?: rd_ctrl_out = 5'b01;
		4'b0110: rd_ctrl_out = 5'b00;
		4'b0111: rd_ctrl_out = 5'b01;

		4'b10??: rd_ctrl_out = 5'b10;
		4'b110?: rd_ctrl_out = 5'b11;
		4'b1110: rd_ctrl_out = 5'b00;
		4'b1111: rd_ctrl_out = 5'b11;

		default: rd_ctrl_out = 5'b00;
		endcase
		{rd_buf_ena, rd_buf2ff_sel} = rd_ctrl_out;
		*/

		rd_buf_ena = b::Decide(
			fifo_empty, false,
			b::Decide(
				rd_data_buf_vld == false, true,
				rd_data_buf_vld == true && rd_data_ff_vld == false, true,
				rd_data_buf_vld == true && rd_data_ff_vld == true, rd_sel,
				false));

		rd_buf2ff_sel = rd_data_buf_vld && ((rd_data_ff_vld == false) || ((rd_data_ff_vld == true) && (rd_sel == true)));

		// determination of next address/sector
		rd_addr_nxt = b::Decide(
			rd_buf_ena, b::Decide(
				rd_addr >= depth - 1, 0,
				b::FloorCast<AddressT>(rd_addr + 1)),
			rd_addr);

		rd_sector_nxt = b::Decide(
			rd_buf_ena, b::Decide(
				rd_addr >= depth - 1, !rd_sector,
				rd_sector),
			rd_sector);

		// determination of next buffer valid flag
		rd_data_buf_vld_nxt = b::Decide(
			rd_buf_ena, !fifo_empty,
			b::Decide(
				rd_buf2ff_sel, false,
				rd_data_buf_vld));

		// determination of next output register valid flag
		rd_data_ff_vld_nxt = b::Decide(
			rd_buf2ff_sel, true,
			b::Decide(
				rd_sel, false,
				rd_data_ff_vld));

		// determination of next empty flag & valid words
		node<dynfix> rd_buf_ff_count_nxt = b::ReinterpretCast<ufix<1>>(rd_data_buf_vld_nxt) +b::ReinterpretCast<ufix<1>>(rd_data_ff_vld_nxt);
		rd_valid_words_nxt = b::FloorCast<WordCountT>(b::Decide(
			rd_addr_nxt == wr_addr_s, b::Decide(                       // border values when effective pointers are identical
				rd_sector_nxt == wr_sector_s, rd_buf_ff_count_nxt,     // empty when sector (wrap around) bits are equal
				depth + rd_buf_ff_count_nxt),                          // full when sector (wrap around) bits are different
			rd_buf_ff_count_nxt + b::Decide(
				wr_addr_s >= rd_addr_nxt, wr_addr_s - rd_addr_nxt,     // neither full nor empty
				depth - (rd_addr_nxt - wr_addr_s))));

		//---------------------------------------------------------------------------

		rd_addr <<= b::Delay(b::Decide(
			wr_awake_s, rd_addr_nxt, AddressT(0)));

		rd_sector <<= b::Delay(b::Decide(
			wr_awake_s, rd_sector_nxt, false));

		rd_data_buf_vld <<= b::Delay(b::Decide(
			wr_awake_s, rd_data_buf_vld_nxt, false));

		rd_data_ff_vld <<= b::Delay(b::Decide(
			wr_awake_s, rd_data_ff_vld_nxt, false));

		rd_valid_words <<= b::Delay(b::Decide(
			wr_awake_s, rd_valid_words_nxt, WordCountT(0)));

		//---------------------------------------------------------------------------

		rd_empty = !rd_data_ff_vld;

		//---------------------------------------------------------------------------
		// FIFO buffer (registers & multiplexer with spike shield or DPRAM)
		//---------------------------------------------------------------------------

		/*rd_data_buf <<= b::Delay(b::Decide(
			!wr_awake_s, T(0),
			rd_buf_ena, b::Memory<T>(depth, rd_addr, wr_buf_ena, wr_addr, wr_data),
			rd_data_buf));*/

		rd_data_buf <<= b::Decide(
			!wr_awake_s, T(0),
			b::Delay(rd_buf_ena), b::Memory<T>(depth, rd_addr, wr_buf_ena, wr_addr, wr_data),
			b::Delay(rd_data_buf));


		//---------------------------------------------------------------------------
		// FIFO read output register and multiplexer
		//---------------------------------------------------------------------------

		rd_data_ff <<= b::Delay(b::Decide(
			!wr_awake_s, T(0),
			rd_buf2ff_sel, rd_data_buf,
			rd_data_ff));

		rd_data = rd_data_ff;

		//---------------------------------------------------------------------------

		Input.Ready = !wr_full;

		Output.Ready = !rd_empty;
		Output.Data = rd_data;

		Control.Level = rd_valid_words;
	}
};


template<typename T, int addressWidth> class Fifo {

private:

	using internalType = typename types::TypeTraits<T>::internalType;

public:

	struct {

		inout::input_node<T> Data;
		inout::input_node<bool> Write;
		inout::output_node<bool> Ready;

	} Input;

	struct {

		inout::output_node<internalType> Data;
		inout::input_node<bool> Read;
		inout::output_node<bool> Ready;

	} Output;

	struct {

		inout::input_node<bool> Reset;

		inout::output_node<dynfix> Level;

	} Control;

	Fifo(int depth)
	{
		BusFifo<T, addressWidth> busFifo(1, depth);

		busFifo.Input.Data <<= bus<internalType>(Input.Data);
		busFifo.Input.Write <<= Input.Write;
		Input.Ready = busFifo.Input.Ready;

		Output.Data = busFifo.Output.Data.first();
		busFifo.Output.Read <<= Output.Read;
		Output.Ready = busFifo.Output.Ready;

		busFifo.Control.Reset <<= Control.Reset;
		Control.Level = busFifo.Control.Level;
	}
};

}
}
