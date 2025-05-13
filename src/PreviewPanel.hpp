#pragma once

#include <gtk/gtk.h>

#include "PreviewState.hpp"
#include "Rect.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/View.hpp"

namespace Gorfector
{
    class App;

    class PreviewPanel : public ZooLib::View
    {
        constexpr static auto gdkModifiers = GDK_SHIFT_MASK | GDK_LOCK_MASK | GDK_CONTROL_MASK | GDK_ALT_MASK |
                                             GDK_SUPER_MASK | GDK_HYPER_MASK | GDK_META_MASK;

        enum class DragMode
        {
            None,
            Pan,
            Move,
            Draw,
            Top,
            Bottom,
            Left,
            Right,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight,
        };

        App *m_App{};
        ZooLib::CommandDispatcher m_Dispatcher{};

        PreviewState *m_PreviewState{};
        ViewUpdateObserver<PreviewPanel, PreviewState> *m_ViewUpdateObserver{};

        double m_ZoomFactor{};

        GtkWidget *m_RootWidget{};
        GtkWidget *m_ZoomDropDown{};
        GtkWidget *m_ProgressBar{};
        GdkPixbuf *m_ScannedImage{};
        // The whole preview image, including the checkerboard.
        GtkWidget *m_PreviewImage{};

        // The scanned data
        GdkPixbuf *m_PreviewPixBuf{};
        // The source data for m_PreviewPixBuf
        const unsigned char *m_UnderlyingBuffer{};
        // Auxiliary buffer for the converted image, if scanned data is not in 8bit RGBA format.
        unsigned char *m_ConvertedImage{};
        int m_ConvertedImageSize{};
        int m_LastLineConverted{-1};

        bool m_IsDragging{};
        double m_DragStartX{};
        double m_DragStartY{};
        DragMode m_DragMode{};
        Point<double> m_OriginalPan{};
        Rect<double> m_PixelScanArea{};
        Point<double> m_LastMousePosition{};

        PreviewPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app);

        void OnPreviewDragBegin(GtkGestureDrag *dragController);
        void OnPreviewDragUpdate(GtkGestureDrag *dragController);
        void OnPreviewDragEnd(GtkGestureDrag *dragController);

        void OnMouseMove(GtkEventControllerMotion *motionController, gdouble x, gdouble y);
        void OnMouseScroll(GtkEventControllerScroll *scrollController, gdouble deltaX, gdouble deltaY);

        void OnPreviewDraw(cairo_t *cr) const;

        void OnResized(GtkWidget *widget, void *data, void *);
        void OnZoomDropDownChanged(GtkDropDown *dropDown, void *data);

        void ComputeScanArea(double deltaX, double deltaY, Rect<double> &outScanArea) const;
        bool ScanAreaToPixels(const Rect<double> &scanArea, Rect<double> &outPixelArea) const;

        void FillWithEmptyPattern() const;
        void Redraw();

    public:
        /**
         * \brief Creates a new instance of a `PreviewPanel` class.
         *
         * This static method allocates and initializes a new `PreviewPanel` instance, ensuring that
         * the `PostCreateView` method is called to set up the destroy signal.
         *
         * \param parentDispatcher Pointer to the parent command dispatcher.
         * \param app Pointer to the application instance.
         * \return A pointer to the newly created `PreviewPanel` instance.
         */
        static PreviewPanel *Create(ZooLib::CommandDispatcher *parentDispatcher, App *app)
        {
            auto view = new PreviewPanel(parentDispatcher, app);
            view->PostCreateView();
            return view;
        }

        ~PreviewPanel() override;

        GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        PreviewState *GetState() const
        {
            return m_PreviewState;
        }

        void Update(const std::vector<uint64_t> &lastSeenVersions) override;
    };

}
