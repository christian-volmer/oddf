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

#pragma once

namespace dfx {
namespace backend {

class logger_BlockBase {

public:

	// Detaches this logger block from the owning Logger object.
	virtual void detach() = 0;

	// Clear all logged data
	virtual void clear() = 0;

	// Number of elements logged
	virtual int get_length() const = 0;

	// Logged data as a vector of strings.
	virtual std::vector<std::string> get_formatted_list() const = 0;
};

class logger_callback {

public:

	virtual bool IsEnabled() const = 0;
	virtual void NotifyRemoval(logger_BlockBase *loggerBlock) = 0;
};

}

namespace modules {

class Logger : private backend::logger_callback {

private:

	bool Enabled;

	enum struct SignalType : int {

		Boolean = 0,
		Integer = 1,
		Analogue = 2
	};


	struct Column {

		std::string Tag;
		std::string Name;
		backend::logger_BlockBase *Block;
		SignalType signalType;
		bool IsBus;
		int BusWidth;
		int Separation;
		int BusFlags;
	};

	std::vector<Column> Columns;

	virtual void NotifyRemoval(backend::logger_BlockBase *loggerBlock) override;

public:

	struct Flags {

		static int const ZeroBasedIndexing = 0;
		static int const OneBasedIndexing = 1;
		static int const NormalElementOrder = 0;
		static int const ReversedElementOrder = 2;
	};

	Logger();
	Logger(Logger const &) = delete;
	~Logger();

	Logger &operator =(Logger const &) = delete;

	//
	// Log functions with user-defined tag.
	//

	void Log(std::string const &tag, std::string const &name, node<bool> const &node, char const *format = "%d");
	void Log(std::string const &tag, std::string const &name, node<std::int32_t> const &node, char const *format = "%" PRId32);
	void Log(std::string const &tag, std::string const &name, node<std::int64_t> const &node, char const *format = "%" PRId64);
	void Log(std::string const &tag, std::string const &name, node<double> const &node, char const *format = "%.17g");
	void Log(std::string const &tag, std::string const &name, node<dynfix> const &node, char const *format = "%.17g");

