/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>

#include <algorithm>

#include "EPModelFixture.hpp"
#include "../AirToAirComponent/HeatExchangerAirToAirSensibleAndLatent.hpp"
#include "../HVACComponent/AirLoopHVACOutdoorAirSystem.hpp"
#include "../HVACComponent/ControllerWaterCoil.hpp"
#include "../Loop/AirLoopHVAC.hpp"
#include "../Loop/PlantLoop.hpp"
#include "../StraightComponent/CoilSystemCoolingWater.hpp"
#include "../StraightComponent/Node.hpp"
#include "../StraightComponent/CoilSystemCoolingWaterHeatExchangerAssisted.hpp"
#include "../WaterToAirComponent/CoilCoolingWater.hpp"
#include <utilities/idd/CoilSystem_Cooling_Water_FieldEnums.hxx>
#include <utilities/idd/CoilSystem_Cooling_Water_HeatExchangerAssisted_FieldEnums.hxx>

using namespace openstudio::epmodel;

TEST_F(EPModelFixture, CoilSystemCoolingWaterHeatExchangerAssisted_DefaultConstructor) {
  Model model;
  CoilSystemCoolingWaterHeatExchangerAssisted coilSystem(model);
  EXPECT_EQ(CoilSystemCoolingWaterHeatExchangerAssisted::iddObjectType(), coilSystem.iddObject().type());
  EXPECT_FALSE(coilSystem.nameString().empty());

  auto heatExchanger = coilSystem.heatExchanger();
  auto coolingCoil = coilSystem.coolingCoil();
  EXPECT_EQ("HeatExchanger:AirToAir:SensibleAndLatent", heatExchanger.iddObject().name());
  EXPECT_EQ("Coil:Cooling:Water", coolingCoil.iddObject().name());
  EXPECT_EQ(heatExchanger.iddObject().name(), coilSystem.heatExchangerObjectType());
  EXPECT_EQ(coolingCoil.iddObject().name(), coilSystem.coolingCoilObjectType());

  const auto children = coilSystem.children();
  EXPECT_EQ(2u, children.size());
  EXPECT_TRUE(std::any_of(children.begin(), children.end(), [&](const auto& object) { return object.handle() == coolingCoil.handle(); }));
  EXPECT_TRUE(std::any_of(children.begin(), children.end(), [&](const auto& object) { return object.handle() == heatExchanger.handle(); }));
}

TEST_F(EPModelFixture, CoilSystemCoolingWaterHeatExchangerAssisted_ScalarAccessors_RoundTrip) {
  Model model;
  CoilSystemCoolingWaterHeatExchangerAssisted coilSystem(model);

  const auto heatExchangerObjectTypes = CoilSystemCoolingWaterHeatExchangerAssisted::heatExchangerObjectTypeValues();
  EXPECT_FALSE(heatExchangerObjectTypes.empty());
  EXPECT_TRUE(std::find(heatExchangerObjectTypes.begin(), heatExchangerObjectTypes.end(), "HeatExchanger:AirToAir:SensibleAndLatent") !=
              heatExchangerObjectTypes.end());

  const auto coolingCoilObjectTypes = CoilSystemCoolingWaterHeatExchangerAssisted::coolingCoilObjectTypeValues();
  EXPECT_FALSE(coolingCoilObjectTypes.empty());
  EXPECT_TRUE(std::find(coolingCoilObjectTypes.begin(), coolingCoilObjectTypes.end(), "Coil:Cooling:Water") != coolingCoilObjectTypes.end());

  EXPECT_TRUE(coilSystem.setHeatExchangerObjectType("HeatExchanger:AirToAir:SensibleAndLatent"));
  EXPECT_EQ("HeatExchanger:AirToAir:SensibleAndLatent", coilSystem.heatExchangerObjectType());

  EXPECT_TRUE(coilSystem.setCoolingCoilObjectType("Coil:Cooling:Water"));
  EXPECT_EQ("Coil:Cooling:Water", coilSystem.coolingCoilObjectType());

  EXPECT_FALSE(coilSystem.setCoolingCoilObjectType("Invalid Coil Type"));
}

TEST_F(EPModelFixture, CoilSystemCoolingWaterHeatExchangerAssisted_RelationshipAccessors_RoundTrip) {
  Model model;
  HeatExchangerAirToAirSensibleAndLatent heatExchanger(model);
  CoilSystemCoolingWaterHeatExchangerAssisted coilSystem(model, heatExchanger);
  CoilCoolingWater coolingCoil(model);

  EXPECT_EQ(heatExchanger.handle(), coilSystem.heatExchanger().handle());
  EXPECT_EQ("Coil:Cooling:Water", coilSystem.coolingCoil().iddObject().name());

  EXPECT_TRUE(coilSystem.setCoolingCoil(coolingCoil));
  EXPECT_EQ(coolingCoil.handle(), coilSystem.coolingCoil().handle());
  EXPECT_EQ(coolingCoil.iddObject().name(), coilSystem.coolingCoilObjectType());

  HeatExchangerAirToAirSensibleAndLatent replacementHeatExchanger(model);
  EXPECT_TRUE(coilSystem.setHeatExchanger(replacementHeatExchanger));
  EXPECT_EQ(replacementHeatExchanger.handle(), coilSystem.heatExchanger().handle());
  EXPECT_EQ(replacementHeatExchanger.iddObject().name(), coilSystem.heatExchangerObjectType());
}

TEST_F(EPModelFixture, CoilSystemCoolingWaterHeatExchangerAssisted_AddToNodeRejected) {
  Model model;
  AirLoopHVAC airLoop(model);
  CoilSystemCoolingWaterHeatExchangerAssisted coilSystem(model);
  AirLoopHVACOutdoorAirSystem oaSystem(model);

  auto supplyInletNode = airLoop.supplyInletNode();
  EXPECT_FALSE(coilSystem.addToNode(supplyInletNode));
  EXPECT_FALSE(coilSystem.inletModelObject());
  EXPECT_FALSE(coilSystem.outletModelObject());

  auto supplyOutletNode = airLoop.supplyOutletNode();
  ASSERT_TRUE(oaSystem.addToNode(supplyOutletNode));
  auto outboardOANode = oaSystem.outboardOANode();
  ASSERT_TRUE(outboardOANode);
  EXPECT_FALSE(coilSystem.addToNode(*outboardOANode));
}

TEST_F(EPModelFixture, CoilSystemCoolingWaterHeatExchangerAssisted_ContainedCoolingCoilDoesNotCreateStandaloneController) {
  Model model;
  CoilSystemCoolingWater system(model);
  CoilSystemCoolingWaterHeatExchangerAssisted hxAssisted(model);
  CoilCoolingWater coil(model);
  AirLoopHVAC airLoop(model);
  PlantLoop plantLoop(model);

  ASSERT_TRUE(hxAssisted.setCoolingCoil(coil));
  ASSERT_TRUE(system.setCoolingCoil(hxAssisted));

  auto supplyInletNode = airLoop.supplyInletNode();
  ASSERT_TRUE(system.addToNode(supplyInletNode));
  ASSERT_TRUE(plantLoop.addDemandBranchForComponent(coil));

  EXPECT_FALSE(coil.controllerWaterCoil());
}
