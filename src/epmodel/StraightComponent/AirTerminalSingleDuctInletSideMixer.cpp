/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Energy Innovation, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "StraightComponent/AirTerminalSingleDuctInletSideMixer.hpp"
#include "StraightComponent/AirTerminalSingleDuctInletSideMixer_Impl.hpp"

#include "HVACComponent/ThermalZone.hpp"
#include "HVACComponent/ThermalZone_Impl.hpp"
#include "ModelObject/ZoneHVACAirDistributionUnit.hpp"
#include "ModelObject/ZoneHVACAirDistributionUnit_Impl.hpp"
#include "ModelObject/ZoneHVACEquipmentConnections.hpp"
#include "ModelObject/ZoneHVACEquipmentConnections_Impl.hpp"
#include "ModelObject/ZoneHVACEquipmentList.hpp"
#include "ModelObject/ZoneHVACEquipmentList_Impl.hpp"
#include "Model.hpp"
#include "Model_Impl.hpp"
#include "ModelObject.hpp"
#include "Node.hpp"
#include "Node_Impl.hpp"
#include "Loop/AirLoopHVAC.hpp"
#include "Loop/AirLoopHVAC_Impl.hpp"
#include "Mixer/AirLoopHVACZoneMixer.hpp"
#include "Mixer/AirLoopHVACZoneMixer_Impl.hpp"
#include "Splitter/AirLoopHVACZoneSplitter.hpp"
#include "Splitter/AirLoopHVACZoneSplitter_Impl.hpp"

#include <utilities/core/Assert.hpp>
#include <utilities/core/Logger.hpp>
#include <utilities/core/StringHelpers.hpp>
#include <algorithm>
#include <utilities/idd/AirTerminal_SingleDuct_Mixer_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>
#include <utilities/idd/IddFactory.hxx>
#include <utilities/idd/IddObject.hpp>
#include <utilities/idd/OS_AirTerminal_SingleDuct_InletSideMixer_FieldEnums.hxx>
#include <utilities/idd/ZoneHVAC_EquipmentConnections_FieldEnums.hxx>

namespace openstudio {
namespace epmodel {

  namespace {

    bool getBooleanFieldValue(const detail::ModelObject_Impl& impl, int fieldIndex) {
      const auto value = impl.getString(fieldIndex, true);
      OS_ASSERT(value);
      return openstudio::istringEqual(*value, "Yes");
    }

    bool setBooleanFieldValue(detail::ModelObject_Impl& impl, int fieldIndex, bool value) {
      return impl.setString(fieldIndex, value ? "Yes" : "No", false);
    }

    void applyConstructorDefaults(AirTerminalSingleDuctInletSideMixer& terminal) {
      OS_ASSERT(terminal.setControlForOutdoorAir(true));
      OS_ASSERT(terminal.setPerPersonVentilationRateMode("CurrentOccupancy"));
    }

    boost::optional<ThermalZone> owningThermalZoneForBranchNode(const Model& model, const Node& node) {
      for (const auto& zone : model.getConcreteModelObjects<ThermalZone>()) {
        if (zone.zoneAirNode() == node) {
          return zone;
        }
      }
      return boost::none;
    }

    bool registerTerminalWithThermalZone(const ModelObject& terminal, ThermalZone& thermalZone) {
      auto zoneImpl = thermalZone.getImpl<detail::ThermalZone_Impl>();
      OS_ASSERT(zoneImpl);

      auto zoneConnections = zoneImpl->getZoneHVACEquipmentConnections();
      auto equipmentList = zoneImpl->zoneHVACEquipmentList();
      if (!equipmentList) {
        ZoneHVACEquipmentList newEquipmentList(thermalZone.model());
        if (!newEquipmentList.name()) {
          newEquipmentList.createName();
        }
        if (!zoneConnections.setPointer(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneConditioningEquipmentListName, newEquipmentList.handle())) {
          return false;
        }
        equipmentList = newEquipmentList;
      }

      auto equipmentListImpl = equipmentList->getImpl<detail::ZoneHVACEquipmentList_Impl>();
      OS_ASSERT(equipmentListImpl);
      return equipmentListImpl->addEquipment(terminal);
    }

