#pragma once

#include <cstring>
#include <nlohmann/json.hpp>
#include <utility>

#include "DeviceOptionValue.hpp"
#include "DeviceOptionValueBase.hpp"
#include "DeviceSelectorState.hpp"
#include "Rect.hpp"
#include "ZooLib/ChangesetBase.hpp"
#include "ZooLib/ChangesetManager.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    class SaneDevice;

    enum class ScanAreaUnit
    {
        e_Pixels,
        e_Millimeters
    };

    union WidgetIndex
    {
        const uint32_t OptionValueIndices[2]; /*
                                               * First index is the option index,
                                               * second index is the value index within the option.
                                               */
        const uint64_t CompositeIndex;
    };

    /**
     * \brief A changeset class for managing changes in device options state.
     */
    class DeviceOptionsStateChangeset : public ZooLib::ChangesetBase
    {
        std::vector<WidgetIndex> m_ChangedIndices;
        bool m_ReloadOptions{};

    public:
        explicit DeviceOptionsStateChangeset(uint64_t stateInitialVersion)
            : ChangesetBase(stateInitialVersion)
        {
        }

        /**
         * \brief Adds a changed index to the changeset.
         *
         * This method records an index that has been modified in the device options state.
         *
         * \param index The `WidgetIndex` representing the changed index.
         */
        void AddChangedIndex(WidgetIndex index)
        {
            m_ChangedIndices.push_back(index);
        }

        /**
         * \brief Retrieves the list of changed indices.
         *
         * This method returns a constant reference to the vector of changed indices
         * recorded in the changeset.
         *
         * \return A constant reference to the vector of changed indices.
         */
        [[nodiscard]] const std::vector<WidgetIndex> &GetChangedIndices() const
        {
            return m_ChangedIndices;
        }

        /**
         * \brief Sets the reload options flag, meaning that all options should be reloaded from the device.
         *
         * This method sets a flag indicating whether the options should be reloaded.
         *
         * \param reloadOptions A boolean value indicating whether to reload options.
         */
        void SetReloadOptions(bool reloadOptions)
        {
            m_ReloadOptions = reloadOptions;
        }

        /**
         * \brief Checks if all options should be rebuilt.
         *
         * This method returns the value of the reload options flag, which indicates
         * whether all options should be rebuilt.
         *
         * \return True if all options should be rebuilt, false otherwise.
         */
        [[nodiscard]] bool ShouldRebuildOptions() const
        {
            return m_ReloadOptions;
        }

        /**
         * \brief Aggregates changes from another changeset.
         *
         * This method combines the changes from the provided changeset into the current
         * changeset, ensuring no duplicate indices are added.
         *
         * \param changeset The `DeviceOptionsStateChangeset` to aggregate.
         */
        void Aggregate(const DeviceOptionsStateChangeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ReloadOptions |= changeset.m_ReloadOptions;

            for (auto changedIndex: changeset.m_ChangedIndices)
            {
                if (std::ranges::find_if(m_ChangedIndices, [changedIndex](const WidgetIndex &a) {
                        return a.CompositeIndex == changedIndex.CompositeIndex;
                    }) == m_ChangedIndices.end())
                {
                    m_ChangedIndices.push_back(changedIndex);
                }
            }
        }
    };

    /**
     * \class DeviceOptionsState
     * \brief Manages the state and options of a scanning device.
     *
     * This class represents the state of a scanning device, including its options and configurations.
     * It inherits from `ZooLib::StateComponent` and `ZooLib::ChangesetManager` to handle state changes
     * and manage changesets for tracking modifications.
     */
    class DeviceOptionsState : public ZooLib::StateComponent
    {
    public:
        static constexpr uint32_t k_InvalidIndex = std::numeric_limits<uint32_t>::max(); ///< Invalid index constant.
        static constexpr const char *k_DeviceKey = "Device"; ///< Key for the device in JSON serialization.
        static constexpr const char *k_DeviceNameKey = "Name"; ///< Key for the device name in JSON serialization.
        static constexpr const char *k_DeviceVendorKey = "Vendor"; ///< Key for the device vendor in JSON serialization.
        static constexpr const char *k_DeviceModelKey = "Model"; ///< Key for the device model in JSON serialization.
        static constexpr const char *k_DeviceTypeKey = "Type"; ///< Key for the device type in JSON serialization.
        static constexpr const char *k_OptionsKey = "Options"; ///< Key for the device options in JSON serialization.
        static constexpr const char *k_TlxKey = "tl-x"; ///< Key for the top-left x-coordinate option.
        static constexpr const char *k_TlyKey = "tl-y"; ///< Key for the top-left y-coordinate option.
        static constexpr const char *k_BrxKey = "br-x"; ///< Key for the bottom-right x-coordinate option.
        static constexpr const char *k_BryKey = "br-y"; ///< Key for the bottom-right y-coordinate option.

    private:
        const std::string m_DeviceName;
        std::vector<DeviceOptionValueBase *> m_OptionValues;

        uint32_t m_PreviewIndex;
        uint32_t m_ModeIndex;
        uint32_t m_TLXIndex;
        uint32_t m_TLYIndex;
        uint32_t m_BRXIndex;
        uint32_t m_BRYIndex;
        uint32_t m_ResolutionIndex;
        uint32_t m_XResolutionIndex;
        uint32_t m_YResolutionIndex;
        uint32_t m_BitDepthIndex;

        ZooLib::ChangesetManager<DeviceOptionsStateChangeset> m_ChangesetManager{};

        [[nodiscard]] DeviceOptionsStateChangeset *GetCurrentChangeset()
        {
            return m_ChangesetManager.GetCurrentChangeset(GetVersion());
        }

        [[nodiscard]] SaneDevice *GetDevice() const
        {
            if (m_DeviceName.empty() || m_State == nullptr)
            {
                return nullptr;
            }

            const auto deviceSelectorState = m_State->GetStateComponentByType<DeviceSelectorState>();
            if (deviceSelectorState == nullptr)
            {
                return nullptr;
            }
            return deviceSelectorState->GetDeviceByName(m_DeviceName);
        }

        void ReloadOptions() const;
        void BuildOptions();

        void AddOptionValue(uint32_t index, DeviceOptionValueBase *optionValue)
        {
            if (m_OptionValues.size() <= index)
            {
                m_OptionValues.resize(index + 1);
            }
            m_OptionValues[index] = optionValue;

            /* Save special indices */

            if (strcmp(optionValue->GetName(), "preview") == 0)
            {
                m_PreviewIndex = index;
            }
            else if (strcmp(optionValue->GetName(), "mode") == 0) // not a "well-known" option
            {
                m_ModeIndex = index;
            }
            else
            {
                if (strcmp(optionValue->GetName(), k_TlxKey) == 0)
                {
                    m_TLXIndex = index;
                }
                else if (strcmp(optionValue->GetName(), k_TlyKey) == 0)
                {
                    m_TLYIndex = index;
                }
                else if (strcmp(optionValue->GetName(), k_BrxKey) == 0)
                {
                    m_BRXIndex = index;
                }
                else if (strcmp(optionValue->GetName(), k_BryKey) == 0)
                {
                    m_BRYIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "resolution") == 0)
                {
                    m_ResolutionIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "x-resolution") == 0) // not a "well-known" option
                {
                    m_XResolutionIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "y-resolution") == 0) // not a "well-known" option
                {
                    m_YResolutionIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "depth") == 0) // not a "well-known" option
                {
                    m_BitDepthIndex = index;
                }
            }
        }

        void Clear()
        {
            for (auto optionValue: m_OptionValues)
            {
                delete optionValue;
            }
            m_OptionValues.clear();
        }

        friend void to_json(nlohmann::json &j, const DeviceOptionsState &p);

    public:
        DeviceOptionsState(ZooLib::State *state, std::string deviceName)
            : StateComponent(state)
            , m_DeviceName(std::move(deviceName))
            , m_PreviewIndex(k_InvalidIndex)
            , m_ModeIndex(k_InvalidIndex)
            , m_TLXIndex(k_InvalidIndex)
            , m_TLYIndex(k_InvalidIndex)
            , m_BRXIndex(k_InvalidIndex)
            , m_BRYIndex(k_InvalidIndex)
            , m_ResolutionIndex(k_InvalidIndex)
            , m_XResolutionIndex(k_InvalidIndex)
            , m_YResolutionIndex(k_InvalidIndex)
            , m_BitDepthIndex(k_InvalidIndex)
        {
            BuildOptions();
        }

        ~DeviceOptionsState() override
        {
            Clear();
        }

        [[nodiscard]] uint64_t FirstChangesetVersion() const
        {
            return m_ChangesetManager.FirstChangesetVersion();
        }

        [[nodiscard]] ZooLib::ChangesetManagerBase *GetChangesetManager() override
        {
            return &m_ChangesetManager;
        }

        [[nodiscard]] DeviceOptionsStateChangeset *GetAggregatedChangeset(uint64_t stateComponentVersion) const
        {
            return m_ChangesetManager.GetAggregatedChangeset(stateComponentVersion);
        }

        /**
         * \class Updater
         * \brief Provides functionality to update and manage the state of `DeviceOptionsState`.
         *
         * This nested class is responsible for applying changes to the `DeviceOptionsState` object.
         * It includes methods for loading settings from JSON, applying scan area configurations,
         * and managing option values. It also ensures that changes are pushed to the current changeset
         * upon destruction.
         */
        class Updater : public StateComponent::Updater<DeviceOptionsState>
        {
            bool ApplyRequestedValuesToDevice(const std::vector<size_t> &changedIndices);
            void FindRequestMismatches(const std::vector<size_t> &indicesToCheck, std::vector<size_t> &mismatches);
            void ApplyPreset(const nlohmann::json &json);

        public:
            /**
             * \brief Constructs an Updater for the `DeviceOptionsState`.
             *
             * \param state Pointer to the `DeviceOptionsState` to be updated.
             */
            explicit Updater(DeviceOptionsState *state)
                : StateComponent::Updater<DeviceOptionsState>(state)
            {
            }

            /**
             * \brief Destructor. Pushes the current changeset to the stack.
             */
            ~Updater() override
            {
                m_StateComponent->m_ChangesetManager.PushCurrentChangeset();
            }

            /**
             * \brief Loads settings from a JSON object into the `DeviceOptionsState`.
             *
             * \param json The JSON object containing the settings to load.
             */
            void LoadFromJson(const nlohmann::json &json) override;

            /**
             * \brief Applies settings from a JSON object to the `DeviceOptionsState`.
             *
             * \param json The JSON object containing the settings to apply.
             */
            void ApplySettings(const nlohmann::json &json)
            {
                LoadFromJson(json);
            }

            /**
             * \brief Applies a scan area configuration from a JSON object.
             *
             * \param json The JSON object containing the scan area configuration.
             */
            void ApplyScanArea(const nlohmann::json &json)
            {
                ApplyPreset(json);
            }

            /**
             * \brief Reloads the options of the `DeviceOptionsState` from the device.
             *
             * This method triggers a reload of the options and marks the changeset
             * to indicate that all options should be rebuilt.
             */
            void ReloadOptions() const
            {
                m_StateComponent->ReloadOptions();
                m_StateComponent->GetCurrentChangeset()->SetReloadOptions(true);
            }

            /**
             * \brief Sets the value of a boolean option in the `DeviceOptionsState`.
             *
             * This method sets the value of a boolean option at the specified index.
             *
             * \param optionIndex The index of the option to set.
             * \param valueIndex The index of the value to set for the option.
             * \param requestedValue The boolean value to set for the option.
             */
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, bool requestedValue) const;

            /**
             * \brief Sets the value of a double option in the `DeviceOptionsState`.
             *
             * This method sets the value of a double option at the specified index.
             *
             * \param optionIndex The index of the option to set.
             * \param valueIndex The index of the value to set for the option.
             * \param requestedValue The double value to set for the option.
             */
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, double requestedValue) const;

            /**
             * \brief Sets the value of an integer option in the `DeviceOptionsState`.
             *
             * This method sets the value of an integer option at the specified index.
             *
             * \param optionIndex The index of the option to set.
             * \param valueIndex The index of the value to set for the option.
             * \param requestedValue The integer value to set for the option.
             */
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, int requestedValue) const;

            /**
             * \brief Sets the value of a string option in the `DeviceOptionsState`.
             *
             * This method sets the value of a string option at the specified index.
             *
             * \param optionIndex The index of the option to set.
             * \param valueIndex The index of the value to set for the option.
             * \param requestedValue The string value to set for the option.
             */
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, const std::string &requestedValue) const;
        };

        /**
         * \brief Retrieves the model of the device.
         *
         * This method returns the model name of the device associated with the current state.
         * If no device is associated, it returns `nullptr`.
         *
         * \return A pointer to a C-string representing the device model, or `nullptr` if no device is found.
         */
        [[nodiscard]] const char *GetDeviceModel() const
        {
            auto saneDevice = GetDevice();
            if (saneDevice == nullptr)
            {
                return nullptr;
            }

            return saneDevice->GetModel();
        }

        /**
         * \brief Retrieves the vendor of the device.
         *
         * This method returns the vendor name of the device associated with the current state.
         * If no device is associated, it returns `nullptr`.
         *
         * \return A pointer to a C-string representing the device vendor, or `nullptr` if no device is found.
         */
        [[nodiscard]] const char *GetDeviceVendor() const
        {
            auto saneDevice = GetDevice();
            if (saneDevice == nullptr)
            {
                return nullptr;
            }

            return saneDevice->GetVendor();
        }

        /**
         * \brief Retrieves the total number of options available.
         *
         * This method returns the total count of options currently stored in the device's option list.
         *
         * \return The number of options available.
         */
        [[nodiscard]] size_t GetOptionCount() const
        {
            return m_OptionValues.size();
        }

        /**
         * \brief Retrieves the option value at the specified index.
         *
         * This method returns a pointer to the base class of the option value
         * at the given index in the device's option list.
         *
         * \param settingIndex The index of the option to retrieve.
         * \return A pointer to the `DeviceOptionValueBase` object, or `nullptr` if the index is invalid.
         */
        [[nodiscard]] const DeviceOptionValueBase *GetOption(uint32_t settingIndex) const
        {
            return m_OptionValues[settingIndex];
        }

        /**
         * \brief Retrieves the option value of a specific type at the specified index.
         *
         * This method returns a pointer to the option value of type `T` at the given index
         * in the device's option list. It uses `dynamic_cast` to ensure the type matches.
         *
         * \tparam T The type of the option value to retrieve.
         * \param settingIndex The index of the option to retrieve.
         * \return A pointer to the `DeviceOptionValue<T>` object, or `nullptr` if the type or index is invalid.
         */
        template<typename T>
        [[nodiscard]] const DeviceOptionValue<T> *GetOption(uint32_t settingIndex) const
        {
            return dynamic_cast<const DeviceOptionValue<T> *>(m_OptionValues[settingIndex]);
        }

        /**
         * \brief Checks if the preview mode is enabled.
         *
         * \return True if the preview mode is enabled, false otherwise.
         */
        [[nodiscard]] bool IsPreview() const;

        /**
         * \brief Retrieves the current mode (color, grayscale, black and white, ...) of the device.
         *
         * \return A string representing the current mode.
         */
        [[nodiscard]] std::string GetMode() const;

        /**
         * \brief Gets the unit of measurement for the scan area.
         *
         * \return The scan area unit, either `e_Pixels` or `e_Millimeters`.
         */
        [[nodiscard]] ScanAreaUnit GetScanAreaUnit() const;

        /**
         * \brief Retrieves the current scan area configuration.
         *
         * \return A rectangle representing the scan area.
         */
        [[nodiscard]] Rect<double> GetScanArea() const;

        /**
         * \brief Retrieves the maximum possible scan area configuration.
         *
         * \return A rectangle representing the maximum scan area.
         */
        [[nodiscard]] Rect<double> GetMaxScanArea() const;

        /**
         * \brief Gets the current resolution of the device.
         *
         * \return The resolution in DPI.
         */
        [[nodiscard]] int GetResolution() const;

        /**
         * \brief Gets the horizontal resolution of the device.
         *
         * \return The horizontal resolution in DPI.
         */
        [[nodiscard]] int GetXResolution() const;

        /**
         * \brief Gets the vertical resolution of the device.
         *
         * \return The vertical resolution in DPI.
         */
        [[nodiscard]] int GetYResolution() const;

        /**
         * \brief Retrieves the bit depth of the device.
         *
         * \return The bit depth, defaulting to 8 if not set.
         */
        [[nodiscard]] int GetBitDepth() const;

        /**
         * \brief Retrieves the index of the "preview" option.
         *
         * This method returns the index of the "preview" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "preview" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetPreviewIndex() const
        {
            return m_PreviewIndex;
        }

        /**
         * \brief Retrieves the index of the "mode" option.
         *
         * This method returns the index of the "mode" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "mode" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetModeIndex() const
        {
            return m_ModeIndex;
        }

        /**
         * \brief Retrieves the index of the "tl-x" (top-left x-coordinate) option.
         *
         * This method returns the index of the "tl-x" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "tl-x" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetTLXIndex() const
        {
            return m_TLXIndex;
        }

        /**
         * \brief Retrieves the index of the "tl-y" (top-left y-coordinate) option.
         *
         * This method returns the index of the "tl-y" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "tl-y" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetTLYIndex() const
        {
            return m_TLYIndex;
        }

        /**
         * \brief Retrieves the index of the "br-x" (bottom-right x-coordinate) option.
         *
         * This method returns the index of the "br-x" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "br-x" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetBRXIndex() const
        {
            return m_BRXIndex;
        }

        /**
         * \brief Retrieves the index of the "br-y" (bottom-right y-coordinate) option.
         *
         * This method returns the index of the "br-y" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "br-y" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetBRYIndex() const
        {
            return m_BRYIndex;
        }

        /**
         * \brief Retrieves the index of the "resolution" option.
         *
         * This method returns the index of the "resolution" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "resolution" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetResolutionIndex() const
        {
            return m_ResolutionIndex;
        }

        /**
         * \brief Retrieves the index of the "x-resolution" option.
         *
         * This method returns the index of the "x-resolution" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "x-resolution" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetXResolutionIndex() const
        {
            return m_XResolutionIndex;
        }

        /**
         * \brief Retrieves the index of the "y-resolution" option.
         *
         * This method returns the index of the "y-resolution" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "y-resolution" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetYResolutionIndex() const
        {
            return m_YResolutionIndex;
        }

        /**
         * \brief Retrieves the index of the "depth" (bit depth) option.
         *
         * This method returns the index of the "depth" option in the device's option list.
         * The index is used to access the corresponding option value.
         *
         * \return The index of the "depth" option, or `k_InvalidIndex` if not set.
         */
        [[nodiscard]] uint32_t GetBitDepthIndex() const
        {
            return m_BitDepthIndex;
        }
    };

    /**
     * \brief Serializes the `DeviceOptionsState` object to a JSON representation.
     *
     * This method converts the state of the `DeviceOptionsState` object into a JSON object.
     * It includes the device's name, vendor, model, type, and all software-settable options.
     *
     * \param j The JSON object to populate with the serialized data.
     * \param p The `DeviceOptionsState` object to serialize.
     */
    inline void to_json(nlohmann::json &j, const DeviceOptionsState &p)
    {
        auto device = p.GetDevice();
        if (device == nullptr)
        {
            return;
        }

        j = nlohmann::json::object();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceNameKey] = device->GetName();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceVendorKey] = device->GetVendor();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceModelKey] = device->GetModel();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceTypeKey] = device->GetType();

        j[DeviceOptionsState::k_OptionsKey] = nlohmann::json::object();
        for (auto optionValue: p.m_OptionValues)
        {
            if (optionValue == nullptr)
            {
                continue;
            }
            if (!optionValue->IsSoftwareSettable())
            {
                continue;
            }

            optionValue->Serialize(j[DeviceOptionsState::k_OptionsKey]);
        }
    }
}
