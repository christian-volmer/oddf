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

	Base functionality of Verilog code emission.

*/

#include "global.h"

#include "verilog.h"
#include "entities/entities.h"

using dfx::generator::Instance;
using dfx::generator::Entity;

//
// VerilogExporter::Configuration
//

VerilogExporter::Configuration::Configuration() :
	negatedResetPinName("nrst"),
	modelsPath(""),
	useIndexedPartSelect(true)
{
}

//
// VerilogExporter
//

VerilogExporter::VerilogExporter(dfx::generator::Generator &theGenerator, Configuration const &theConfiguration) :
	configuration(theConfiguration),
	generator(theGenerator)
{
	defaultProcessor = std::unique_ptr<EntityProcessor>(new entities::Default(this));

	entityProcessors["input_port"] = std::unique_ptr<EntityProcessor>(new entities::InputPort(this));
	entityProcessors["output_port"] = std::unique_ptr<EntityProcessor>(new entities::OutputPort(this));
	entityProcessors["delay"] = std::unique_ptr<EntityProcessor>(new entities::Delay(this));
	entityProcessors["instance"] = std::unique_ptr<EntityProcessor>(new entities::Instance(this));
	entityProcessors["constant"] = std::unique_ptr<EntityProcessor>(new entities::Constant(this));
	entityProcessors["not"] = std::unique_ptr<EntityProcessor>(new entities::Not(this));
	entityProcessors["decide"] = std::unique_ptr<EntityProcessor>(new entities::Decide(this));
	entityProcessors["and"] = std::unique_ptr<EntityProcessor>(new entities::Logic(this, "&"));
	entityProcessors["or"] = std::unique_ptr<EntityProcessor>(new entities::Logic(this, "|"));
	entityProcessors["xor"] = std::unique_ptr<EntityProcessor>(new entities::Logic(this, "^"));
	entityProcessors["negate"] = std::unique_ptr<EntityProcessor>(new entities::Negate(this));
	entityProcessors["bit_extract"] = std::unique_ptr<EntityProcessor>(new entities::BitExtract(this));
	entityProcessors["bit_compose"] = std::unique_ptr<EntityProcessor>(new entities::BitCompose(this));
	entityProcessors["plus"] = std::unique_ptr<EntityProcessor>(new entities::Add(this));
	entityProcessors["times"] = std::unique_ptr<EntityProcessor>(new entities::Mul(this));
	entityProcessors["floor_cast"] = std::unique_ptr<EntityProcessor>(new entities::FloorCast(this));
	entityProcessors["reinterpret_cast"] = std::unique_ptr<EntityProcessor>(new entities::ReinterpretCast(this));
	entityProcessors["equal"] = std::unique_ptr<EntityProcessor>(new entities::Compare(this, "=="));
	entityProcessors["not_equal"] = std::unique_ptr<EntityProcessor>(new entities::Compare(this, "!="));
	entityProcessors["less"] = std::unique_ptr<EntityProcessor>(new entities::Compare(this, "<"));
	entityProcessors["less_equal"] = std::unique_ptr<EntityProcessor>(new entities::Compare(this, "<="));
	entityProcessors["memory"] = std::unique_ptr<EntityProcessor>(new entities::MemoryDualPort(this));
	entityProcessors["select"] = std::unique_ptr<EntityProcessor>(new entities::Select(this));
	entityProcessors["stimulus"] = std::unique_ptr<EntityProcessor>(new entities::Stimulus(this));
	entityProcessors["checker"] = std::unique_ptr<EntityProcessor>(new entities::Checker(this));
	entityProcessors["recorder"] = std::unique_ptr<EntityProcessor>(new entities::Recorder(this));
	entityProcessors["spare_ff"] = std::unique_ptr<EntityProcessor>(new entities::Spare(this));

	entityProcessors["$label"] = std::unique_ptr<EntityProcessor>(new entities::Ignore(this));
	entityProcessors["$terminate"] = std::unique_ptr<EntityProcessor>(new entities::Ignore(this));
	entityProcessors["write_register"] = std::unique_ptr<EntityProcessor>(new entities::Ignore(this));
	entityProcessors["read_register"] = std::unique_ptr<EntityProcessor>(new entities::Ignore(this));
}

