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

	Logger functions record node and bus values during simulation.
	Content can be printed in table form or exported as a
	machine-readable text file.

*/

#include "../global.h"

#include "logger.h"
#include "../blocks/function.h"
#include "../formatting.h"

namespace fs = std::filesystem;

// Helper functions
static std::string indexing_string(int flags, int width)
{
	using Flags = dfx::modules::Logger::Flags;

	switch (flags & 3) {

		case Flags::ZeroBasedIndexing | Flags::NormalElementOrder:
			return string_printf("[0..%d]", width - 1);

		case Flags::OneBasedIndexing | Flags::NormalElementOrder:
			return string_printf("(1..%d)", width);

		case Flags::ZeroBasedIndexing | Flags::ReversedElementOrder:
			return string_printf("[%d..0]", width - 1);

		case Flags::OneBasedIndexing | Flags::ReversedElementOrder:
			return string_printf("(%d..1)", width);
	}

	return "[error]";
}

static int max_element_width(std::vector<std::string> const &column)
{
	std::size_t maxWidth = 0;
	for (auto &element : column)
		if (element.length() > maxWidth)
			maxWidth = element.length();

	return (int)maxWidth;
}

static std::vector<std::string> format_as_bus(std::vector<std::string> const &elements, int width, int separation, bool reverse)
{
	int elementWidth = max_element_width(elements);
	int elementWidthWithSeparation = elementWidth + separation;

	int length = (int)elements.size() / width;

	std::vector<std::string> result;
	result.reserve(length);

	int firstElement = reverse ? width - 1 : 0;
	int direction = reverse != 0 ? -1 : 1;

	for (int i = 0; i < length; ++i) {

		std::stringstream element;

		element << std::setw(elementWidth) << elements[i * width + firstElement];

		for (int j = 1; j < width; ++j)
			element << std::setw(elementWidthWithSeparation) << elements[i * width + firstElement + j*direction];

		result.push_back(element.str());
	}

	return result;
}

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class logger_block: public BlockBase, public logger_BlockBase, private IStep {

private:

	logger_callback *callback;

	std::list<InputPin<T>> inputs;
	std::vector<T> values;

	std::string formatString;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	bool CanEvaluate() const override
	{
		return false;
	}

	void Evaluate() override
	{
	}

	void Step() override
	{
		if (callback && callback->IsEnabled()) {

			for (auto &input : inputs)
				values.push_back(input.GetValue());
		}
	}

	void AsyncReset() override
	{
	}

	IStep *GetStep()
	{
		return this;
	}

	virtual void clear() override
	{
		values.clear();
	}

	virtual int get_length() const override
	{
		return (int)(values.size() / inputs.size());
	}

	virtual std::vector<std::string> get_formatted_list() const override
	{
		std::vector<std::string> elements;
		elements.reserve(values.size());

		for (auto const &value : values)
			elements.push_back(string_printf(formatString, value));

		return elements;
	}

	virtual void detach() override
	{
		callback = nullptr;
	}

public:

	logger_block(logger_callback *callback, std::string const &formatString) :
		BlockBase("logger"),
		callback(callback),
		inputs(),
		values(),
		formatString(formatString)
	{
	}

	virtual ~logger_block()
	{
		// Inform the parent Logger class when this block becomes destructed.
		if (callback)
			callback->NotifyRemoval(this);
	}

	void add_node(node<T> const &node)
	{
		inputs.emplace_back(this, node);
	}

	void add_bus(bus<T> bus)
	{
		unsigned width = bus.width();
		for (unsigned i = 1; i <= width; ++i)
			add_node(bus(i));
	}
};

}
}

