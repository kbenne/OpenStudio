/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#ifndef EPMODEL_COILSYSTEMCOOLINGWATERHEATEXCHANGERASSISTED_HPP
#define EPMODEL_COILSYSTEMCOOLINGWATERHEATEXCHANGERASSISTED_HPP

#include "EPModelAPI.hpp"
#include "StraightComponent/StraightComponent.hpp"

#include <memory>
#include <vector>

namespace openstudio {
namespace epmodel {

  class Model;
  class Node;
  class AirToAirComponent;
  class WaterToAirComponent;

  namespace detail {
    class CoilSystemCoolingWaterHeatExchangerAssisted_Impl;
  }

  class EPMODEL_API CoilSystemCoolingWaterHeatExchangerAssisted : public StraightComponent
  {
   public:
    explicit CoilSystemCoolingWaterHeatExchangerAssisted(const Model& model);
    explicit CoilSystemCoolingWaterHeatExchangerAssisted(const Model& model, const AirToAirComponent& heatExchanger);

    virtual ~CoilSystemCoolingWaterHeatExchangerAssisted() override = default;
    CoilSystemCoolingWaterHeatExchangerAssisted(const CoilSystemCoolingWaterHeatExchangerAssisted& other) = default;
    CoilSystemCoolingWaterHeatExchangerAssisted(CoilSystemCoolingWaterHeatExchangerAssisted&& other) = default;
    CoilSystemCoolingWaterHeatExchangerAssisted& operator=(const CoilSystemCoolingWaterHeatExchangerAssisted&) = default;
    CoilSystemCoolingWaterHeatExchangerAssisted& operator=(CoilSystemCoolingWaterHeatExchangerAssisted&&) = default;

    static IddObjectType iddObjectType();

    static std::vector<std::string> heatExchangerObjectTypeValues();
    static std::vector<std::string> coolingCoilObjectTypeValues();

    // Schema Alignment Notes:
    // - Status: Partial Parity. The canonical child-object relationships and constructor defaults are now present, while broader model-owned
    //   topology conveniences remain out of scope.
    // - Canonical Counterpart: openstudio::model::CoilSystemCoolingWaterHeatExchangerAssisted.
    // - Implemented Parity: The default constructors, `heatExchanger`, `coolingCoil`, their relationship setters, and child traversal preserve
    //   the bounded canonical relationship slice for the assisted water coil-system wrapper.
    // - Documented Delta: Separate name helpers and broader model-owned topology conveniences from canonical
    //   `openstudio::model::CoilSystemCoolingWaterHeatExchangerAssisted` are not exposed yet.
    // - Field/Storage Mapping: Relationship targets map directly to the EnergyPlus heat-exchanger and cooling-coil name fields, while the
    //   object-type choices are synchronized from the linked targets' IDD object names.
    // - Evidence: `src/model/CoilSystemCoolingWaterHeatExchangerAssisted.hpp`, `src/energyplus/ForwardTranslator/ForwardTranslateCoilSystemCoolingWaterHeatExchangerAssisted.cpp`, and `src/epmodel/test/CoilSystemCoolingWaterHeatExchangerAssisted_GTest.cpp`.
    // - Remaining Parity Work: Add the omitted name conveniences and the missing standalone air-node topology support without changing the
    //   preserved relationship signatures.
    AirToAirComponent heatExchanger() const;
    bool setHeatExchanger(const AirToAirComponent& heatExchanger);

    WaterToAirComponent coolingCoil() const;
    bool setCoolingCoil(const WaterToAirComponent& coolingCoil);

    bool addToNode(Node& node);
    /** @name Heat exchanger object type */
    //@{
    std::string heatExchangerObjectType() const;
    bool setHeatExchangerObjectType(const std::string& heatExchangerObjectType);
    //@}

    /** @name Cooling coil object type */
    //@{
    std::string coolingCoilObjectType() const;
    bool setCoolingCoilObjectType(const std::string& coolingCoilObjectType);
    //@}

   protected:
    using ImplType = detail::CoilSystemCoolingWaterHeatExchangerAssisted_Impl;

    friend class Model;
    friend class openstudio::IdfObject;
    friend class openstudio::detail::IdfObject_Impl;

    explicit CoilSystemCoolingWaterHeatExchangerAssisted(std::shared_ptr<detail::CoilSystemCoolingWaterHeatExchangerAssisted_Impl> impl);
  };

}  // namespace epmodel
}  // namespace openstudio

#endif
