#pragma once

#include <gtk/gtk.h>
#include <vector>

namespace ZooLib
{

    class View
    {
        static void DestroyView(GtkWidget *widget, gpointer data)
        {
            auto *view = static_cast<View *>(data);
            delete view;
        }

    protected:
        View() = default;

        void PostCreateView()
        {
            g_signal_connect(GetRootWidget(), "destroy", G_CALLBACK(DestroyView), this);
        }

    public:
        template<typename TView, typename... TArgs>
        static TView *Create(TArgs &&...args)
        {
            auto view = new TView(std::forward<TArgs>(args)...);
            view->PostCreateView();
            return view;
        }

        virtual ~View() = default;

        [[nodiscard]] virtual GtkWidget *GetRootWidget() const = 0;

        virtual void Update(const std::vector<uint64_t> &lastSeenVersions) = 0;
    };

}
