/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>

#include "../Curve/CurveQuadratic.hpp"
#include "EPModelFixture.hpp"
#include "../Loop/AirLoopHVAC.hpp"
#include "../Loop/PlantLoop.hpp"
#include "../Splitter/AirLoopHVACZoneSplitter.hpp"
#include "../Schedule/ScheduleConstant.hpp"
#include "../Schedule/ScheduleConstant_Impl.hpp"
#include "../StraightComponent/CoilHeatingGasMultiStage.hpp"
#include "../StraightComponent/Node.hpp"

#include <utilities/idd/Coil_Heating_Gas_MultiStage_FieldEnums.hxx>

using namespace openstudio::epmodel;

TEST_F(EPModelFixture, CoilHeatingGasMultiStage_DefaultConstructor) {
  Model model;
  CoilHeatingGasMultiStage coil(model);
  EXPECT_EQ(CoilHeatingGasMultiStage::iddObjectType(), coil.iddObject().type());
  EXPECT_FALSE(coil.nameString().empty());

  auto availability = coil.availabilitySchedule().optionalCast<ScheduleConstant>();
  ASSERT_TRUE(availability);
  EXPECT_DOUBLE_EQ(1.0, availability->value());
  EXPECT_EQ(1u, coil.numberOfStages());
  EXPECT_FALSE(coil.offCycleParasiticGasLoad());
  EXPECT_FALSE(coil.parasiticGasLoad());
  EXPECT_FALSE(coil.partLoadFractionCorrelationCurve());
}

TEST_F(EPModelFixture, CoilHeatingGasMultiStage_ScalarAccessors_RoundTrip) {
  Model model;
  CoilHeatingGasMultiStage coil(model);
  ScheduleConstant availability(model);
  ASSERT_TRUE(availability.setValue(0.3));
  CurveQuadratic plf(model);
  ASSERT_TRUE(plf.setCoefficient1Constant(0.7));
  ASSERT_TRUE(plf.setCoefficient2x(0.3));
  ASSERT_TRUE(plf.setCoefficient3xPOW2(0.0));

  EXPECT_TRUE(coil.setAvailabilitySchedule(availability));
  EXPECT_EQ(availability.handle(), coil.availabilitySchedule().handle());

  EXPECT_TRUE(coil.setPartLoadFractionCorrelationCurve(plf));
  ASSERT_TRUE(coil.partLoadFractionCorrelationCurve());
  EXPECT_EQ(plf.handle(), coil.partLoadFractionCorrelationCurve()->handle());
  const auto children = coil.children();
  ASSERT_EQ(1u, children.size());
  EXPECT_EQ(plf.handle(), children[0].handle());

  EXPECT_TRUE(coil.setUnsigned(openstudio::Coil_Heating_Gas_MultiStageFields::NumberofStages, 3u));
  EXPECT_EQ(3u, coil.numberOfStages());

  EXPECT_TRUE(coil.setOffCycleParasiticGasLoad(12.5));
  ASSERT_TRUE(coil.offCycleParasiticGasLoad());
  EXPECT_DOUBLE_EQ(12.5, *coil.offCycleParasiticGasLoad());
  ASSERT_TRUE(coil.parasiticGasLoad());
  EXPECT_DOUBLE_EQ(12.5, *coil.parasiticGasLoad());

  EXPECT_TRUE(coil.setParasiticGasLoad(9.75));
  ASSERT_TRUE(coil.offCycleParasiticGasLoad());
  EXPECT_DOUBLE_EQ(9.75, *coil.offCycleParasiticGasLoad());

  coil.resetParasiticGasLoad();
  EXPECT_FALSE(coil.offCycleParasiticGasLoad());

  EXPECT_TRUE(coil.setOffCycleParasiticGasLoad(3.2));
  coil.resetOffCycleParasiticGasLoad();
  EXPECT_FALSE(coil.offCycleParasiticGasLoad());

  coil.resetPartLoadFractionCorrelationCurve();
  EXPECT_FALSE(coil.partLoadFractionCorrelationCurve());
  EXPECT_TRUE(coil.children().empty());
}

TEST_F(EPModelFixture, CoilHeatingGasMultiStage_AddToNodeSupplyOnly) {
  Model model;
  AirLoopHVAC airLoop(model);
  CoilHeatingGasMultiStage supplyCoil(model);
  CoilHeatingGasMultiStage demandCoil(model);

  auto supplyInletNode = airLoop.supplyInletNode();
  EXPECT_FALSE(supplyCoil.addToNode(supplyInletNode));
  EXPECT_FALSE(supplyCoil.inletModelObject());
  EXPECT_FALSE(supplyCoil.outletModelObject());

  auto demandInletNode = airLoop.demandInletNode();
  EXPECT_FALSE(demandCoil.addToNode(demandInletNode));
  EXPECT_FALSE(demandCoil.airLoopHVAC());
}
