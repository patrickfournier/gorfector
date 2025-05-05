#pragma once

#include <functional>
#include <gtk/gtk.h>

namespace ZooLib
{
    /**
     * \brief Invokes a GTK callback function with the provided arguments.
     *
     * This function is used to call a callback function stored as a `std::function` with the given arguments.
     *
     * \tparam TArgs The types of the arguments to pass to the callback.
     * \param args The arguments to pass to the callback function.
     * \param data A pointer to the `std::function` object.
     */
    template<typename... TArgs>
    void InvokeGtkCallback(TArgs... args, gpointer data)
    {
        auto *cb = static_cast<std::function<void(TArgs...)> *>(data);
        (*cb)(args...);
    }

    /**
     * \brief Invokes a GTK callback function with the provided arguments and parameter specifications.
     *
     * This function is used to call a callback function stored as a `std::function` with the given arguments,
     * while ignoring the `GParamSpec` parameter.
     *
     * \tparam TArgs The types of the arguments to pass to the callback.
     * \param args The arguments to pass to the callback function.
     * \param paramSpec A pointer to the `GParamSpec` object (ignored).
     * \param data A pointer to the `std::function` object.
     */
    template<typename... TArgs>
    void InvokeGtkCallbackWithParamSpecs(TArgs... args, GParamSpec *paramSpec, gpointer data)
    {
        auto *cb = static_cast<std::function<void(TArgs...)> *>(data);
        (*cb)(args...);
    }

    /**
     * \brief Connects a GTK signal to a member function of a class.
     *
     * This function connects a GTK signal to a member function of a class, allowing the member function
     * to be called when the signal is emitted.
     *
     * \tparam TClass The type of the class containing the member function.
     * \tparam TArgs The types of the arguments expected by the member function.
     * \tparam TGtkType The type of the GTK widget emitting the signal.
     * \param obj A pointer to the class instance.
     * \param method A pointer to the member function to connect.
     * \param widget The GTK widget emitting the signal.
     * \param signalName The name of the GTK signal to connect.
     * \return The signal handler ID.
     */
    template<typename TClass, typename... TArgs, typename TGtkType>
    gulong ConnectGtkSignal(TClass *obj, void (TClass::*method)(TArgs...), TGtkType widget, const char *signalName)
    {
        auto *cb = new std::function<void(TArgs...)>([obj, method](TArgs... args) { (obj->*method)(args...); });
        return g_signal_connect(widget, signalName, G_CALLBACK(InvokeGtkCallback<TArgs...>), cb);
    }

    /**
     * \brief Connects a GTK signal to a member function of a class, ensuring the callback is executed after default
     * handlers.
     *
     * This function connects a GTK signal to a member function of a class, ensuring the member function
     * is called after the default signal handlers.
     *
     * \tparam TClass The type of the class containing the member function.
     * \tparam TArgs The types of the arguments expected by the member function.
     * \tparam TGtkType The type of the GTK widget emitting the signal.
     * \param obj A pointer to the class instance.
     * \param method A pointer to the member function to connect.
     * \param widget The GTK widget emitting the signal.
     * \param signalName The name of the GTK signal to connect.
     * \return The signal handler ID.
     */
    template<typename TClass, typename... TArgs, typename TGtkType>
    gulong ConnectGtkSignalAfter(TClass *obj, void (TClass::*method)(TArgs...), TGtkType widget, const char *signalName)
    {
        auto *cb = new std::function<void(TArgs...)>([obj, method](TArgs... args) { (obj->*method)(args...); });
        return g_signal_connect_after(widget, signalName, G_CALLBACK(InvokeGtkCallback<TArgs...>), cb);
    }

    /**
     * \brief Connects a GTK signal to a member function of a class, handling parameter specifications.
     *
     * This function connects a GTK signal to a member function of a class, allowing the member function
     * to be called when the signal is emitted. It also handles parameter specifications (`GParamSpec`).
     *
     * \tparam TClass The type of the class containing the member function.
     * \tparam TArgs The types of the arguments expected by the member function.
     * \tparam TGtkType The type of the GTK widget emitting the signal.
     * \param obj A pointer to the class instance.
     * \param method A pointer to the member function to connect.
     * \param widget The GTK widget emitting the signal.
     * \param signalName The name of the GTK signal to connect.
     * \return The signal handler ID.
     */
    template<typename TClass, typename... TArgs, typename TGtkType>
    gulong ConnectGtkSignalWithParamSpecs(
            TClass *obj, void (TClass::*method)(TArgs...), TGtkType widget, const char *signalName)
    {
        auto *cb = new std::function<void(TArgs...)>([obj, method](TArgs... args) { (obj->*method)(args...); });
        return g_signal_connect(widget, signalName, G_CALLBACK(InvokeGtkCallbackWithParamSpecs<TArgs...>), cb);
    }
}
