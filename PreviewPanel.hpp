#pragma once

#include <gtk/gtk.h>

#include "PreviewState.hpp"
#include "Rect.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/CommandDispatcher.hpp"

namespace ZooScan
{
    class App;

    class PreviewPanel
    {
    private:
        enum class ScanAreaDragMode
        {
            Move,
            Rect,
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
        ViewUpdateObserver<PreviewPanel, PreviewState> *m_ViewUpdateObserver;

        double m_ZoomFactor{};

        GtkWidget *m_RootWidget{};
        GtkWidget *m_ZoomDropDown{};
        GdkPixbuf *m_ScannedImage{};
        GdkPixbuf *m_PreviewPixBuf{};
        GtkWidget *m_PreviewImage{};

        bool m_IsDragging{};
        double m_DragStartX{};
        double m_DragStartY{};
        Rect<double> m_OriginalScanArea;
        ScanAreaDragMode m_DragMode{};

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

        void FillWithEmptyPattern();
        void Redraw();

    public:
        PreviewPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app);
        ~PreviewPanel();

        GtkWidget *GetRootWidget() const
        {
            return m_RootWidget;
        }

        PreviewState *GetState() const
        {
            return m_PreviewState;
        }

        void Update(u_int64_t lastSeenVersion);
    };

}
