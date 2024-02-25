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

	Main include for Verilog code emission.

*/

#pragma once

#include "global.h"

class VerilogExporter;

class EntityProcessor {

protected:

	VerilogExporter *exporter;

public:

	EntityProcessor(VerilogExporter *theExporter) : exporter(theExporter) {}
	virtual ~EntityProcessor() {}

	virtual void WritePreamble(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const = 0;
	virtual void WriteCode(std::ofstream &f, dfx::generator::Instance &module, dfx::generator::Entity &entity) const = 0;
};


class VerilogExporter {

public:

	struct Configuration {

		// Name of the negated reset input, e.g. "nrst".
		std::string negatedResetPinName;

		// Module names prefixed with ! are considered models that can be put into different directory, if desired.
		std::string modelsPath;

		// Allow the 'indexed part select' feature (+: operator) of SystemVerilog.
		bool useIndexedPartSelect;

		Configuration();
	};

private:

	Configuration configuration;

	dfx::generator::Generator &generator;
	std::unordered_map<std::string, std::unique_ptr<EntityProcessor>> entityProcessors;
	std::unique_ptr<EntityProcessor> defaultProcessor;

	std::string MakeCompliantName(std::string name) const;

	void PrepareModule(dfx::generator::Instance &module);
	void ExportModule(std::string const &basePath, dfx::generator::Instance &module, std::basic_ostream<char> &os, bool includeInListOfFiles);

	std::list<std::string> listOfFiles;

public:

	VerilogExporter(dfx::generator::Generator &generator, Configuration const &theConfiguration = Configuration());
	void Export(std::string const &basePath, std::basic_ostream<char> &os);

	Configuration const &GetConfiguration() const;

	std::list<std::string> const &GetListOfFiles() const;
};


// Returns an expression for the given output, including bus index.
std::string GetNodeExpression(dfx::generator::Entity::Output const *output);

// Returns an expression for the given input, including bus index.
std::string GetNodeExpression(dfx::generator::Entity::Input const &input);

// Returns an expression for the given input, including bus index.
std::string GetExpandedNodeExpression(dfx::types::TypeDescription const &targetType, dfx::generator::Entity::Input const &input);

// Returns an array expression for the given list of inputs if possible, or an empty string otherwise.
std::string GetArrayExpression(dfx::generator::Entity::Input const *inputs, int firstIndex, int length);

// Tests if the given range of inputs is driven through a bus or a slice. 
bool IsInputBusOrSlice(dfx::generator::Entity::Input const *inputs, int firstIndex, int length);

std::string expand_signal(dfx::generator::Entity::Output *signal, int const &right, int const &left);
std::string signal_declaration(dfx::types::TypeDescription const &type, std::string const &name, int busSize, std::string const &delimiter);
int word_width(const dfx::types::TypeDescription &type);
int fraction(const dfx::types::TypeDescription &type);
std::string width2string(int width);
