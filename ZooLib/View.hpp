#pragma once

#include <gtk/gtk.h>
#include <vector>

namespace ZooLib
{
    /**
     * \class View
     * \brief Base class for creating and managing UI views.
     *
     * The `View` class provides a foundation for creating graphical user interface (GUI) components
     * using GTK. It includes functionality for managing the lifecycle of views and updating their state.
     */
    class View
    {
        /**
         * \brief Callback function to destroy the view when the associated GTK widget is destroyed.
         * \param widget The GTK widget being destroyed.
         * \param data Pointer to the `View` instance.
         */
        static void DestroyView(GtkWidget *widget, gpointer data)
        {
            auto *view = static_cast<View *>(data);
            delete view;
        }

    protected:
        /**
         * \brief Protected default constructor to prevent direct instantiation.
         */
        View() = default;

        /**
         * \brief Connects the GTK widget's destroy signal to the `DestroyView` callback.
         *
         * This method should be called after the GTK widget is created to ensure proper cleanup.
         */
        void PostCreateView()
        {
            g_signal_connect(GetRootWidget(), "destroy", G_CALLBACK(DestroyView), this);
        }

    public:
        /**
         * \brief Creates a new instance of a derived `View` class.
         *
         * This static method allocates and initializes a new `View` instance, ensuring that
         * the `PostCreateView` method is called to set up the destroy signal.
         *
         * \tparam TView The type of the derived `View` class.
         * \tparam TArgs The types of the arguments to pass to the constructor.
         * \param args The arguments to pass to the constructor of the derived class.
         * \return A pointer to the newly created `View` instance.
         */
        template<typename TView, typename... TArgs>
        static TView *Create(TArgs &&...args)
        {
            auto view = new TView(std::forward<TArgs>(args)...);
            view->PostCreateView();
            return view;
        }

        /**
         * \brief Virtual destructor to allow proper cleanup of derived classes.
         */
        virtual ~View() = default;

        /**
         * \brief Retrieves the root GTK widget associated with the view.
         * \return A pointer to the root `GtkWidget`.
         */
        [[nodiscard]] virtual GtkWidget *GetRootWidget() const = 0;

        /**
         * \brief Updates the view based on the provided state versions.
         *
         * This method is called by an `Observer` to refresh the view's state, typically in response to changes
         * in the application state.
         *
         * \param lastSeenVersions A vector of version numbers representing the last seen state versions by the calling
         * `Observer`.
         */
        virtual void Update(const std::vector<uint64_t> &lastSeenVersions) = 0;
    };
}
