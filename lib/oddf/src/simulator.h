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

	Simulator class and related classes.

*/

#pragma once

#include "node.h"

namespace dfx {

namespace backend {

class Component {

public:

	int sortingOrder;
	int size;
	BlockBase *blocksFirst;
	BlockBase **blocksEnd;
	bool outdated;

	Component() : size(0), blocksFirst(nullptr), blocksEnd(&blocksFirst), outdated(true) {}
};

}

class Simulator {

private:

	std::list<backend::Component> components;
	std::list<backend::Component *> reusableComponents;
	backend::Component *currentComponent;

	std::vector<backend::IStep *> steppables;

	void RecursiveBuildExecutionOrder(backend::BlockBase *current);
	void Prepare();


	std::atomic<int> currentSteppableIndex;
	std::atomic<int> currentTaskIndex;
	std::vector<std::list<backend::Component *>> tasks;

	std::list<std::thread> runThreads;
	std::list<int> runStates;
	std::mutex runMutex;
	std::condition_variable runCv;

	void Propagate();
	void PropagateCore();

	void Step();
	void StepCore();

	void RunWorkerThread(int *state);

public:

	Simulator(Design const &design);
	~Simulator();

	void Run(unsigned numberOfIterations = 1);

	void AsyncReset();

	void Report(std::basic_ostream<char> &os) const;
};

}
