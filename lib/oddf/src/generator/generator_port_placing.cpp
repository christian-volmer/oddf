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

	Classes that support the generation of code (e.g., Verilog) from the
	design. Implements the identification and placement of module inputs
	and outputs.

*/

#include "../global.h"

#include "generator.h"
#include "../hierarchy.h"
#include "../formatting.h"

namespace dfx {
namespace generator {

bool Generator::PlaceInputPorts(bool firstPass)
{
	int addedCount = 0;

	for (auto &instance : instances) {

		for (auto &ent : instance.entities) {

			if (firstPass == false && ent.className != "$input_connector")
				continue;
			else if (ent.className == "input_port")
				continue;

			bool breakOut = true;
			while (breakOut) {

				breakOut = false;

				for (auto &input : ent.inputs) {

					auto *driver = input.driver;

					// Check if this input is driven by an entity from "outside" the current instance.
					if (!((driver->entity->instance == &instance) || driver->entity->instance->ChildOf(&instance))) {

						std::list<Entity::Input *> inputBunch;

						std::copy_if(
							driver->targets.begin(),
							driver->targets.end(),
							std::back_inserter(inputBunch),
							[&instance](Entity::Input *in) {

							return (in->entity->instance == &instance) || in->entity->instance->ChildOf(&instance);
						});

						instance.entities.emplace_back(
							&instance,
							"input_port",
							string_printf("input_port%d", (int)instance.inputPorts.size() + 1),
							1, 1
							);

						Entity &inputPort = instance.entities.back();
						inputPort.outputs[0].type = driver->type;

						for (Entity::Input *in : inputBunch) {

							BreakConnection(*in);
							MakeConnection(inputPort.outputs[0], *in);
						}

						MakeConnection(*driver, inputPort.inputs[0]);

						instance.inputPorts.push_back(&inputPort);
						++addedCount;

						breakOut = true;
						break;
					}

					if (breakOut)
						break;
				}
			}
		}
	}

	return addedCount > 0;
}

bool Generator::PlaceInputConnectors()
{
	int addedCount = 0;

	for (auto &instance : instances) {

		for (auto *inputPort : instance.inputPorts) {

			if (inputPort->className != "input_port")
				continue;

			if (inputPort->inputs[0].driver->entity->className == "$input_connector")
				continue;

			Entity::Output *driver = inputPort->inputs[0].driver;
			Instance &parent = *instance.parent;

			parent.entities.emplace_back(&parent, "$input_connector", "", 1, 1);

			Entity &inputConnector = parent.entities.back();
			inputConnector.outputs[0].type = driver->type;

			BreakConnection(inputPort->inputs[0]);
			MakeConnection(*driver, inputConnector.inputs[0]);
			MakeConnection(inputConnector.outputs[0], inputPort->inputs[0]);

			++addedCount;
		}
	}

	return addedCount > 0;
}

bool Generator::PlaceOutputPorts(bool firstPass)
{
	int addedCount = 0;

	for (auto &instance : instances) {

		for (auto &ent : instance.entities) {

			if (firstPass == false && ent.className != "$output_connector")
				continue;
			else if (ent.className == "output_port")
				continue;

			for (auto &output : ent.outputs) {

				auto &targets = output.targets;

				std::list<Entity::Input *> targetBunch;

				std::copy_if(
					targets.begin(),
					targets.end(),
					std::back_inserter(targetBunch),
					[&instance](Entity::Input *target) {

					return !((target->entity->instance == &instance) || target->entity->instance->ChildOf(&instance));
				});

				if (targetBunch.size() > 0) {

					instance.entities.emplace_back(
						&instance,
						"output_port",
						string_printf("output_port%d", (int)instance.outputPorts.size() + 1),
						1, 1
						);

					Entity &outputPort = instance.entities.back();
					outputPort.outputs[0].type = output.type;

					for (Entity::Input *target : targetBunch) {

						BreakConnection(*target);
						MakeConnection(outputPort.outputs[0], *target);
					}

					MakeConnection(output, outputPort.inputs[0]);

					instance.outputPorts.push_back(&outputPort);
					++addedCount;
				}
			}
		}
	}

	return addedCount > 0;
}


bool Generator::PlaceOutputConnectors()
{
	int addedCount = 0;

	for (auto &instance : instances) {

		for (auto *outputPort : instance.outputPorts) {

			if (outputPort->className != "output_port")
				continue;

			if ((outputPort->outputs[0].targets.size() == 1) && (outputPort->outputs[0].targets.front()->entity->className == "$output_connector"))
				continue;

			Instance &parent = *instance.parent;
			parent.entities.emplace_back(&parent, "$output_connector", "", 1, 1);

			Entity &outputConnector = parent.entities.back();
			outputConnector.outputs[0].type = outputPort->outputs[0].type;

			std::vector<Entity::Input *> targets(outputPort->outputs[0].targets.begin(), outputPort->outputs[0].targets.end());

			for (auto *target : targets) {

				BreakConnection(*target);
				MakeConnection(outputConnector.outputs[0], *target);
			}

			MakeConnection(outputPort->outputs[0], outputConnector.inputs[0]);

			++addedCount;
		}
	}

	return addedCount > 0;
}

void Generator::PlacePorts(std::basic_ostream<char> &os)
{
	os << "Placing input and output ports..." << std::endl;

	bool doContinue;

	doContinue = PlaceInputPorts(true);
	while (doContinue) {

		PlaceInputConnectors();
		doContinue = PlaceInputPorts(false);
	}

	doContinue = PlaceOutputPorts(true);
	while (doContinue) {

		PlaceOutputConnectors();
		doContinue = PlaceOutputPorts(false);
	}
}

}
}
