#pragma once

#include <functional>
#include <gtk/gtk.h>

namespace ZooLib
{
    template<typename... TArgs>
    void InvokeGtkCallback(TArgs... args, gpointer data)
    {
        auto *cb = static_cast<std::function<void(TArgs...)> *>(data);
        (*cb)(args...);
    }

    template<typename... TArgs>
    void InvokeGtkCallbackWithParamSpecs(TArgs... args, GParamSpec *, gpointer data)
    {
        auto *cb = static_cast<std::function<void(TArgs...)> *>(data);
        (*cb)(args...);
    }

    template<typename TClass, typename... TArgs, typename TGtkType>
    gulong ConnectGtkSignal(TClass *obj, void (TClass::*method)(TArgs...), TGtkType widget, const char *signalName)
    {
        auto *cb = new std::function<void(TArgs...)>([obj, method](TArgs... args) { (obj->*method)(args...); });
        return g_signal_connect(widget, signalName, G_CALLBACK(InvokeGtkCallback<TArgs...>), cb);
    }

    template<typename TClass, typename... TArgs, typename TGtkType>
    gulong ConnectGtkSignalAfter(TClass *obj, void (TClass::*method)(TArgs...), TGtkType widget, const char *signalName)
    {
        auto *cb = new std::function<void(TArgs...)>([obj, method](TArgs... args) { (obj->*method)(args...); });
        return g_signal_connect_after(widget, signalName, G_CALLBACK(InvokeGtkCallback<TArgs...>), cb);
    }

    template<typename TClass, typename... TArgs, typename TGtkType>
    gulong ConnectGtkSignalWithParamSpecs(
            TClass *obj, void (TClass::*method)(TArgs...), TGtkType widget, const char *signalName)
    {
        auto *cb = new std::function<void(TArgs...)>([obj, method](TArgs... args) { (obj->*method)(args...); });
        return g_signal_connect(widget, signalName, G_CALLBACK(InvokeGtkCallbackWithParamSpecs<TArgs...>), cb);
    }
}
