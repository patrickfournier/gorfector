#pragma once

#include "DeviceSelectorState.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/Application.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/View.hpp"

namespace Gorfector
{
    /**
     * \class DeviceSelector
     * \brief A class responsible for managing the device selection interface.
     *
     * This class extends the ZooLib::View and provides functionality to display
     * and manage a list of devices, handle user interactions, and update the view
     * accordingly. It interacts with the application and command dispatcher to
     * perform its operations.
     */
    class DeviceSelector : public ZooLib::View
    {
        DeviceSelectorState *m_State; ///< Pointer to the state of the device selector.
        ViewUpdateObserver<DeviceSelector, DeviceSelectorState> *m_Observer; ///< Observer for view updates.

        ZooLib::Application *m_App; ///< Pointer to the application instance.
        ZooLib::CommandDispatcher m_Dispatcher{}; ///< Command dispatcher for handling commands.

        GtkWidget *m_DeviceSelectorRoot{}; ///< Root widget for the device selector UI.
        GtkWidget *m_DeviceSelectorList{}; ///< Widget for the list of devices.

        gulong m_DropdownSelectedSignalId; ///< Signal ID for dropdown selection events.

        /**
         * \brief Constructor for the DeviceSelector class.
         * \param parent Pointer to the parent command dispatcher.
         * \param app Pointer to the application instance.
         * \param deviceSelectorState Pointer to the device selector state.
         */
        DeviceSelector(
                ZooLib::CommandDispatcher *parent, ZooLib::Application *app, DeviceSelectorState *deviceSelectorState);

        /**
         * \brief Callback for the "Refresh Devices" button click event.
         * \param widget The GTK widget triggering the event.
         */
        void OnRefreshDevicesClicked(GtkWidget *widget);

        /**
         * \brief Callback for the device selection event.
         * \param widget The GTK widget triggering the event.
         */
        void OnDeviceSelected(GtkWidget *widget);

        /**
         * \brief Callback for the "Activate Network" button click event.
         * \param widget The GTK widget triggering the event.
         */
        void OnActivateNetwork(GtkWidget *widget);

        /**
         * \brief Selects a device by its index.
         * \param deviceIndex The index of the device to select.
         */
        void SelectDevice(int deviceIndex);

    public:
        /**
         * \brief Creates a new instance of a `DeviceSelector` class.
         *
         * This static method allocates and initializes a new `DeviceSelector` instance, ensuring that
         * the `PostCreateView` method is called to set up the destroy signal.
         *
         * \param parent Pointer to the parent command dispatcher.
         * \param app Pointer to the application instance.
         * \param deviceSelectorState Pointer to the device selector state.
         * \return A pointer to the newly created `DeviceSelector` instance.
         */
        static DeviceSelector *
        Create(ZooLib::CommandDispatcher *parent, ZooLib::Application *app, DeviceSelectorState *deviceSelectorState)
        {
            auto view = new DeviceSelector(parent, app, deviceSelectorState);
            view->PostCreateView();
            return view;
        }

        /**
         * \brief Destructor for the DeviceSelector class.
         */
        ~DeviceSelector() override;

        /**
         * \brief Retrieves the current state of the device selector.
         * \return Pointer to the current DeviceSelectorState.
         */
        DeviceSelectorState *GetState() const
        {
            return m_State;
        }

        /**
         * \brief Retrieves the root GTK widget of the device selector.
         * \return Pointer to the root GTK widget.
         */
        GtkWidget *GetRootWidget() const override
        {
            return m_DeviceSelectorRoot;
        }

        /**
         * \brief Updates the device selector view based on the provided version information.
         * \param lastSeenVersion A vector of version identifiers for the devices.
         */
        void Update(const std::vector<uint64_t> &lastSeenVersion) override;

        /**
         * \brief Selects the default device (index 0).
         */
        void SelectDefaultDevice()
        {
            SelectDevice(0);
        }
    };
}
