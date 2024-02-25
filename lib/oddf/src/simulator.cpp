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

	The simulator core.

*/

#include "global.h"

#include "simulator.h"
#include "messages.h"
#include "hierarchy.h"

#include "simulator_optimisations.h"

namespace dfx {

static int const STATE_IDLE = 0;
static int const STATE_PROPAGATING = 1;
static int const STATE_STEPPING = 2;
static int const STATE_TERMINATING = 3;

Simulator::Simulator(Design const &design) :
	currentComponent(nullptr),
	runMutex(),
	runCv()
{
	for (auto &block : design.blocks)
		block->Simplify();

	// Collect all steppable blocks
	steppables.reserve(1000);
	for (auto &block : design.blocks) {

		backend::IStep *steppable = block->GetStep();
		if (steppable != nullptr)
			steppables.push_back(steppable);
	}

	//
	// Collect all components of the execution graph and sort them into topological order
	//

	components.emplace_back();
	reusableComponents.push_front(&components.back());

	/*
	// Shuffle the blocks to check the validity of the block sorting algorithms.
	std::deque<backend::BlockBase *> shuffledBlocks;
	for (auto &block : design.Blocks)
		shuffledBlocks.push_back(block.get());

	std::random_device rd;
	std::mt19937 g(rd());

	//std::shuffle(shuffledBlocks.begin(), shuffledBlocks.end(), g);

	for (auto *block : shuffledBlocks) {
	*/

	//int position = 0;
	for (auto &block : design.blocks) {

		if (block->CanEvaluate()/* && block->IsConnected()*/) {

			if (reusableComponents.empty()) {

#if defined(DFX_SIMULATOR_ENABLE_COMPONENTS) && (DFX_SIMULATOR_ENABLE_COMPONENTS == 1)

					components.emplace_back();
					currentComponent = &components.back();

#elif defined(DFX_SIMULATOR_ENABLE_COMPONENTS) && (DFX_SIMULATOR_ENABLE_COMPONENTS == 0)

#else

				DFX_SIMULATOR_ENABLE_COMPONENTS must be set to 0 or 1.

#endif
			}
			else {

				currentComponent = reusableComponents.front();
				reusableComponents.pop_front();
			}

			//currentComponent->sortingOrder = position++;
			RecursiveBuildExecutionOrder(block.get());
		}

	}

	components.remove_if([](backend::Component const &component) { return component.blocksFirst == nullptr; });

	components.sort([](backend::Component const &component1, backend::Component const &component2) { return component1.size / 256 > component2.size / 256; });

	//
	// Merge components into tasks of suitable size (does not seem to improve performance much)
	//

	int totalComponentCount = std::accumulate(components.begin(), components.end(), 0, [](int current, backend::Component const &component) { return current + component.size; });
	int minTaskSize = totalComponentCount / 200;

	tasks.emplace_back();
	int currentSize = 0;
	for (auto &component : components) {

		if (currentSize >= minTaskSize) {

			currentSize = 0;
			tasks.emplace_back();
		}

		tasks.back().push_back(&component);
		currentSize += component.size;
	}

	//
	// Create background threads
	//

#if defined(DFX_SIMULATOR_USE_MULTITHREADING) && (DFX_SIMULATOR_USE_MULTITHREADING == 1)

	int numberOfThreads = (int)std::thread::hardware_concurrency() - 1;

#elif defined(DFX_SIMULATOR_USE_MULTITHREADING) && (DFX_SIMULATOR_USE_MULTITHREADING == 0)

	int numberOfThreads = 1;

#else

	DFX_SIMULATOR_USE_MULTITHREADING must be set to 0 or 1.

#endif

	if (numberOfThreads <= 0)
		numberOfThreads = 1;

	for (int i = 0; i < numberOfThreads - 1; ++i) {

		runStates.push_back(STATE_IDLE);
		runThreads.push_back(std::thread(&Simulator::RunWorkerThread, this, &runStates.back()));
	}
}



Simulator::~Simulator()
{
	std::unique_lock<std::mutex> lock(runMutex);

	std::fill(runStates.begin(), runStates.end(), STATE_TERMINATING);

	lock.unlock();
	runCv.notify_all();

	while (!runThreads.empty()) {

		runThreads.back().join();
		runThreads.pop_back();
	}
}

void Simulator::RecursiveBuildExecutionOrder(backend::BlockBase *current)
{
	if (!current->CanEvaluate())
		return;

	if (current->mark) {

		design_info("A computational cycle was detected involving block '" + current->GetFullName() + "'.");
		throw design_error("The design contains at least one computational cycle. This means that an output of a block eventually leads to an input of the same block without at least one delay element in the path.");
	}

	if (current->component) {

		if (current->component != currentComponent) {

			backend::Component *toComponent = current->component;
			backend::Component *fromComponent = currentComponent;

			// Making sure that the smaller list becomes mergerd into the larger one 
			// gives a significant performance improvement.
			// TODO: are we really free to swap the order in which the two lists are merged?
			if (fromComponent->size > toComponent->size)
				std::swap(toComponent, fromComponent);

			if (fromComponent->size > 0) {

				for (auto *block = fromComponent->blocksFirst; block != nullptr; block = block->componentNext)
					block->component = toComponent;

				toComponent->size += fromComponent->size;

				// concatenation order based on sortingOrder does not yield the expected speed improvement. Keep it here for reference.
				//if (toComponent->sortingOrder < fromComponent->sortingOrder) {

					*toComponent->blocksEnd = fromComponent->blocksFirst;
					toComponent->blocksEnd = fromComponent->blocksEnd;
				/*}
				else {

					*fromComponent->blocksEnd = toComponent->blocksFirst;
					fromComponent->blocksEnd = toComponent->blocksEnd;

					toComponent->sortingOrder = fromComponent->sortingOrder;
					toComponent->blocksFirst = fromComponent->blocksFirst;
					toComponent->blocksEnd = fromComponent->blocksEnd;
				}*/
			}

			fromComponent->size = 0;
			fromComponent->blocksFirst = nullptr;
			fromComponent->blocksEnd = &fromComponent->blocksFirst;
			reusableComponents.push_front(fromComponent);

			currentComponent = toComponent;
		}

		return;
	}

	current->mark = true;

	// https://en.wikipedia.org/wiki/Topological_sorting
	for (auto *block : current->GetSourceBlocks())
		RecursiveBuildExecutionOrder(block);

	current->mark = false;
	current->component = currentComponent;

	++currentComponent->size;
	*currentComponent->blocksEnd = current;
	currentComponent->blocksEnd = &current->componentNext;
}

void Simulator::PropagateCore()
{
	int index = currentTaskIndex.fetch_add(1, std::memory_order_relaxed);

	while (index < (int)tasks.size()) {

		auto const &task = tasks[index];

		for (auto *component : task) {

			if (component->outdated) {

				component->outdated = false;

				for (auto *block = component->blocksFirst; block != nullptr; block = block->componentNext)
					block->Evaluate();
			}
		}

		index = currentTaskIndex.fetch_add(1, std::memory_order_relaxed);
	}
}

void Simulator::RunWorkerThread(int *state)
{
	std::unique_lock<std::mutex> lock(runMutex);

	while (*state != STATE_TERMINATING) {

		runCv.wait(lock, [state] { return *state != STATE_IDLE; });
		lock.unlock();

		switch (*state) {

			case STATE_TERMINATING:
				return;

			case STATE_PROPAGATING:
				PropagateCore();
				break;

			case STATE_STEPPING:
				StepCore();
				break;
		}

		lock.lock();
		*state = STATE_IDLE;
		runCv.notify_all();
	}
}

void Simulator::Propagate()
{
	std::unique_lock<std::mutex> lock(runMutex);

	currentTaskIndex = 0;
	std::fill(runStates.begin(), runStates.end(), STATE_PROPAGATING);
	runCv.notify_all();
	lock.unlock();

	PropagateCore();

	lock.lock();
	runCv.wait(lock, [this] { 
		return std::all_of(runStates.cbegin(), runStates.cend(), [](int state) { return state == STATE_IDLE; });
	});
}

void Simulator::StepCore()
{
	int index = currentSteppableIndex.fetch_add(1, std::memory_order_relaxed);

	while (index < (int)steppables.size()) {

		steppables[index]->Step();
		index = currentSteppableIndex.fetch_add(1, std::memory_order_relaxed);
	}
}

	

void Simulator::Step()
{
	std::unique_lock<std::mutex> lock(runMutex);

	currentSteppableIndex = 0;
	std::fill(runStates.begin(), runStates.end(), STATE_STEPPING);
	runCv.notify_all();
	lock.unlock();

	StepCore();

	lock.lock();
	runCv.wait(lock, [this] {
		return std::all_of(runStates.cbegin(), runStates.cend(), [](int state) { return state == STATE_IDLE; });
	});
}

void Simulator::Run(unsigned numberOfIterations /* = 1 */)
{
	Propagate();

	while (numberOfIterations-- > 0) {

		Step();
		Propagate();
	}
}

void Simulator::AsyncReset()
{
	for (auto *steppable : steppables)
		steppable->AsyncReset();

	Propagate();
}

void Simulator::Report(std::basic_ostream<char> &os) const
{
	using std::endl;
	using std::setw;

	std::map<int, int> componentSizes;
	for (auto const &component : components)
		componentSizes[(int)std::ceil(std::log2(component.size))]++;

	os << " --- Simulator --- " << endl << endl;

	os << " Number of components        : " << components.size() << endl;
	os << " Number of computable blocks : " << std::accumulate(components.begin(), components.end(), 0, [](int current, backend::Component const &component) { return current + component.size; }) << endl;
	os << " Number of steppable blocks  : " << steppables.size() << endl;
	os << " Number of tasks             : " << tasks.size() << endl;
	os << " Number of parallel threads  : " << runThreads.size() + 1 << endl;

	os << endl;
	os << " Components by size" << endl;
	os << endl;

	for (auto const &x : componentSizes)
		os << setw(10) << (1 << x.first) << " : " << x.second << endl;

	os << endl;
}


}