    boost::optional<ZoneHVACAirDistributionUnit> zoneHVACAirDistributionUnitForTerminal(const ModelObject& terminal) {
      for (const auto& source : terminal.getSources(openstudio::IddObjectType::ZoneHVAC_AirDistributionUnit)) {
        if (auto adu = source.optionalCast<ZoneHVACAirDistributionUnit>()) {
          return adu;
        }
      }
      return boost::none;
    }

  }  // namespace

  AirTerminalSingleDuctInletSideMixer::AirTerminalSingleDuctInletSideMixer(const Model& model)
    : StraightComponent(AirTerminalSingleDuctInletSideMixer::iddObjectType(), model) {
    auto impl = getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>();
    OS_ASSERT(impl);
    detail::LoadContext context{const_cast<Model&>(model), SanitizationPolicy::Repair, SanitizationReport{}, {}};  // NOLINT
    impl->canonicalize(context);
    applyConstructorDefaults(*this);
  }

  AirTerminalSingleDuctInletSideMixer::AirTerminalSingleDuctInletSideMixer(std::shared_ptr<detail::AirTerminalSingleDuctInletSideMixer_Impl> impl)
    : StraightComponent(std::move(impl)) {}

  IddObjectType AirTerminalSingleDuctInletSideMixer::iddObjectType() {
    return IddObjectType::AirTerminal_SingleDuct_Mixer;
  }

    std::vector<std::string> AirTerminalSingleDuctInletSideMixer::perPersonVentilationRateModeValues() {
      return getIddKeyNames(IddFactory::instance().getObject(iddObjectType()).get(),
                          openstudio::AirTerminal_SingleDuct_MixerFields::PerPersonVentilationRateMode);
  }

  bool AirTerminalSingleDuctInletSideMixer::addToNode(Node& node) {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->addToNode(node);
  }

  std::string AirTerminalSingleDuctInletSideMixer::perPersonVentilationRateMode() const {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->perPersonVentilationRateMode();
  }

  bool AirTerminalSingleDuctInletSideMixer::setPerPersonVentilationRateMode(const std::string& perPersonVentilationRateMode) {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->setPerPersonVentilationRateMode(perPersonVentilationRateMode);
  }

  bool AirTerminalSingleDuctInletSideMixer::controlForOutdoorAir() const {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->controlForOutdoorAir();
  }

  bool AirTerminalSingleDuctInletSideMixer::setControlForOutdoorAir(bool controlForOutdoorAir) {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->setControlForOutdoorAir(controlForOutdoorAir);
  }

  unsigned AirTerminalSingleDuctInletSideMixer::secondaryAirInletPort() const {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->secondaryAirInletPort();
  }

  boost::optional<Node> AirTerminalSingleDuctInletSideMixer::secondaryAirInletNode() const {
    return getImpl<detail::AirTerminalSingleDuctInletSideMixer_Impl>()->secondaryAirInletNode();
  }

}  // namespace epmodel
}  // namespace openstudio

namespace openstudio {
namespace epmodel {
  namespace detail {