namespace modules {

Logger::Logger() :
	Enabled(true),
	Columns()
{
}

Logger::~Logger()
{
	// Detach all logger blocks
	for (auto &column : Columns)
		column.Block->detach();
}

void Logger::Disable()
{
	Enabled = false;
}

void Logger::Enable()
{
	Enabled = true;
}

bool Logger::IsEnabled() const
{
	return Enabled;
}

void Logger::NotifyRemoval(backend::logger_BlockBase *loggerBlock)
{
	auto found = std::find_if(Columns.begin(), Columns.end(), [=](Column const &column) { return column.Block == loggerBlock; });
	Columns.erase(found);
}


//
// Node logging
//

// node<bool>
void Logger::Log(std::string const &tag, std::string const &name, node<bool> const &node, char const *format /* = "%d" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<bool>>((logger_callback *)this, format);
	block.add_node(node);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Boolean;
	column.IsBus = false;
	column.BusWidth = 1;
	column.Separation = 0;
	column.BusFlags = 0;

	Columns.push_back(column);
}

// node<int32>
void Logger::Log(std::string const &tag, std::string const &name, node<std::int32_t> const &node, char const *format /* = "%" PRId32 */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<std::int32_t>>((logger_callback *)this, format);
	block.add_node(node);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Integer;
	column.IsBus = false;
	column.BusWidth = 1;
	column.Separation = 0;
	column.BusFlags = 0;

	Columns.push_back(column);
}

// node<int64>
void Logger::Log(std::string const &tag, std::string const &name, node<std::int64_t> const &node, char const *format /* = "%" PRId64 */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<std::int64_t>>((logger_callback *)this, format);
	block.add_node(node);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Integer;
	column.IsBus = false;
	column.BusWidth = 1;
	column.Separation = 0;
	column.BusFlags = 0;

	Columns.push_back(column);
}

// node<double>
void Logger::Log(std::string const &tag, std::string const &name, node<double> const &node, char const *format /* = "%g" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<double>>((logger_callback *)this, format);
	block.add_node(node);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Analogue;
	column.IsBus = false;
	column.BusWidth = 1;
	column.Separation = 0;
	column.BusFlags = 0;

	Columns.push_back(column);
}

// node<dynfix>
void Logger::Log(std::string const &tag, std::string const &name, node<dynfix> const &node, char const *format /* = "%g" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<double>>((logger_callback *)this, format);
	block.add_node(blocks::Function(node, [](dynfix const &value) { return (double)value; }));

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Analogue;
	column.IsBus = false;
	column.BusWidth = 1;
	column.Separation = 0;
	column.BusFlags = 0;

	Columns.push_back(column);
}

//
// Bus logging
//

// bus<bool>
void Logger::Log(std::string const &tag, std::string const &name, bus_access<bool> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%d" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<bool>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Boolean;
	column.IsBus = true;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// bus<int32>
void Logger::Log(std::string const &tag, std::string const &name, bus_access<std::int32_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId32 */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<std::int32_t>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;	
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Integer;
	column.IsBus = true;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// bus<int64>
void Logger::Log(std::string const &tag, std::string const &name, bus_access<std::int64_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId64 */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<std::int64_t>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Integer;
	column.IsBus = true;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// bus<double>
void Logger::Log(std::string const &tag, std::string const &name, bus_access<double> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<double>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Analogue;
	column.IsBus = true;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// bus<dynfix>
void Logger::Log(std::string const &tag, std::string const &name, bus_access<dynfix> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<double>>((logger_callback *)this, format);
	block.add_bus(blocks::Function(bus, [](dynfix const &value) { return (double)value; }));

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Analogue;
	column.IsBus = true;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}


//
// Sequence logging
//

// sequence<bool>
void Logger::LogSequence(std::string const &tag, std::string const &name, bus_access<bool> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%d" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<bool>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Boolean;
	column.IsBus = false;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// sequence<int32>
void Logger::LogSequence(std::string const &tag, std::string const &name, bus_access<std::int32_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId32 */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<std::int32_t>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Integer;
	column.IsBus = false;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// sequence<int64>
void Logger::LogSequence(std::string const &tag, std::string const &name, bus_access<std::int64_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId64 */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<std::int64_t>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Integer;
	column.IsBus = false;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// sequence<double>
void Logger::LogSequence(std::string const &tag, std::string const &name, bus_access<double> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<double>>((logger_callback *)this, format);
	block.add_bus(bus);

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Analogue;
	column.IsBus = false;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

// sequence<dynfix>
void Logger::LogSequence(std::string const &tag, std::string const &name, bus_access<dynfix> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::logger_block<double>>((logger_callback *)this, format);
	block.add_bus(blocks::Function(bus, [](dynfix const &value) { return (double)value; }));

	Column column;
	column.Tag = tag;
	column.Name = name;
	column.Block = &block;
	column.signalType = SignalType::Analogue;
	column.IsBus = false;
	column.BusWidth = bus.width();
	column.Separation = separation;
	column.BusFlags = flags;

	Columns.push_back(column);
}

void Logger::Clear()
{
	for (auto &column : Columns)
		column.Block->clear();
}

void Logger::WriteTable(std::basic_ostream<char> &os, std::unordered_set<std::string> const &tags) const
{
	struct FormattedColumn {

		std::string Name;
		std::vector<std::string> Elements;
		int Width;
	};

	std::vector<FormattedColumn> formattedColumns;

	for (auto const &column : Columns) {

		if ((tags.size() > 0) && (tags.find(column.Tag) == tags.end()))
			continue;

		FormattedColumn formattedColumn;

		if (column.IsBus) {

			formattedColumn.Name = column.Name + indexing_string(column.BusFlags, column.BusWidth);

			formattedColumn.Elements = format_as_bus(column.Block->get_formatted_list(), column.BusWidth, column.Separation, (column.BusFlags & Flags::ReversedElementOrder) != 0);
			formattedColumn.Width = std::max(
				(int)formattedColumn.Name.length(),
				max_element_width(formattedColumn.Elements)
				);
		}
		else {

			formattedColumn.Name = column.Name;

			if (column.BusWidth == 1)
				formattedColumn.Elements = column.Block->get_formatted_list();
			else {

				formattedColumn.Elements = std::vector<std::string>(column.Block->get_length(), "<sequence not supported>");
			}

			formattedColumn.Width = std::max(
				(int)formattedColumn.Name.length(),
				max_element_width(formattedColumn.Elements)
				);
		}

		formattedColumns.push_back(formattedColumn);
	}

	int numberOfRows = formattedColumns.size() > 0 ? (int)formattedColumns.front().Elements.size() : 0;

	if (numberOfRows == 0) {

		os << "The logger has not logged any signals.\n";
		return;
	}

	int sequenceColumnWidth = (int)std::ceil(std::log10(numberOfRows + 0.1));

	// Print the header
	os << std::setw(sequenceColumnWidth + 2) << "#" << " |";
	for (auto const &column : formattedColumns)
		os << "|" << std::setw(column.Width + 1) << column.Name << " ";
	os << "\n";

	// Print separator
	os << std::string(sequenceColumnWidth + 3, '-') << "+";
	for (auto const &column : formattedColumns)
		os << "+" << std::string(column.Width + 2, '-');
	os << "\n";

	// Print elements
	for (int i = 0; i < numberOfRows; ++i) {

		os << std::setw(sequenceColumnWidth + 2) << i << " |";
		for (auto const &column : formattedColumns)
			os << "|" << std::setw(column.Width + 1) << column.Elements[i] << " ";
		os << "\n";
	}
}

void Logger::ExportToVaryPlot(std::string const &s_basePath, std::unordered_set<std::string> const &tags, std::string const &xLabel, std::string const &xUnit, double xScaling) const
{
	struct Waveform {

		Column const *Column;
		std::string Name;
		std::string FileName;
	};

	fs::path basePath(s_basePath);

	fs::create_directories(basePath);

	std::vector<Waveform> waveforms;
	std::unordered_set<std::string> usedFileNames;

	int numberOfClocks = 0;

	for (auto const &column : Columns) {

		if ((tags.size() > 0) && (tags.find(column.Tag) == tags.end()))
			continue;

		std::string fullName = column.Tag.length() > 0
			? column.Tag + "." + column.Name
			: column.Name;

		std::string fileName = fullName;

		{
			int counter = 2;
			while (usedFileNames.find(fileName) != usedFileNames.end())
				fileName = string_printf("%s(%d)", fullName.c_str(), counter++);
		}

		usedFileNames.insert(fileName);

		std::ofstream file(basePath / fs::path(fileName + ".txt"));

		file << std::setprecision(16);

		std::vector<std::string> values = column.IsBus 
			? format_as_bus(column.Block->get_formatted_list(), column.BusWidth, 1, false)
			: column.Block->get_formatted_list();

		numberOfClocks = std::max(numberOfClocks, column.Block->get_length());

		double clockStretch = column.IsBus
			? 1.0
			: column.BusWidth;

		for (std::size_t clock = 0; clock < values.size(); ++clock)
			file << clock / clockStretch * xScaling << " " << values[clock] << "\n";

		waveforms.push_back({ &column, fullName, fileName });
	}

	// Create clock waveform to show in VaryPlot along with the user signals
	{
		std::ofstream clockFile(basePath / fs::path("#clock.txt"));
		clockFile << "0 0" << "\n";
		for (int i = 1; i < numberOfClocks; ++i)
			clockFile << i * xScaling << " 1" << "\n" << (i + 0.5) * xScaling << " 0" << "\n";

		waveforms.insert(waveforms.begin(), { nullptr, "CLOCK", "#clock" });
	}


	// Create VaryPlot script file.
	std::ofstream script(basePath / fs::path("plots.vps"));

	// Digital graphs
	script <<
		"setapplication maximize=yes\n"
		"setapplication synchrange=x\n"
		"setapplication autoreloaddata=yes\n\n"
		"addwindow ti=\"Digital\"\n"
		"addgraph ti=\"Digital\" type=digital\n" 
		"setgraph xlabel=\"" << xLabel << "\" xunit=\"" << xUnit << "\"\n"
		"setgraph legendpos=13\n"
		<< "\n";

	for (auto const &formattedColumn : waveforms) {

		Column const *column = formattedColumn.Column;
		auto signalType = column != nullptr ? column->signalType : SignalType::Boolean; // only the clock signal, which is boolean, has Column == nullptr.
		if (signalType != SignalType::Analogue) {

			if (column != nullptr && column->IsBus) {

				int firstElement = (column->BusFlags & Flags::ReversedElementOrder) != 0 ? column->BusWidth - 1 : 0;
				int direction = (column->BusFlags & Flags::ReversedElementOrder) != 0 ? -1 : 1;

				for (int i = 0; i < column->BusWidth; ++i) {

					std::string indexStr = (column->BusFlags & Flags::OneBasedIndexing) != 0
						? string_printf("(%d)", firstElement + i*direction + 1)
						: string_printf("[%d]", firstElement + i*direction);

					script <<
						"addcurvetxt \"" << formattedColumn.FileName << ".txt\" 1:" << firstElement + i*direction + 2 << "\n"
						"setcurve title=\"" << formattedColumn.Name << indexStr << "\" identifier="" visible=yes inlegend=yes\n"
						"setcurve digiconnect=" << (signalType == SignalType::Boolean ? 1 : 4) << "\n"
						<< "\n";
				}
			}
			else {

				script <<
					"addcurvetxt \"" << formattedColumn.FileName << ".txt\" 1:2\n"
					"setcurve title=\"" << formattedColumn.Name << "\" identifier="" visible=yes inlegend=yes\n"
					"setcurve digiconnect=" << (signalType == SignalType::Boolean ? 1 : 4) << "\n"
					<< "\n";
			}
		}
	}

	// Analogue graphs
	script <<
		"addwindow ti=\"Analogue\"\n"
		"addgraph ti=\"Analogue\" type=analog\n"
		"setgraph xlabel=\"" << xLabel << "\" xunit=\"" << xUnit << "\"\n"
		"setgraph legendpos=13\n"
		<< "\n";

	for (auto const &formattedColumn : waveforms) {

		Column const *column = formattedColumn.Column;
		auto signalType = column != nullptr ? column->signalType : SignalType::Boolean; // only the clock signal, which is boolean, has Column == nullptr.

		if (signalType == SignalType::Analogue) {

			if (column != nullptr && column->IsBus) {

				int firstElement = (column->BusFlags & Flags::ReversedElementOrder) != 0 ? column->BusWidth - 1 : 0;
				int direction = (column->BusFlags & Flags::ReversedElementOrder) != 0 ? -1 : 1;

				for (int i = 0; i < column->BusWidth; ++i) {

					std::string indexStr = (column->BusFlags & Flags::OneBasedIndexing) != 0
						? string_printf("(%d)", firstElement + i*direction + 1)
						: string_printf("[%d]", firstElement + i*direction);

					script <<
						"addcurvetxt \"" << formattedColumn.FileName << ".txt\" 1:" << firstElement + i*direction + 2 << "\n"
						"setcurve title=\"" << formattedColumn.Name << indexStr << "\" identifier="" visible=yes inlegend=yes\n"
						"setcurve connect=4\n"
						<< "\n";
				}
			}
			else {

				script <<
					"addcurvetxt \"" << formattedColumn.FileName << ".txt\" 1:2\n"
					"setcurve title=\"" << formattedColumn.Name << "\" identifier="" visible=yes inlegend=yes\n"
					"setcurve connect=4\n"
					<< "\n";
			}
		}
	}

	script <<
		"setapplication arrangewindows=1\n"
		"setapplication freeze=yes\n"
		<< "\n";
}

}
}
