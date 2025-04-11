#pragma once

#include <gtk/gtk.h>

#include "PreviewState.hpp"
#include "Rect.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/View.hpp"

namespace ZooScan
{
    class App;

    class PreviewPanel : public ZooLib::View
    {
    private:
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
        GdkPixbuf *m_PreviewPixBuf{};
        GtkWidget *m_PreviewImage{};

        bool m_IsDragging{};
        double m_DragStartX{};
        double m_DragStartY{};
        DragMode m_DragMode{};
        Point<double> m_OriginalPan{};
        Rect<double> m_OriginalScanArea{};
        Point<double> m_LastMousePosition{};

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
        PreviewPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app);
        ~PreviewPanel() override;

        GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        PreviewState *GetState() const
        {
            return m_PreviewState;
        }

        void Update(uint64_t lastSeenVersion) override;
    };

}