    bool AirTerminalSingleDuctInletSideMixer_Impl::addToNode(Node& node) {
      if (node.model() != model()) {
        LOG_FREE(Warn, "openstudio.epmodel.AirTerminalSingleDuctInletSideMixer",
                 "addToNode requires a node in the same model as the inlet-side mixer terminal.");
        return false;
      }

      auto airLoop = node.airLoopHVAC();
      if (!airLoop) {
        LOG_FREE(Warn, "openstudio.epmodel.AirTerminalSingleDuctInletSideMixer",
                 "addToNode requires a node that resolves to an AirLoopHVAC context.");
        return false;
      }

      auto zoneSplitter = airLoop->zoneSplitter();
      auto zoneMixer = airLoop->zoneMixer();
      const auto thisNode = node.cast<ModelObject>();
      const auto splitterOutlets = zoneSplitter.outletModelObjects();
      const auto splitterIt = std::find(splitterOutlets.begin(), splitterOutlets.end(), thisNode);
      if (splitterIt == splitterOutlets.end()) {
        LOG_FREE(Warn, "openstudio.epmodel.AirTerminalSingleDuctInletSideMixer",
                 "addToNode requires the drop node to be a ZoneSplitter outlet node for the target AirLoopHVAC.");
        return false;
      }
      const auto splitterBranchIndex = static_cast<unsigned>(std::distance(splitterOutlets.begin(), splitterIt));

      if (!zoneMixer.inletModelObject(splitterBranchIndex)) {
        LOG_FREE(Warn, "openstudio.epmodel.AirTerminalSingleDuctInletSideMixer",
                 "addToNode requires a corresponding ZoneMixer inlet for ZoneSplitter branch index " << splitterBranchIndex << ".");
        return false;
      }

      auto thisObject = getObject<ModelObject>();
      if (!thisObject.name()) {
        thisObject.createName();
        if (!thisObject.name()) {
          return false;
        }
      }

      const std::string inletNodeName = node.nameString() + " - " + thisObject.nameString() + " Inlet Node";
      auto inletNode = model().getOrCreateTransientByName<Node>(inletNodeName);
      if (!zoneSplitter.setOutletModelObject(splitterBranchIndex, inletNode.cast<ModelObject>())) {
        return false;
      }

      if (!setPointer(inletPort(), inletNode.handle(), false)) {
        return false;
      }

      if (!setPointer(outletPort(), node.handle(), false)) {
        return false;
      }

      if (auto adu = zoneHVACAirDistributionUnitForTerminal(thisObject)) {
        adu->getImpl<detail::ZoneHVACAirDistributionUnit_Impl>()->setOutletNode(node);
      }

      if (auto thermalZone = owningThermalZoneForBranchNode(model(), node)) {
        if (!registerTerminalWithThermalZone(thisObject, *thermalZone)) {
          LOG_FREE(Warn, "openstudio.epmodel.AirTerminalSingleDuctInletSideMixer",
                   "addToNode failed to register the inlet-side mixer terminal with the owning thermal zone.");
          return false;
        }
      }

      return true;
    }

    std::string AirTerminalSingleDuctInletSideMixer_Impl::perPersonVentilationRateMode() const {
      const auto value = getString(openstudio::AirTerminal_SingleDuct_MixerFields::PerPersonVentilationRateMode, true);
      OS_ASSERT(value);
      return *value;
    }

    bool AirTerminalSingleDuctInletSideMixer_Impl::setPerPersonVentilationRateMode(const std::string& perPersonVentilationRateMode) {
      return setString(openstudio::AirTerminal_SingleDuct_MixerFields::PerPersonVentilationRateMode, perPersonVentilationRateMode);
    }

    bool AirTerminalSingleDuctInletSideMixer_Impl::controlForOutdoorAir() const {
      return getBooleanFieldValue(*this, openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::ControlForOutdoorAir);
    }

    bool AirTerminalSingleDuctInletSideMixer_Impl::setControlForOutdoorAir(bool controlForOutdoorAir) {
      return setBooleanFieldValue(*this, openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::ControlForOutdoorAir, controlForOutdoorAir);
    }

    std::vector<std::string> AirTerminalSingleDuctInletSideMixer_Impl::perPersonVentilationRateModeValues() const {
      return AirTerminalSingleDuctInletSideMixer::perPersonVentilationRateModeValues();
    }

    unsigned AirTerminalSingleDuctInletSideMixer_Impl::inletPort() const {
      return openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::TerminalUnitPrimaryAirInlet;
    }

    unsigned AirTerminalSingleDuctInletSideMixer_Impl::outletPort() const {
      return openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::TerminalUnitOutlet;
    }

    unsigned AirTerminalSingleDuctInletSideMixer_Impl::secondaryAirInletPort() const {
      return openstudio::OS_AirTerminal_SingleDuct_InletSideMixerFields::TerminalUnitSecondaryAirInlet;
    }

    boost::optional<Node> AirTerminalSingleDuctInletSideMixer_Impl::secondaryAirInletNode() const {
      return getObject<ModelObject>().getModelObjectTarget<Node>(secondaryAirInletPort());
    }

  }  // namespace detail
}  // namespace epmodel
}  // namespace openstudio
