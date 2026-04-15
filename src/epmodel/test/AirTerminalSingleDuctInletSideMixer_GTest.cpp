/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>

#include <algorithm>

#include "EPModelFixture.hpp"
#include "../StraightComponent/AirTerminalSingleDuctInletSideMixer.hpp"
#include "../HVACComponent/ThermalZone.hpp"
#include "../Loop/AirLoopHVAC.hpp"
#include "../ModelObject/ModelObject.hpp"
#include "../ModelObject/ZoneHVACAirDistributionUnit.hpp"
#include "../ModelObject/ZoneHVACAirDistributionUnit_Impl.hpp"
#include "../StraightComponent/Node.hpp"
#include "../Splitter/AirLoopHVACZoneSplitter.hpp"

#include <utilities/idd/OS_AirTerminal_SingleDuct_InletSideMixer_FieldEnums.hxx>

using namespace openstudio::epmodel;

TEST_F(EPModelFixture, AirTerminalSingleDuctInletSideMixer_DefaultConstructor) {
  Model model;
  AirTerminalSingleDuctInletSideMixer terminal(model);
  EXPECT_EQ(AirTerminalSingleDuctInletSideMixer::iddObjectType(), terminal.iddObject().type());
  EXPECT_FALSE(terminal.nameString().empty());

  EXPECT_TRUE(terminal.controlForOutdoorAir());
  EXPECT_EQ("CurrentOccupancy", terminal.perPersonVentilationRateMode());
  EXPECT_FALSE(terminal.secondaryAirInletNode());
}

TEST_F(EPModelFixture, AirTerminalSingleDuctInletSideMixer_ScalarAccessors_RoundTrip) {
  Model model;
  AirTerminalSingleDuctInletSideMixer terminal(model);

  EXPECT_TRUE(terminal.controlForOutdoorAir());
  EXPECT_TRUE(terminal.setControlForOutdoorAir(true));
  EXPECT_TRUE(terminal.controlForOutdoorAir());

  EXPECT_TRUE(terminal.setPerPersonVentilationRateMode("DesignOccupancy"));
  EXPECT_EQ("DesignOccupancy", terminal.perPersonVentilationRateMode());

  EXPECT_FALSE(terminal.setPerPersonVentilationRateMode("InvalidChoice"));

  const auto values = AirTerminalSingleDuctInletSideMixer::perPersonVentilationRateModeValues();
  EXPECT_FALSE(values.empty());
  EXPECT_NE(values.end(), std::find(values.begin(), values.end(), "CurrentOccupancy"));
  EXPECT_NE(values.end(), std::find(values.begin(), values.end(), "DesignOccupancy"));
}

TEST_F(EPModelFixture, AirTerminalSingleDuctInletSideMixer_Relationships_RoundTrip) {
  Model model;
  AirTerminalSingleDuctInletSideMixer terminal(model);
  Node secondaryAirNode(model);

  ASSERT_TRUE(
    terminal.setPointer(openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::TerminalUnitSecondaryAirInlet, secondaryAirNode.handle()));
  auto secondary = terminal.secondaryAirInletNode();
  ASSERT_TRUE(secondary);
  EXPECT_EQ(secondaryAirNode, *secondary);
}

TEST_F(EPModelFixture, AirTerminalSingleDuctInletSideMixer_AddToNode_RejectsInvalidNodesAndContexts) {
  Model model;
  AirLoopHVAC airLoop(model);
  ThermalZone zone(model);
  AirTerminalSingleDuctInletSideMixer terminal(model);
  Node standaloneNode(model);
  auto supplyInletNode = airLoop.supplyInletNode();
  auto zoneAirNode = zone.zoneAirNode();

  EXPECT_FALSE(terminal.addToNode(standaloneNode));
  EXPECT_FALSE(terminal.addToNode(supplyInletNode));
  EXPECT_FALSE(terminal.addToNode(zoneAirNode));
  EXPECT_FALSE(terminal.inletModelObject());
  EXPECT_FALSE(terminal.outletModelObject());
  EXPECT_FALSE(terminal.airLoopHVAC());
}

TEST_F(EPModelFixture, AirTerminalSingleDuctInletSideMixer_AddToNode_ResolvesAirLoopHVACRegistersZoneEquipmentAndPreservesSecondaryAirNode) {
  Model model;
  AirLoopHVAC airLoop(model);
  ThermalZone zone(model);
  AirTerminalSingleDuctInletSideMixer terminal(model);
  ZoneHVACAirDistributionUnit adu(model);
  Node secondaryAirNode(model);
  secondaryAirNode.setName("Secondary Air Node");

  ASSERT_TRUE(
    terminal.setPointer(openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::TerminalUnitSecondaryAirInlet, secondaryAirNode.handle()));
  auto aduImpl = adu.getImpl<detail::ZoneHVACAirDistributionUnit_Impl>();
  ASSERT_TRUE(aduImpl);
  ASSERT_TRUE(aduImpl->setAirTerminal(terminal.cast<ModelObject>()));

  auto branchObject = airLoop.zoneSplitter().lastOutletModelObject();
  ASSERT_TRUE(branchObject);
  auto branchNode = branchObject->optionalCast<Node>();
  ASSERT_TRUE(branchNode);
  ASSERT_TRUE(zone.addToNode(*branchNode));

  auto zoneAirNode = zone.zoneAirNode();
  ASSERT_TRUE(terminal.addToNode(zoneAirNode));

  auto linkedAirLoop = terminal.airLoopHVAC();
  ASSERT_TRUE(linkedAirLoop);
  EXPECT_EQ(airLoop, *linkedAirLoop);

  auto inletObject = terminal.inletModelObject();
  ASSERT_TRUE(inletObject);
  auto inletNode = inletObject->optionalCast<Node>();
  ASSERT_TRUE(inletNode);
  EXPECT_NE(zoneAirNode, *inletNode);

  auto outletObject = terminal.outletModelObject();
  ASSERT_TRUE(outletObject);
  auto outletNode = outletObject->optionalCast<Node>();
  ASSERT_TRUE(outletNode);
  EXPECT_EQ(zoneAirNode, *outletNode);

  auto resolvedSecondaryAirNode = terminal.secondaryAirInletNode();
  ASSERT_TRUE(resolvedSecondaryAirNode);
  EXPECT_EQ(secondaryAirNode, *resolvedSecondaryAirNode);

  auto resolvedOutletNode = adu.outletNode();
  ASSERT_TRUE(resolvedOutletNode);
  EXPECT_EQ(zoneAirNode, resolvedOutletNode.get());

  const auto equipment = zone.equipment();
  ASSERT_EQ(1u, equipment.size());
  EXPECT_EQ(terminal.cast<ModelObject>(), equipment.front());
}