	void Log(std::string const &tag, std::string const &name, bus_access<bool> const &bus, int flags = 0, int separation = 1, char const *format = "%d");
	void Log(std::string const &tag, std::string const &name, bus_access<std::int32_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId32);
	void Log(std::string const &tag, std::string const &name, bus_access<std::int64_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId64);
	void Log(std::string const &tag, std::string const &name, bus_access<double> const &bus, int flags = 0, int separation = 1, char const *format = "%g");
	void Log(std::string const &tag, std::string const &name, bus_access<dynfix> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");

	void LogSequence(std::string const &tag, std::string const &name, bus_access<bool> const &bus, int flags = 0, int separation = 1, char const *format = "%d");
	void LogSequence(std::string const &tag, std::string const &name, bus_access<std::int32_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId32);
	void LogSequence(std::string const &tag, std::string const &name, bus_access<std::int64_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId64);
	void LogSequence(std::string const &tag, std::string const &name, bus_access<double> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");
	void LogSequence(std::string const &tag, std::string const &name, bus_access<dynfix> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");

	//
	// Log functions with default tag = ""
	//

	void Log(std::string const &name, node<bool> const &node, char const *format = "%d");
	void Log(std::string const &name, node<std::int32_t> const &node, char const *format = "%" PRId32);
	void Log(std::string const &name, node<std::int64_t> const &node, char const *format = "%" PRId64);
	void Log(std::string const &name, node<double> const &node, char const *format = "%.17g");
	void Log(std::string const &name, node<dynfix> const &node, char const *format = "%.17g");

	void Log(std::string const &name, bus_access<bool> const &bus, int flags = 0, int separation = 1, char const *format = "%d");
	void Log(std::string const &name, bus_access<std::int32_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId32);
	void Log(std::string const &name, bus_access<std::int64_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId64);
	void Log(std::string const &name, bus_access<double> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");
	void Log(std::string const &name, bus_access<dynfix> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");

	void LogSequence(std::string const &name, bus_access<bool> const &bus, int flags = 0, int separation = 1, char const *format = "%d");
	void LogSequence(std::string const &name, bus_access<std::int32_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId32);
	void LogSequence(std::string const &name, bus_access<std::int64_t> const &bus, int flags = 0, int separation = 1, char const *format = "%" PRId64);
	void LogSequence(std::string const &name, bus_access<double> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");
	void LogSequence(std::string const &name, bus_access<dynfix> const &bus, int flags = 0, int separation = 1, char const *format = "%.17g");


	// Clears all logged content.
	void Clear();

	// Disables logging to save memory and speed up the simulation.
	void Disable();

	// Enables logging.
	void Enable();

	// Checks if logging is enabled.
	virtual bool IsEnabled() const override;

	// Writes the logged data with default tag ("") to the specified stream in table format.
	void WriteTable(std::basic_ostream<char> &os) const;

	// Writes the logged data with the given tags to the specified stream in table format.
	void WriteTable(std::basic_ostream<char> &os, std::unordered_set<std::string> const &tags) const;

	// Exports the logged data with the specified tags in a format compatible with VaryPlot with a custom x-axis.
	void ExportToVaryPlot(std::string const &basePath, std::unordered_set<std::string> const &tags, std::string const &xLabel, std::string const &xUnit, double xScaling) const;

	// Exports the logged data with the specified tags in a format compatible with VaryPlot.
	void ExportToVaryPlot(std::string const &basePath, std::unordered_set<std::string> const &tags) const;

	// Exports the logged data with default tag ("") in a format compatible with VaryPlot.
	void ExportToVaryPlot(std::string const &basePath) const;

};


//
// Node logging without explicit tag
//

inline void Logger::Log(std::string const &name, node<bool> const &node, char const *format /* = "%d" */)
{
	Log("", name, node, format);
}

inline void Logger::Log(std::string const &name, node<std::int32_t> const &node, char const *format /* = "%" PRId32 */)
{
	Log("", name, node, format);
}

inline void Logger::Log(std::string const &name, node<std::int64_t> const &node, char const *format /* = "%" PRId64 */)
{
	Log("", name, node, format);
}

inline void Logger::Log(std::string const &name, node<double> const &node, char const *format /* = "%g" */)
{
	Log("", name, node, format);
}

inline void Logger::Log(std::string const &name, node<dynfix> const &node, char const *format /* = "%g" */)
{
	Log("", name, node, format);
}


//
// Bus logging without explicit tag
//

inline void Logger::Log(std::string const &name, bus_access<bool> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%d" */)
{
	Log("", name, bus, flags, separation, format);
}

inline void Logger::Log(std::string const &name, bus_access<std::int32_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId32 */)
{
	Log("", name, bus, flags, separation, format);
}

inline void Logger::Log(std::string const &name, bus_access<std::int64_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId64 */)
{
	Log("", name, bus, flags, separation, format);
}

inline void Logger::Log(std::string const &name, bus_access<double> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	Log("", name, bus, flags, separation, format);
}

inline void Logger::Log(std::string const &name, bus_access<dynfix> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	Log("", name, bus, flags, separation, format);
}




//
// Sequence logging without explicit tag
//

inline void Logger::LogSequence(std::string const &name, bus_access<bool> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%d" */)
{
	LogSequence("", name, bus, flags, separation, format);
}

inline void Logger::LogSequence(std::string const &name, bus_access<std::int32_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId32 */)
{
	LogSequence("", name, bus, flags, separation, format);
}

inline void Logger::LogSequence(std::string const &name, bus_access<std::int64_t> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%" PRId64 */)
{
	LogSequence("", name, bus, flags, separation, format);
}

inline void Logger::LogSequence(std::string const &name, bus_access<double> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	LogSequence("", name, bus, flags, separation, format);
}

inline void Logger::LogSequence(std::string const &name, bus_access<dynfix> const &bus, int flags /* = 0 */, int separation /* = 1 */, char const *format /* = "%g" */)
{
	LogSequence("", name, bus, flags, separation, format);
}


//
// Wrappers for exporting without explicit tag
//

inline void Logger::WriteTable(std::basic_ostream<char> &os) const
{
	WriteTable(os, { "" });
}

inline void Logger::ExportToVaryPlot(std::string const &basePath, std::unordered_set<std::string> const &tags) const
{
	ExportToVaryPlot(basePath, tags, "clock", "cycles", 1);
}


inline void Logger::ExportToVaryPlot(std::string const &basePath) const
{
	ExportToVaryPlot(basePath, { "" });
}


}
}
