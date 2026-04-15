/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#ifndef EPMODEL_AIRTERMINALSINGLEDUCTINLETSIDEMIXER_HPP
#define EPMODEL_AIRTERMINALSINGLEDUCTINLETSIDEMIXER_HPP

#include "EPModelAPI.hpp"
#include "StraightComponent/StraightComponent.hpp"

#include <utilities/idd/IddEnums.hxx>

#include <memory>
#include <vector>

namespace openstudio {
namespace epmodel {

  class Model;
  class Node;

  namespace detail {
    class AirTerminalSingleDuctInletSideMixer_Impl;
  }

  class EPMODEL_API AirTerminalSingleDuctInletSideMixer : public StraightComponent
  {
   public:
    explicit AirTerminalSingleDuctInletSideMixer(const Model& model);

    virtual ~AirTerminalSingleDuctInletSideMixer() override = default;
    AirTerminalSingleDuctInletSideMixer(const AirTerminalSingleDuctInletSideMixer& other) = default;
    AirTerminalSingleDuctInletSideMixer(AirTerminalSingleDuctInletSideMixer&& other) = default;
    AirTerminalSingleDuctInletSideMixer& operator=(const AirTerminalSingleDuctInletSideMixer&) = default;
    AirTerminalSingleDuctInletSideMixer& operator=(AirTerminalSingleDuctInletSideMixer&&) = default;

    static IddObjectType iddObjectType();

    static std::vector<std::string> perPersonVentilationRateModeValues();
    bool addToNode(Node& node);

    // Schema Alignment Notes:
    // - Status: Partial Parity. The scalar inlet-side mixer controls are aligned, and the current epmodel zone-branch insertion path is exposed,
    //   but the broader canonical local-topology surface remains intentionally narrower.
    // - Canonical Counterpart: openstudio::model::AirTerminalSingleDuctInletSideMixer.
    // - Implemented Parity: `perPersonVentilationRateMode`, `controlForOutdoorAir`, `secondaryAirInletNode`, and `addToNode` preserve the
    //   canonical scalar/relationship contract on the current epmodel zone-branch path.
    // - Documented Delta: The wrapper preserves the epmodel-only default constructor, and the broader canonical local-topology / translator-side
    //   node relationships are still not surfaced as public epmodel helpers.
    // - Field/Storage Mapping: The preserved scalar and direct object links map directly to the EnergyPlus `AirTerminal:SingleDuct:Mixer` fields.
    // - Evidence: `src/model/AirTerminalSingleDuctInletSideMixer.hpp`, `src/model/AirTerminalSingleDuctInletSideMixer.cpp`, `src/energyplus/ForwardTranslator/ForwardTranslateAirTerminalSingleDuctInletSideMixer.cpp`, and `src/epmodel/test/AirTerminalSingleDuctInletSideMixer_GTest.cpp`.
    // - Remaining Parity Work: Broaden the local-topology surface only if canonical insertion behavior needs to be mirrored more fully.

    /** @name Per Person Ventilation Rate Mode */
    //@{
    std::string perPersonVentilationRateMode() const;
    bool setPerPersonVentilationRateMode(const std::string& perPersonVentilationRateMode);
    //@}

    /** @name Outdoor Air Control */
    //@{
    bool controlForOutdoorAir() const;
    bool setControlForOutdoorAir(bool controlForOutdoorAir);
    //@}

    /** @name Secondary Air */
    //@{
    unsigned secondaryAirInletPort() const;
    boost::optional<Node> secondaryAirInletNode() const;
    //@}

   protected:
    using ImplType = detail::AirTerminalSingleDuctInletSideMixer_Impl;

    friend class Model;
    friend class openstudio::IdfObject;
    friend class openstudio::detail::IdfObject_Impl;

    explicit AirTerminalSingleDuctInletSideMixer(std::shared_ptr<detail::AirTerminalSingleDuctInletSideMixer_Impl> impl);
  };

}  // namespace epmodel
}  // namespace openstudio

#endif
