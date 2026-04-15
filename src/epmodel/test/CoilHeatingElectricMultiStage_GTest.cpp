/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>

#include "EPModelFixture.hpp"
#include "../Loop/AirLoopHVAC.hpp"
#include "../Loop/PlantLoop.hpp"
#include "../Schedule/ScheduleConstant.hpp"
#include "../Schedule/ScheduleConstant_Impl.hpp"
#include "../Splitter/AirLoopHVACZoneSplitter.hpp"
#include "../StraightComponent/CoilHeatingElectricMultiStage.hpp"
#include "../StraightComponent/Node.hpp"

#include <utilities/idd/Coil_Heating_Electric_MultiStage_FieldEnums.hxx>

using namespace openstudio::epmodel;

TEST_F(EPModelFixture, CoilHeatingElectricMultiStage_DefaultConstructor) {
  Model model;
  CoilHeatingElectricMultiStage coil(model);
  EXPECT_EQ(CoilHeatingElectricMultiStage::iddObjectType(), coil.iddObject().type());
  EXPECT_FALSE(coil.nameString().empty());
  auto availability = coil.availabilitySchedule().optionalCast<ScheduleConstant>();
  ASSERT_TRUE(availability);
  EXPECT_DOUBLE_EQ(1.0, availability->value());
  EXPECT_EQ(1u, coil.numberOfStages());
}

TEST_F(EPModelFixture, CoilHeatingElectricMultiStage_ScalarAccessors_RoundTrip) {
  Model model;
  CoilHeatingElectricMultiStage coil(model);
  ScheduleConstant availability(model);
  ASSERT_TRUE(availability.setValue(0.3));

  EXPECT_TRUE(coil.setAvailabilitySchedule(availability));
  EXPECT_EQ(availability.handle(), coil.availabilitySchedule().handle());

  EXPECT_TRUE(coil.setUnsigned(openstudio::Coil_Heating_Electric_MultiStageFields::NumberofStages, 3u));
  EXPECT_EQ(3u, coil.numberOfStages());

  EXPECT_TRUE(coil.setName("HP Electric MultiStage Coil"));
  EXPECT_EQ("HP Electric MultiStage Coil", coil.nameString());
}

TEST_F(EPModelFixture, CoilHeatingElectricMultiStage_AddToNodeSupplyOnly) {
  Model model;
  AirLoopHVAC airLoop(model);
  CoilHeatingElectricMultiStage supplyCoil(model);
  CoilHeatingElectricMultiStage demandCoil(model);

  auto supplyInletNode = airLoop.supplyInletNode();
  EXPECT_FALSE(supplyCoil.addToNode(supplyInletNode));
  EXPECT_FALSE(supplyCoil.inletModelObject());
  EXPECT_FALSE(supplyCoil.outletModelObject());

  auto demandInletNode = airLoop.demandInletNode();
  EXPECT_FALSE(demandCoil.addToNode(demandInletNode));
  EXPECT_FALSE(demandCoil.airLoopHVAC());
}
