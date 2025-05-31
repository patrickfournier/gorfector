#pragma once

#include <cstring>
#include <glib.h>
#include <sane/sane.h>

namespace Gorfector
{
    class SaneDevice
    {
        const SANE_Device *m_Device{};
        SANE_Handle m_Handle{};

    public:
        explicit SaneDevice(const SANE_Device *device)
            : m_Device(device)
        {
        }

        ~SaneDevice()
        {
            Close();
        }

        SaneDevice(const SaneDevice &) = delete;

        SaneDevice &operator=(const SaneDevice &) = delete;

        SaneDevice(SaneDevice &&other) noexcept
            : m_Device(other.m_Device)
            , m_Handle(other.m_Handle)
        {
            other.m_Device = nullptr;
            other.m_Handle = nullptr;
        }

        SaneDevice &operator=(SaneDevice &&other) noexcept
        {
            if (this != &other)
            {
                m_Device = other.m_Device;
                m_Handle = other.m_Handle;
                other.m_Device = nullptr;
                other.m_Handle = nullptr;
            }
            return *this;
        }

        bool Open()
        {
            g_debug("Opening device %s", m_Device->name);

            SANE_Status status = sane_open(m_Device->name, &m_Handle);
            if (status != SANE_STATUS_GOOD)
            {
                g_debug("Failed to open device %s: %s", m_Device->name, sane_strstatus(status));
                m_Handle = nullptr;
                return false;
            }
            return true;
        }

        void Close()
        {
            if (m_Handle != nullptr)
            {
                g_debug("Closing device %s", m_Device->name);

                sane_cancel(m_Handle);
                sane_close(m_Handle);
            }
            m_Handle = nullptr;
        }

        [[nodiscard]] const char *GetName() const
        {
            return m_Device->name;
        }

        [[nodiscard]] const char *GetVendor() const
        {
            return m_Device->vendor;
        }

        [[nodiscard]] const char *GetModel() const
        {
            return m_Device->model;
        }

        [[nodiscard]] const char *GetType() const
        {
            return m_Device->type;
        }

        [[nodiscard]] const SANE_Option_Descriptor *GetOptionDescriptor(uint32_t optionIndex) const
        {
            if (m_Handle == nullptr)
            {
                return nullptr;
            }

            g_debug("Getting option descriptor for option %d", optionIndex);

            const SANE_Option_Descriptor *optionDescriptor =
                    sane_get_option_descriptor(m_Handle, static_cast<int>(optionIndex));
            return optionDescriptor;
        }

        bool GetOptionValue(uint32_t optionIndex, void *value) const
        {
            if (m_Handle == nullptr)
            {
                return false;
            }

            g_debug("Getting option %d value", optionIndex);

            auto status =
                    sane_control_option(m_Handle, static_cast<int>(optionIndex), SANE_ACTION_GET_VALUE, value, nullptr);
            if (status != SANE_STATUS_GOOD)
            {
                g_debug("Failed to get option value for option %d: %s", optionIndex, sane_strstatus(status));
                return false;
            }

            return true;
        }

        bool SetOptionValue(uint32_t optionIndex, void *value, int *optionInfo) const
        {
            if (m_Handle == nullptr)
            {
                return false;
            }

            int *intValue = static_cast<int *>(value);
            char *charValue = static_cast<char *>(value);
            g_debug("Setting option %d (%d) (%c%c%c%c)", optionIndex, *intValue, charValue[0], charValue[1],
                    charValue[2], charValue[3]);

            auto status = sane_control_option(
                    m_Handle, static_cast<int>(optionIndex), SANE_ACTION_SET_VALUE, value, optionInfo);
            if (status != SANE_STATUS_GOOD)
            {
                g_debug("Failed to set option value for option %d: %s", optionIndex, sane_strstatus(status));
                return false;
            }

            return true;
        }

        bool StartScan() const
        {
            if (m_Handle == nullptr)
            {
                return false;
            }

            g_debug("Starting scan for device %s", m_Device->name);

            auto status = sane_start(m_Handle);
            if (status != SANE_STATUS_GOOD)
            {
                g_debug("Failed to start scan: %s", sane_strstatus(status));
                return false;
            }

            return true;
        }

        bool Read(SANE_Byte *buffer, SANE_Int maxLength, SANE_Int *length) const
        {
            if (m_Handle == nullptr)
            {
                return false;
            }

            auto status = sane_read(m_Handle, buffer, maxLength, length);
            switch (status)
            {
                case SANE_STATUS_GOOD:
                case SANE_STATUS_DEVICE_BUSY:
                    return true;

                default:
                    g_debug("Failed to read from device %s: %s", m_Device->name, sane_strstatus(status));
                    return false;
            }
        }

        void CancelScan() const
        {
            if (m_Handle == nullptr)
            {
                return;
            }

            g_debug("Cancelling scan for device %s", m_Device->name);
            sane_cancel(m_Handle);
        }

        /**
         * Gets the parameters of the device. Must be called after StartScan().
         * \param parameters A pointer to the parameter structure to be filled with the device parameters.
         * \return True if the parameters were successfully retrieved, false otherwise.
         */
        bool GetParameters(SANE_Parameters *parameters) const
        {
            if (m_Handle == nullptr)
            {
                return false;
            }

            g_debug("Getting parameters for device %s", m_Device->name);

            auto status = sane_get_parameters(m_Handle, parameters);
            if (status != SANE_STATUS_GOOD)
            {
                g_debug("Failed to get parameters for device %s: %s", m_Device->name, sane_strstatus(status));
                return false;
            }

            return true;
        }

        [[nodiscard]] static bool IsDisplayOnly(const SANE_Option_Descriptor &optionDescriptor)
        {
            auto caps = optionDescriptor.cap;
            return (caps & SANE_CAP_SOFT_DETECT) && !(caps & SANE_CAP_SOFT_SELECT) && !(caps & SANE_CAP_HARD_SELECT);
        }

        [[nodiscard]] static bool ShouldHide(const SANE_Option_Descriptor &optionDescriptor)
        {
            auto caps = optionDescriptor.cap;
            bool notSettable = (caps & SANE_CAP_INACTIVE) || (caps & SANE_CAP_HARD_SELECT);

            if (notSettable)
                return true;

            bool hideBecauseOfTitle =
                    optionDescriptor.title != nullptr && (strcasestr(optionDescriptor.title, "deprecat") != nullptr ||
                                                          strcasestr(optionDescriptor.title, "preview") != nullptr);

            if (hideBecauseOfTitle)
                return true;

            bool hideBecauseOfDesc =
                    optionDescriptor.desc != nullptr && (strcasestr(optionDescriptor.desc, "deprecat") != nullptr ||
                                                         strcasestr(optionDescriptor.desc, "preview") != nullptr);

            return hideBecauseOfDesc;
        }

        [[nodiscard]] static bool IsSoftwareSettable(const SANE_Option_Descriptor &optionDescriptor)
        {
            return (optionDescriptor.cap & SANE_CAP_SOFT_SELECT);
        }

        [[nodiscard]] static bool IsAdvanced(const SANE_Option_Descriptor &optionDescriptor)
        {
            return (optionDescriptor.cap & SANE_CAP_ADVANCED);
        }
    };
}