VerilogExporter::Configuration const &VerilogExporter::GetConfiguration() const
{
	return configuration;
}

void VerilogExporter::Export(std::string const &basePath, std::basic_ostream<char> &os)
{
	listOfFiles.clear();

	//dfx::generator::Generator generator(design, os);

	os << std::endl << " --- Export to Verilog --- " << std::endl << std::endl;

	os << "Preparing for conversion..." << std::endl << std::endl;

	for (auto &instance : generator.instances)
		if (!instance.unifiedWith)
			PrepareModule(instance);

	os << "Writing verilog files" << std::endl << std::endl;

	std::string modelsBasePath = basePath;
	if (!GetConfiguration().modelsPath.empty()) {

		fs::path tmpPath = fs::path(basePath) / fs::path(GetConfiguration().modelsPath);
		modelsBasePath = tmpPath.string();
		fs::create_directories(tmpPath);
	}

	for (auto &instance : generator.instances)
		if (!instance.unifiedWith)
			ExportModule(instance.isModel ? modelsBasePath : basePath, instance, os, !instance.isModel);
}

void VerilogExporter::PrepareModule(dfx::generator::Instance &module)
{
	// Generate names for input ports
	for (auto *inputPort : module.inputPorts) {

		assert(inputPort->inputs.size() == inputPort->outputs.size());
		for (auto &out : inputPort->outputs)
			out.SetBaseName("in_" + out.GetBaseName());
	}

	// Generate names for output ports
	for (auto *outputPort : module.outputPorts) {

		assert(outputPort->inputs.size() == outputPort->outputs.size());
		for (auto &in : outputPort->inputs)
			in.name = "out_" + in.name;
	}

	// Generate names for instances
	for (auto &ent : module.entities) {

		if (ent.className != "instance")
			continue;

		ent.name = "u_" + MakeCompliantName(ent.name);
	}

	// Generate names for outputs of entities
	for (auto &ent : module.entities) {

		if (ent.className == "output_port" || ent.className == "input_port")
			continue;

		ent.name = MakeCompliantName(ent.name);

		for (auto &output : ent.outputs)
			output.SetBaseName(ent.name + "_" + output.GetBaseName());
	}
}

void VerilogExporter::ExportModule(std::string const &basePath, dfx::generator::Instance &module, std::basic_ostream<char> &os, bool includeInListOfFiles)
{
	os << "Processing module '" << module.moduleName << "'...";

	std::string nrstName = GetConfiguration().negatedResetPinName;
	std::string fileName = module.moduleName + ".sv";

	if (includeInListOfFiles)
		listOfFiles.push_back(fileName);

	std::string pathName = (fs::path(basePath) / fs::path(fileName)).string();
	std::ofstream f(pathName);

	f << "module " << module.moduleName << "\n";
	f << "(\n\tinput var logic clk,\n\tinput var logic " << nrstName;
	if (module.outputPorts.size() > 0 || module.inputPorts.size() > 0)
		f << ",\n\n";
	else
		f << "\n";

	unsigned int i = 0; 
	for (auto const *input : module.inputPorts)
		f << "\tinput var logic" << signal_declaration(
			input->outputs[0].type, 
			input->outputs[0].GetBaseName(), 
			input->outputs[0].busSize,
			(++i == module.inputPorts.size() && module.outputPorts.size() == 0) ? " " : ",") << "\n";

	if (module.outputPorts.size() > 0)
		f << "\n";

	i = 0;
	for (auto const *output : module.outputPorts)
		f << "\toutput var logic" << signal_declaration(
			output->inputs[0].driver->type, 
			output->inputs[0].name, 
			output->outputs[0].busSize, // Entity::Input does not have a bus-size field. We store it in the outputs of this 'output_port' entity.
			(++i == module.outputPorts.size()) ? " " : ",") << "\n";

	f << ");\n";

	// signal declarations
	f << "\n";
	for (auto &ent : module.entities) {

		if (ent.className == "output_port" || ent.className == "input_port")
			continue;

		auto it = entityProcessors.find(ent.className);
		EntityProcessor *processor = it != entityProcessors.end() ? it->second.get() : defaultProcessor.get();

		processor->WritePreamble(f, module, ent);
	}

	// instantiate all entities that are not output ports
	f << "\n";
	for (auto &ent : module.entities) {

		if (ent.className == "output_port")
			continue;

		auto it = entityProcessors.find(ent.className);
		EntityProcessor *processor = it != entityProcessors.end() ? it->second.get() : defaultProcessor.get();

		processor->WriteCode(f, module, ent);
	}


	// output ports at the end
	f << "\n";
	for (auto &ent : module.entities) {

		if (ent.className != "output_port")
			continue;

		auto it = entityProcessors.find(ent.className);
		EntityProcessor *processor = it != entityProcessors.end() ? it->second.get() : defaultProcessor.get();

		processor->WriteCode(f, module, ent);
	}

	f << "\nendmodule\n";

	os << " --> '" << pathName << "'" << std::endl;
}

