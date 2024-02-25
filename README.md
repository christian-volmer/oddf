# ODDF - Open Digital Design Framework

The Open Digital Design Framework, or ODDF for short, is a framework written in and for C++ for the modelling, design, simulation, verification, and Verilog netlist creation of digital circuits. 

It has a strong focus on arithmetic operations and therefore on Digital Signal Processing (DSP) applications. Although it was used extensively in the creation of an Application Specific Integrated Circuit (ASIC), the generated Verilog netlists should be compatible with FPGA (Field Programmable Gate Array) synthesis as well.

## Motivation

Traditional Hardware Description Languages (HDL), such as VHDL and Verilog, describe digital hardware on a very low level: the ‘bit’. Although higher level operations, such as addition or multiplication exist, the designer must constantly pay attention to things like word width or signedness of operands in order to reach a correct design in the end. This becomes very cumbersome and prone to errors with more elaborate designs.

The goal of ODDF is to relieve the high-level circuit designer of the burden of having think in terms of low-level constructs. Word widths and signedness of operands are tracked automatically and it is ensured that enough bits become allocated to accommodate the results of arithmetic operations without overflow. There is full support for fixed point number arithmetic. Numbers and bits can be stored as single nodes or collected in the form of busses.

## Use model

Within ODDF, a digital circuit is described using simple C++ data-types and functions. Check out the example programs in the `examples` directory.

Circuit simulation happens from within the same C++ program and the results can be exported for further analysis by external software (MATLAB, python, Mathematica, etc.)

As the final step the design can be exported as Verilog. The Verilog output is fully synthesisable and is understood by commercial ASIC design software, for instance, by Cadence or Synopsys.

## Features

- Single-clock domain
- Nodes and busses
- Data-types:
	- Boolean, integer, fixed-point (synthesisable)
	- system integer and double (non-synthesisable, for modelling only)
- Binary operations
	- `Not`, `And`, `Or`, `Xor` 
	- Boolean reduction across busses
- Arithmetic operations
	- unary `-`
	- binary `+`, `-`, `*`
	- `*` and `/` by power of two
	- word width and fractional part of all arithmetic operations will always be able to accommodate the result without overflow or loss of precision
- Comparisons: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Rounding
	- Floor, Nearest, and Convergent 
	- with and without saturation
- Pipelining
- Data sources and sinks
- Models for memories and FIFOs
- Fast generation of synthesisable Verilog code
	- hierarchical code generation with full support for modules and instances

