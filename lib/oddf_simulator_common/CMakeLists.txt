add_library(oddf_simulator_common
	"src/oddf/simulator/common/Simulator.cpp"
	"src/oddf/simulator/common/backend/blocks/BooleanFlat/BooleanFlat.cpp"
	"src/oddf/simulator/common/backend/blocks/Constant/Constant.cpp"
	"src/oddf/simulator/common/backend/blocks/delay/DelayEndpoint.cpp"
	"src/oddf/simulator/common/backend/blocks/delay/DelayMaster.cpp"
	"src/oddf/simulator/common/backend/blocks/minus/Minus.cpp"
	"src/oddf/simulator/common/backend/blocks/floor_cast/FloorCast.cpp"
	"src/oddf/simulator/common/backend/blocks/not/NotMaster.cpp"
	"src/oddf/simulator/common/backend/blocks/Probe/Probe.cpp"
	"src/oddf/simulator/common/backend/blocks/plus/plus.cpp"
	"src/oddf/simulator/common/backend/blocks/Signal/Signal.cpp"
	"src/oddf/simulator/common/backend/blocks/Identity.cpp"
	"src/oddf/simulator/common/backend/blocks/Temp.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/BuildComponents.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/ElaborateBlocks.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/FinaliseTranslation.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/GenerateCode.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/MapBlocks.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/MapConnections.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/NamedSimulatorNode.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/NamedSimulatorNodeComparer.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/RegisterDefaultBlockFactories.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/RegisterSimulatorBlockFactory.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/Run.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/SimulatorNodeHierarchyNode.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/SimulatorNodeHierarchyNodeComparer.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore/TranslateDesign.cpp"
	"src/oddf/simulator/common/backend/types/Boolean.cpp"
	"src/oddf/simulator/common/backend/types/FixedPoint.cpp"
	"src/oddf/simulator/common/backend/SimulatorBlockBase.cpp"
	"src/oddf/simulator/common/backend/SimulatorBlockBase/Internals.cpp"
	"src/oddf/simulator/common/backend/SimulatorBlockInput.cpp"
	"src/oddf/simulator/common/backend/SimulatorBlockOutput.cpp"
	"src/oddf/simulator/common/backend/SimulatorBlockOutput/SimulatorNode.cpp"
	"src/oddf/simulator/common/backend/SimulatorComponent.cpp"
	"src/oddf/simulator/common/backend/SimulatorCore.cpp"
)

target_include_directories(oddf_simulator_common PUBLIC include)

target_compile_features(oddf_simulator_common PUBLIC cxx_std_17)

target_link_libraries(oddf_simulator_common PUBLIC oddf_core)
