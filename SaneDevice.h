#pragma once

#include <sane/sane.h>
#include <stdexcept>

namespace ZooScan
{
    class SaneException : public std::runtime_error
    {
    public:
        explicit SaneException(const std::string &arg)
                : runtime_error(arg)
        {
        }
    };

    class SaneHandle
    {
        friend class SaneDevice;

        SANE_Handle m_Handle;

        explicit SaneHandle(const SANE_Device *device)
        {
            SANE_Handle handle;
            SANE_Status status = sane_open(device->name, &handle);
            if (status != SANE_STATUS_GOOD)
            {
                m_Handle = nullptr;
                throw SaneException("Failed to open device.");
            }
            m_Handle = handle;
        }

    public:
        ~SaneHandle()
        {
            if (m_Handle != nullptr)
            {
                sane_close(m_Handle);
            }
        }

        SaneHandle(const SaneHandle &) = delete;

        SaneHandle &operator=(const SaneHandle &) = delete;

        SaneHandle(SaneHandle &&other) noexcept
                : m_Handle(other.m_Handle)
        {
            other.m_Handle = nullptr;
        }

        SaneHandle &operator=(SaneHandle &&other) noexcept
        {
            if (this != &other)
            {
                m_Handle = other.m_Handle;
                other.m_Handle = nullptr;
            }
            return *this;
        }

        [[nodiscard]] SANE_Handle Get() const
        {
            return m_Handle;
        }

        [[nodiscard]] const SANE_Option_Descriptor *GetOptionDescriptor(uint32_t optionIndex) const
        {
            return sane_get_option_descriptor(m_Handle, int(optionIndex));
        }

        void GetOption(uint32_t optionIndex, void *value) const
        {
            if (sane_control_option(m_Handle, int(optionIndex), SANE_ACTION_GET_VALUE, value, nullptr) !=
                SANE_STATUS_GOOD)
            {
                throw SaneException("Failed to get option.");
            }
        }

        void SetOption(uint32_t optionIndex, void *value, int *info) const
        {
            if (sane_control_option(m_Handle, int(optionIndex), SANE_ACTION_SET_VALUE, value, info) != SANE_STATUS_GOOD)
            {
                throw SaneException("Failed to set option.");
            }
        }

        void SetOptionToDefault(uint32_t optionIndex, int *info) const
        {
            if (sane_control_option(m_Handle, int(optionIndex), SANE_ACTION_SET_AUTO, nullptr, info) !=
                SANE_STATUS_GOOD)
            {
                throw SaneException("Failed to set option to default value.");
            }
        }
    };


    class SaneDevice
    {
        const SANE_Device *m_Device{};
        const SaneHandle *m_Handle{};

    public:
        explicit SaneDevice(const SANE_Device *device)
                : m_Device(device)
        {
        }

        ~SaneDevice()
        {
            Close();
        }

        void Open()
        {
            m_Handle = new SaneHandle(m_Device);
        }

        void Close()
        {
            delete m_Handle;
            m_Handle = nullptr;
        }

        [[nodiscard]] const SaneHandle *Handle() const
        {
            if (m_Handle == nullptr)
            {
                throw SaneException("Device is not open.");
            }
            return m_Handle;
        }

        [[nodiscard]] const SANE_Device *Get() const
        { return m_Device; }

        [[nodiscard]] const char *Name() const
        { return m_Device->name; }

        [[nodiscard]] const char *Vendor() const
        { return m_Device->vendor; }

        [[nodiscard]] const char *Model() const
        { return m_Device->model; }

        [[nodiscard]] const char *Type() const
        { return m_Device->type; }
    };

}
