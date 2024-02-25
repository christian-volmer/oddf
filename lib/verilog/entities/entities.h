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

	Classes for Verilog code emission.

*/

#include "../verilog.h"

namespace entities {

// default

class Default : public EntityProcessor {

public:

	Default(VerilogExporter *theExporter) : EntityProcessor(theExporter) {}

	void WritePreamble(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};


// ignore

class Ignore : public EntityProcessor {

public:

	Ignore(VerilogExporter *theExporter) : EntityProcessor(theExporter) {}

	void WritePreamble(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};


// input_port

class InputPort : public Default {

public:

	InputPort(VerilogExporter *theExporter) : Default(theExporter) {}

	void WritePreamble(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};


// output_port

class OutputPort : public Default {

public:

	OutputPort(VerilogExporter *theExporter) : Default(theExporter) {}

	void WritePreamble(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};


// delay

class Delay : public Default {

public:

	Delay(VerilogExporter *theExporter) : Default(theExporter) {}

	void WritePreamble(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};


// instance

class Instance : public Default {

public:

	Instance(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};


// constant

class Constant : public Default {

public:

	Constant(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

// not

class Not : public Default {

public:

	Not(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

// decide

class Decide : public Default {

public:

	Decide(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

// and, or, xor
class Logic : public Default {

private:
	std::string operation;

public:

	Logic(VerilogExporter *theExporter, std::string const &theOperation) : Default(theExporter), operation(theOperation) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

// bit_extract
class BitExtract : public Default {

public:

	BitExtract(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

// bit compose
class BitCompose : public Default {

public:

	BitCompose(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Negate : public Default {

public:

	Negate(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Add : public Default {

public:

	Add(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Mul : public Default {

public:

	Mul(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class FloorCast : public Default {

public:

	FloorCast(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class ReinterpretCast : public Default {

public:

	ReinterpretCast(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

// equal, not_equal, less, less_equal
class Compare : public Default {

private:
	std::string operation;

public:

	Compare(VerilogExporter *theExporter, std::string const &theOperation) : Default(theExporter), operation(theOperation) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class MemoryDualPort: public Default {

public:

	MemoryDualPort(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Select: public Default {

public:

	Select(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Stimulus : public Default {

public:

	Stimulus(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Checker : public Default {

public:

	Checker(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Recorder : public Default {

private:
	static int const MaxTestDataChunk = 2048; // must be a multiple of 4!

public:

	Recorder(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

class Spare : public Default {

public:

	Spare(VerilogExporter *theExporter) : Default(theExporter) {}

	void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const override;
};

} // namespace entities
