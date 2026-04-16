/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>

#include <utilities/idd/DistrictCooling_FieldEnums.hxx>

#include "EPModelFixture.hpp"
#include "../Loop/AirLoopHVAC.hpp"
#include "../Loop/PlantLoop.hpp"
#include "../Schedule/Schedule.hpp"
#include "../Schedule/Schedule_Impl.hpp"
#include "../Schedule/ScheduleConstant.hpp"
#include "../Splitter/AirLoopHVACZoneSplitter.hpp"
#include "../StraightComponent/DistrictCooling.hpp"
#include "../StraightComponent/DistrictCooling_Impl.hpp"
#include "../StraightComponent/Node.hpp"

using namespace openstudio::epmodel;

TEST_F(EPModelFixture, DistrictCooling_DefaultConstructor) {
  Model model;
  DistrictCooling districtCooling(model);
  EXPECT_EQ(DistrictCooling::iddObjectType(), districtCooling.iddObject().type());
  EXPECT_FALSE(districtCooling.nameString().empty());
  EXPECT_TRUE(districtCooling.isNominalCapacityAutosized());
  EXPECT_FALSE(districtCooling.nominalCapacity());
  EXPECT_EQ(model.alwaysOnDiscreteSchedule().handle(), districtCooling.capacityFractionSchedule().handle());

  auto capacityFractionSchedule =
    districtCooling.getModelObjectTarget<Schedule>(openstudio::DistrictCoolingFields::CapacityFractionScheduleName);
  ASSERT_TRUE(capacityFractionSchedule);
  EXPECT_EQ(districtCooling.capacityFractionSchedule().handle(), capacityFractionSchedule->handle());
}

TEST_F(EPModelFixture, DistrictCooling_ScheduleAndScalarAccessors_RoundTrip) {
  Model model;
  DistrictCooling districtCooling(model);

  ScheduleConstant capacityFractionSchedule(model);
  capacityFractionSchedule.setValue(0.5);
  EXPECT_TRUE(districtCooling.setCapacityFractionSchedule(capacityFractionSchedule));
  EXPECT_EQ(capacityFractionSchedule.handle(), districtCooling.capacityFractionSchedule().handle());

  auto storedSchedule = districtCooling.getModelObjectTarget<Schedule>(openstudio::DistrictCoolingFields::CapacityFractionScheduleName);
  ASSERT_TRUE(storedSchedule);
  EXPECT_EQ(capacityFractionSchedule.handle(), storedSchedule->handle());

  EXPECT_TRUE(districtCooling.setNominalCapacity(12345.0));
  ASSERT_TRUE(districtCooling.nominalCapacity());
  EXPECT_DOUBLE_EQ(12345.0, districtCooling.nominalCapacity().get());
  EXPECT_FALSE(districtCooling.isNominalCapacityAutosized());

  auto districtCoolingCloneObject = model.addObject(districtCooling.clone());
  ASSERT_TRUE(districtCoolingCloneObject);
  auto districtCoolingClone = districtCoolingCloneObject->cast<DistrictCooling>();
  EXPECT_EQ(capacityFractionSchedule.handle(), districtCoolingClone.capacityFractionSchedule().handle());
  ASSERT_TRUE(districtCoolingClone.nominalCapacity());
  EXPECT_DOUBLE_EQ(12345.0, districtCoolingClone.nominalCapacity().get());
  EXPECT_FALSE(districtCoolingClone.isNominalCapacityAutosized());

  districtCooling.autosizeNominalCapacity();
  EXPECT_TRUE(districtCooling.isNominalCapacityAutosized());
  EXPECT_FALSE(districtCooling.nominalCapacity());

  EXPECT_FALSE(districtCooling.autosizedNominalCapacity());
}

TEST_F(EPModelFixture, DistrictCooling_AddToNode_PlantSupplyOnly) {
  Model model;
  DistrictCooling districtCooling(model);

  AirLoopHVAC airLoop(model);
  Node airSupplyOutletNode = airLoop.supplyOutletNode();
  EXPECT_FALSE(districtCooling.addToNode(airSupplyOutletNode));
  EXPECT_EQ(2u, airLoop.supplyComponents().size());

  Node airDemandNode = airLoop.zoneSplitter().lastOutletModelObject()->cast<Node>();
  EXPECT_FALSE(districtCooling.addToNode(airDemandNode));
  EXPECT_EQ(5u, airLoop.demandComponents().size());

  PlantLoop plantLoop(model);
  Node supplyOutletNode = plantLoop.supplyOutletNode();
  EXPECT_TRUE(districtCooling.addToNode(supplyOutletNode));
  EXPECT_EQ(7u, plantLoop.supplyComponents().size());
  ASSERT_TRUE(districtCooling.loop());
  ASSERT_TRUE(districtCooling.inletModelObject());
  ASSERT_TRUE(districtCooling.outletModelObject());
  EXPECT_TRUE(districtCooling.isRemovable());

  auto inletNode = districtCooling.getModelObjectTarget<Node>(openstudio::DistrictCoolingFields::ChilledWaterInletNodeName);
  auto outletNode = districtCooling.getModelObjectTarget<Node>(openstudio::DistrictCoolingFields::ChilledWaterOutletNodeName);
  ASSERT_TRUE(inletNode);
  ASSERT_TRUE(outletNode);
  EXPECT_EQ(districtCooling.inletModelObject()->handle(), inletNode->handle());
  EXPECT_EQ(districtCooling.outletModelObject()->handle(), outletNode->handle());

  Node demandOutletNode = plantLoop.demandOutletNode();
  EXPECT_FALSE(districtCooling.addToNode(demandOutletNode));
  EXPECT_EQ(5u, plantLoop.demandComponents().size());

  districtCooling.disconnect();
  EXPECT_FALSE(districtCooling.loop());
  EXPECT_FALSE(districtCooling.inletModelObject());
  EXPECT_FALSE(districtCooling.outletModelObject());

  auto districtCoolingCloneObject = model.addObject(districtCooling.clone());
  ASSERT_TRUE(districtCoolingCloneObject);
  auto districtCoolingClone = districtCoolingCloneObject->cast<DistrictCooling>();
  supplyOutletNode = plantLoop.supplyOutletNode();
  EXPECT_TRUE(districtCoolingClone.addToNode(supplyOutletNode));
  EXPECT_EQ(9u, plantLoop.supplyComponents().size());
  ASSERT_TRUE(districtCoolingClone.inletModelObject());
  ASSERT_TRUE(districtCoolingClone.outletModelObject());
}