std::list<std::string> const &VerilogExporter::GetListOfFiles() const
{
	return listOfFiles;
}

std::string VerilogExporter::MakeCompliantName(std::string name) const
{
	name.erase(std::remove_if(name.begin(), name.end(), [](char c) { return !((c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')); }), name.end());
	return name;
}

std::string expand_signal(dfx::generator::Entity::Output *signal, int const &left, int const &right)
{
	std::string temp = "";
	
	// { xxxx }
	if (left > 0 || right > 0) {
		temp += "{";
		
		// padding on the left		
		if (signal->type.IsSigned()) { 

			int w = signal->type.GetWordWidth();

			//sign extension
			if (left > 1)
			{ // {nn{xxx[mm]}},
				temp += "{" + std::to_string(left) + "{" + GetNodeExpression(signal) + (w > 1 ? "[" + std::to_string(w - 1) + "]}}, " : "}}, ");
			}
			else if (left == 1) 
			{ // xxx[mm]
				temp += GetNodeExpression(signal) + (w > 1 ? "[" + std::to_string(w - 1) + "], " : ", ");
			}
			// left == 0 -> nothing to do here
		}
		else // unsigned - just pad with zeros
		{
			if (left >= 1) 
			{ // nn'd0,
				temp += std::to_string(left) + "'d0, ";
			}
			// left == 0 -> nothing to do here
		}

		temp += GetNodeExpression(signal);

		// padding on the right		
		if (right >= 1)
		{	// pad with zeros
			// nn'd0,
			temp += ", " + std::to_string(right) + "'d0";
		}

		temp += "}";
	}
	else // no padding
	{
		temp = GetNodeExpression(signal);
	}

	return temp;
}

std::string GetNodeExpression(Entity::Output const *output)
{
	if (output->busSize == -1) {

		assert(output->busIndex == 0);
		return output->GetBaseName();
	}
	else if (output->busSize == 1) {

		assert(output->busIndex == 0);
		return output->GetBaseName();
	}
	else if (output->busSize > 1) {

		assert(output->busIndex >= 0 && output->busIndex < output->busSize);
		return output->GetBaseName() + "[" + std::to_string(output->busIndex) + "]";
	}
	else
		throw std::runtime_error("Internal Error: value of Entity::Output::busSize is invalid.");
}

std::string GetNodeExpression(Entity::Input const &input)
{
	return GetNodeExpression(input.driver);
}

std::string GetExpandedNodeExpression(dfx::types::TypeDescription const &targetType, dfx::generator::Entity::Input const &input)
{
	using dfx::types::TypeDescription;

	if (targetType.IsClass(TypeDescription::Class::Boolean)) {

		assert(input.driver->type.IsClass(TypeDescription::Class::Boolean));
		return GetNodeExpression(input);
	}
	else if (targetType.IsClass(TypeDescription::Class::FixedPoint)) {

		if (input.driver->type == targetType)
			return GetNodeExpression(input);

		assert(input.driver->type.IsClass(TypeDescription::Class::FixedPoint));

		int right = targetType.GetFraction() - input.driver->type.GetFraction();
		int left = targetType.GetWordWidth() - input.driver->type.GetWordWidth() - right;
		assert(right >= 0);
		assert(left >= 0);

		return expand_signal(input.driver, left, right);
	}

	throw std::runtime_error("Internal Error: GetExpandedNodeExpression() called with inconsistent types.");
}

std::string GetArrayExpression(Entity::Input const *inputs, int firstIndex, int length)
{
	if (length == 1)
		return GetNodeExpression(inputs[firstIndex].driver);
	
	if (length > 1 && IsInputBusOrSlice(inputs, firstIndex, length)) {

		int first = inputs[firstIndex].driver->busIndex;

		if (first == 0 && inputs[firstIndex].driver->busSize == length)
			return inputs[firstIndex].driver->GetBaseName();

		int last = inputs[firstIndex + length - 1].driver->busIndex;
		return inputs[firstIndex].driver->GetBaseName() + "[" + std::to_string(first) + ":" + std::to_string(last) + "]";
	}

	return "";
}

bool IsInputBusOrSlice(dfx::generator::Entity::Input const *inputs, int firstIndex, int length)
{
	if (length < 1)
		return false;

	Entity::Output *lastDriver = nullptr;

	for (int i = 0; i < length; ++i) {

		Entity::Output *thisDriver = inputs[i + firstIndex].driver;

		if (lastDriver == nullptr) {

			if (thisDriver->busSize < length)
				return false;
		}
		else {

			if (thisDriver->busIndex != lastDriver->busIndex + 1)
				return false;

			if (thisDriver->entity != lastDriver->entity)
				return false;

			if (thisDriver->groupIndex != lastDriver->groupIndex)
				return false;

			assert(thisDriver->busSize == lastDriver->busSize);
			assert(thisDriver->type == lastDriver->type);
		}

		lastDriver = thisDriver;
	}

	return true;
}

std::string signal_declaration(dfx::types::TypeDescription const &type, std::string const &name, int busSize, std::string const &delimiter)
{
	using dfx::types::TypeDescription;

//	assert(type.IsClass(dfx::generator::DriverType::FixedPoint) || type.IsClass(dfx::generator::DriverType::Boolean));

	assert((busSize == -1) || (busSize >= 1)); // -1 is normal node, busses of size 0 are not supported in Verilog.
	
	std::string tmp;
	
	if (type.GetClass() == TypeDescription::FixedPoint)
	{
		// add extra signal specifier for fixedpoint

		if (type.IsSigned()) {
			tmp += " signed";
		}

		int width = type.GetWordWidth();
		if (width > 1) {
			tmp += " [" + std::to_string(width - 1) + ":0]";
		}
	}

	if (busSize == -1)
		tmp += " " + name + delimiter + " // " + type.ToString(); 
	else if(busSize == 1)
		tmp += " " + name + delimiter + " // " + type.ToString() + "[" + std::to_string(busSize) + "]";
	else
		tmp += " " + name + "[0:" + std::to_string(busSize-1) + "]" + delimiter + " // " + type.ToString() + "[" + std::to_string(busSize) + "]";

	return tmp;
}

int word_width(const dfx::types::TypeDescription &type)
{
	if (type.IsClass(dfx::types::TypeDescription::Boolean))
		return 1;
	else if (type.IsClass(dfx::types::TypeDescription::FixedPoint))
		return type.GetWordWidth();
	else throw dfx::design_error("during Verilog generation only bool and fixedpoint types are allowed");
}

int fraction(const dfx::types::TypeDescription &type)
{
	if (type.IsClass(dfx::types::TypeDescription::Boolean))
		return 0;
	else if (type.IsClass(dfx::types::TypeDescription::FixedPoint))
		return type.GetFraction();
	else throw dfx::design_error("during Verilog generation only bool and fixedpoint types are allowed");
}

std::string width2string(int width) 
{
	// converts width=1 to "" and width=2,3,4... to "[width-1:0]"
	return (width > 1) ? "[" + std::to_string(width - 1) + ":0] " : "";
}
