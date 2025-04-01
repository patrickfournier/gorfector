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
        const int k_PreviewWidth = 750;
        const int k_PreviewHeight = 1000;

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

        GdkPixbuf *m_PreviewPixBuf{};
        GtkWidget *m_PreviewImage{};

        bool m_IsDragging{};
        double m_SelectionStartX{};
        double m_SelectionStartY{};
        Rect<double> m_OriginalScanArea;
        ScanAreaDragMode m_DragMode{};

        void OnPreviewDragBegin(GtkGestureDrag *dragController);
        void OnPreviewDragUpdate(GtkGestureDrag *dragController);
        void OnPreviewDragEnd(GtkGestureDrag *dragController);

        void OnMouseMove(GtkEventControllerMotion *motionController, gdouble x, gdouble y);

        void OnPreviewDraw(GtkDrawingArea *widget, cairo_t *cr, int width, int height) const;

        void ComputeScanArea(GtkWidget *widget, double w, double h, Rect<double> &outScanArea) const;
        void ScanAreaToPixels(GtkWidget *widget, const Rect<double> &scanArea, Rect<double> &outScanArea) const;

    public:
        PreviewPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app);
        ~PreviewPanel();

        GtkWidget *GetRootWidget() const
        {
            return m_PreviewImage;
        }

        PreviewState *GetState() const
        {
            return m_PreviewState;
        }

        void Update(u_int64_t lastSeenVersion) const;
    };

}
