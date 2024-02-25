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

	The recorder can be used to record the inputs and outputs of a design
	during simulation. Recorded content can be written to file and used
	in a Verilog simulation to confirm the correctness of the generated
	code.

*/

#pragma once

namespace dfx {
namespace modules {

class Recorder;

}
namespace backend {
namespace blocks {

class recorder_block : public BlockBase {

private:
	dfx::modules::Recorder *Recorder;
    void GetProperties(dfx::generator::Properties &properties) const;
	int StimWidth;
	int RefWidth;
	void Evaluate() override { };
	bool CanEvaluate() const override {	return false; }
	source_blocks_t GetSourceBlocks() const override { return source_blocks_t(); }

public:
	recorder_block(dfx::modules::Recorder *Recorder);
	bool CanRemove() const { return false; };
};

class stimcheck_block_base {

protected:

	int startIndex;
	int bitWidth;

	std::string valueToString(bool val) const;
	std::string valueToString(const dynfix &val) const;

	int getNodeWidth(const node<bool> &) const;
	int getNodeWidth(const node<dynfix> &val) const;

public:
	stimcheck_block_base(const int startIndex, const int bitWidth) : startIndex(startIndex), bitWidth(bitWidth) {};
	virtual int getNumElements() const = 0;
	virtual int getElementWidth() const = 0;
	virtual std::string getElement(const int index) const = 0;
	virtual void clear() = 0;

	int getBitWidth() const;
	int getStartIndex() const;

	void GetPropertiesBase(dfx::generator::Properties &properties) const;

};

template<typename T> class stim_block : public BlockBase, private IStep, public stimcheck_block_base {

private:

	InputPin<T> input;
	OutputPin<T> output;

	std::vector<T> data;

	void Evaluate() override
	{
		output.value = input.GetValue();
	}

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;
		blocks.insert(input.GetDrivingBlock());
		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Step()
	{
		data.push_back(input.GetValue());
	}

	void AsyncReset() override
	{
	}

	IStep *GetStep() override
	{
		return this;
	}

public:
	stim_block(node<T> const &in, const int startIndex) :
		BlockBase("stimulus"),
		stimcheck_block_base(startIndex, getNodeWidth(in)),
		input(this, in),
		output(this, types::DefaultFrom(in.GetDriver()->value))
	{
	};

	node<T> getOutNode()
	{
		return output.GetNode();
	}

	int getNumElements() const override
	{
		return (int)data.size();
	}

	int getElementWidth() const override
	{
		return bitWidth;
	}

	std::string getElement(const int index) const override
	{
		return valueToString(data[index]);
	}

	void clear() override
	{
		data.clear();
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		stimcheck_block_base::GetPropertiesBase(properties);
	}

};

template<typename T> class checker_block : public BlockBase, private IStep, public stimcheck_block_base  {

private:

	InputPin<T> input;

	std::vector<T> data;

	void Evaluate() override { }

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;
		//blocks.insert(input.GetDrivingBlock());
		return blocks;
	}

	bool CanEvaluate() const override
	{
		return false;
	}

	void Step()
	{
		data.push_back(input.GetValue());
	}

	void AsyncReset() override
	{
	}

	IStep *GetStep() override
	{
		return this;
	}


public:

	checker_block(node<T> const &in, const int startIndex) :
		BlockBase("checker"),
		stimcheck_block_base(startIndex, getNodeWidth(in)),
		input(this, in)
	{
	};

	int getNumElements() const override
	{
		return (int)data.size();
	}

	int getElementWidth() const override
	{
		return bitWidth;
	}

	std::string getElement(const int index) const override
	{
		return valueToString(data[index]);
	}

	void clear() override
	{
		data.clear();
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		stimcheck_block_base::GetPropertiesBase(properties);
	}
};
}
}

namespace modules {

class Recorder
{

private:

	backend::blocks::recorder_block *rec_block;
	
	std::list<dfx::backend::blocks::stimcheck_block_base *> stims;
	std::list<dfx::backend::blocks::stimcheck_block_base *> checkers;

	int stim_index;
	int check_index;

	void internal_WriteToFile(const std::string &fileName, std::list<dfx::backend::blocks::stimcheck_block_base *> &list);
	std::string Binary2Hex(std::string &);

public:
	Recorder();

	node<bool>   AddStimulus(node<bool> const &in);
	node<dynfix> AddStimulus(node<dynfix> const &in);
	bus<bool>    AddStimulus(bus<bool> const &in_bus);
	bus<dynfix>  AddStimulus(bus<dynfix> const &in_bus);
	node<bool>   AddChecker(node<bool> const &in);
	node<dynfix> AddChecker(node<dynfix> const &in);
	bus<bool>    AddChecker(bus<bool> const &in_bus);
	bus<dynfix>  AddChecker(bus<dynfix> const &in_bus);

	void WriteToFile(const std::string &basePath);
	void Clear();

	int GetStimWidth();
	int GetRefWidth();
};

}
}
