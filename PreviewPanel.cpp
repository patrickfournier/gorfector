#include "PreviewPanel.hpp"

#include "App.hpp"
#include "Commands/SetScanAreaCommand.hpp"
#include "ZooLib/SignalSupport.hpp"

enum class ScanAreaCursorRegions
{
    Outside,
    Inside,
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

ZooScan::PreviewPanel::PreviewPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app)
    : m_App(app)
    , m_Dispatcher(parentDispatcher)
{
    m_PreviewPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, k_PreviewWidth, k_PreviewHeight);
    m_PreviewImage = gtk_drawing_area_new();
    gtk_widget_set_size_request(m_PreviewImage, k_PreviewWidth, k_PreviewHeight);
    gtk_widget_add_css_class(m_PreviewImage, "preview-overlay");

    auto dragHandler = gtk_gesture_drag_new();
    gtk_widget_add_controller(m_PreviewImage, GTK_EVENT_CONTROLLER(dragHandler));
    auto mouseHandler = gtk_event_controller_motion_new();
    gtk_widget_add_controller(m_PreviewImage, GTK_EVENT_CONTROLLER(mouseHandler));
    gtk_drawing_area_set_draw_func(
            GTK_DRAWING_AREA(m_PreviewImage),
            [](GtkDrawingArea *widget, cairo_t *cr, int width, int height, gpointer data) {
                auto previewPanel = static_cast<PreviewPanel *>(data);
                previewPanel->OnPreviewDraw(widget, cr, width, height);
            },
            this, nullptr);

    ZooLib::ConnectGtkSignal(this, &PreviewPanel::OnPreviewDragBegin, dragHandler, "drag-begin");
    ZooLib::ConnectGtkSignal(this, &PreviewPanel::OnPreviewDragUpdate, dragHandler, "drag-update");
    ZooLib::ConnectGtkSignal(this, &PreviewPanel::OnPreviewDragEnd, dragHandler, "drag-end");
    ZooLib::ConnectGtkSignal(this, &PreviewPanel::OnMouseMove, mouseHandler, "motion");

    m_PreviewState = new PreviewState(m_App->GetState());
    m_ViewUpdateObserver = new ViewUpdateObserver(this, m_PreviewState);
    m_App->GetObserverManager()->AddObserver(m_ViewUpdateObserver);
}

ZooScan::PreviewPanel::~PreviewPanel()
{
    m_App->GetObserverManager()->RemoveObserver(m_ViewUpdateObserver);
    delete m_ViewUpdateObserver;
    delete m_PreviewState;
}

void ZooScan::PreviewPanel::ComputeScanArea(GtkWidget *widget, double w, double h, Rect<double> &outScanArea) const
{
    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        outScanArea = Rect<double>{0, 0, 0, 0};
        return;
    }

    // Assuming widget top-left is always (0, 0)
    const int widgetWidth = gtk_widget_get_width(widget);
    const int widgetHeight = gtk_widget_get_height(widget);

    // Assuming GetMaxScanArea() top-left is always (0, 0)
    auto maxScanArea = m_App->GetDeviceOptions()->GetMaxScanArea();
    double scaleW = maxScanArea.width / static_cast<double>(widgetWidth);
    double scaleH = maxScanArea.height / static_cast<double>(widgetHeight);

    switch (m_DragMode)
    {
        case ScanAreaDragMode::Top:
        {
            h = std::min(h, m_OriginalScanArea.height);
            h = std::max(h, -m_OriginalScanArea.y);

            outScanArea.x = m_OriginalScanArea.x * scaleW;
            outScanArea.y = (m_OriginalScanArea.y + h) * scaleH;
            outScanArea.width = m_OriginalScanArea.width * scaleW;
            outScanArea.height = (m_OriginalScanArea.height - h) * scaleH;
            break;
        }
        case ScanAreaDragMode::Bottom:
        {
            h = std::max(h, -m_OriginalScanArea.height);
            h = std::min(h, widgetHeight - m_OriginalScanArea.y - m_OriginalScanArea.height);

            outScanArea.x = m_OriginalScanArea.x * scaleW;
            outScanArea.y = m_OriginalScanArea.y * scaleH;
            outScanArea.width = m_OriginalScanArea.width * scaleW;
            outScanArea.height = (m_OriginalScanArea.height + h) * scaleH;
            break;
        }
        case ScanAreaDragMode::Left:
        {
            w = std::min(w, m_OriginalScanArea.width);
            w = std::max(w, -m_OriginalScanArea.x);

            outScanArea.x = (m_OriginalScanArea.x + w) * scaleW;
            outScanArea.y = m_OriginalScanArea.y * scaleH;
            outScanArea.width = (m_OriginalScanArea.width - w) * scaleW;
            outScanArea.height = m_OriginalScanArea.height * scaleH;
            break;
        }
        case ScanAreaDragMode::Right:
        {
            w = std::max(w, -m_OriginalScanArea.width);
            w = std::min(w, widgetWidth - m_OriginalScanArea.x - m_OriginalScanArea.width);

            outScanArea.x = m_OriginalScanArea.x * scaleW;
            outScanArea.y = m_OriginalScanArea.y * scaleH;
            outScanArea.width = (m_OriginalScanArea.width + w) * scaleW;
            outScanArea.height = m_OriginalScanArea.height * scaleH;
            break;
        }
        case ScanAreaDragMode::TopLeft:
        {
            w = std::min(w, m_OriginalScanArea.width);
            w = std::max(w, -m_OriginalScanArea.x);
            h = std::min(h, m_OriginalScanArea.height);
            h = std::max(h, -m_OriginalScanArea.y);

            outScanArea.x = (m_OriginalScanArea.x + w) * scaleW;
            outScanArea.y = (m_OriginalScanArea.y + h) * scaleH;
            outScanArea.width = (m_OriginalScanArea.width - w) * scaleW;
            outScanArea.height = (m_OriginalScanArea.height - h) * scaleH;
            break;
        }
        case ScanAreaDragMode::TopRight:
        {
            w = std::max(w, -m_OriginalScanArea.width);
            w = std::min(w, widgetWidth - m_OriginalScanArea.x - m_OriginalScanArea.width);
            h = std::min(h, m_OriginalScanArea.height);
            h = std::max(h, -m_OriginalScanArea.y);

            outScanArea.x = m_OriginalScanArea.x * scaleW;
            outScanArea.y = (m_OriginalScanArea.y + h) * scaleH;
            outScanArea.width = (m_OriginalScanArea.width + w) * scaleW;
            outScanArea.height = (m_OriginalScanArea.height - h) * scaleH;
            break;
        }
        case ScanAreaDragMode::BottomLeft:
        {
            w = std::min(w, m_OriginalScanArea.width);
            w = std::max(w, -m_OriginalScanArea.x);
            h = std::max(h, -m_OriginalScanArea.height);
            h = std::min(h, widgetHeight - m_OriginalScanArea.y - m_OriginalScanArea.height);

            outScanArea.x = (m_OriginalScanArea.x + w) * scaleW;
            outScanArea.y = m_OriginalScanArea.y * scaleH;
            outScanArea.width = (m_OriginalScanArea.width - w) * scaleW;
            outScanArea.height = (m_OriginalScanArea.height + h) * scaleH;
            break;
        }
        case ScanAreaDragMode::BottomRight:
        {
            w = std::max(w, -m_OriginalScanArea.width);
            w = std::min(w, widgetWidth - m_OriginalScanArea.x - m_OriginalScanArea.width);
            h = std::max(h, -m_OriginalScanArea.height);
            h = std::min(h, widgetHeight - m_OriginalScanArea.y - m_OriginalScanArea.height);

            outScanArea.x = m_OriginalScanArea.x * scaleW;
            outScanArea.y = m_OriginalScanArea.y * scaleH;
            outScanArea.width = (m_OriginalScanArea.width + w) * scaleW;
            outScanArea.height = (m_OriginalScanArea.height + h) * scaleH;
            break;
        }
        case ScanAreaDragMode::Move:
        {
            w = std::max(w, -m_OriginalScanArea.x);
            w = std::min(w, widgetWidth - m_OriginalScanArea.x - m_OriginalScanArea.width);
            h = std::max(h, -m_OriginalScanArea.y);
            h = std::min(h, widgetHeight - m_OriginalScanArea.y - m_OriginalScanArea.height);

            outScanArea.x = (m_OriginalScanArea.x + w) * scaleW;
            outScanArea.y = (m_OriginalScanArea.y + h) * scaleH;
            outScanArea.width = m_OriginalScanArea.width * scaleW;
            outScanArea.height = m_OriginalScanArea.height * scaleH;
            break;
        }
        case ScanAreaDragMode::Rect:
        default:
        {
            w = std::min(w, widgetWidth - m_SelectionStartX);
            w = std::max(w, -m_SelectionStartX);
            h = std::min(h, widgetHeight - m_SelectionStartY);
            h = std::max(h, -m_SelectionStartY);

            outScanArea.x = std::min(m_SelectionStartX, m_SelectionStartX + w) * scaleW;
            outScanArea.y = std::min(m_SelectionStartY, m_SelectionStartY + h) * scaleH;
            outScanArea.width = std::max(m_SelectionStartX, m_SelectionStartX + w) * scaleW - outScanArea.x;
            outScanArea.height = std::max(m_SelectionStartY, m_SelectionStartY + h) * scaleH - outScanArea.y;
            break;
        }
    }

    // Normalize rectangle
    if (outScanArea.width < 0)
    {
        outScanArea.x += outScanArea.width;
        outScanArea.width = -outScanArea.width;
    }
    if (outScanArea.height < 0)
    {
        outScanArea.y += outScanArea.height;
        outScanArea.height = -outScanArea.height;
    }

    // Apply maxScanArea limits.
    if (outScanArea.x + outScanArea.width > maxScanArea.x + maxScanArea.width)
    {
        outScanArea.width = maxScanArea.x + maxScanArea.width - outScanArea.x;
    }
    if (outScanArea.y + outScanArea.height > maxScanArea.y + maxScanArea.height)
    {
        outScanArea.height = maxScanArea.y + maxScanArea.height - outScanArea.y;
    }
    if (outScanArea.x < maxScanArea.x)
    {
        outScanArea.x = maxScanArea.x;
    }
    if (outScanArea.y < maxScanArea.y)
    {
        outScanArea.y = maxScanArea.y;
    }
}

void ZooScan::PreviewPanel::ScanAreaToPixels(
        GtkWidget *widget, const Rect<double> &scanArea, Rect<double> &outPixelArea) const
{
    int widgetWidth = gtk_widget_get_width(widget);
    int widgetHeight = gtk_widget_get_height(widget);

    double scaleW;
    double scaleH;

    if (m_App != nullptr && m_App->GetDeviceOptions() != nullptr)
    {
        // Assuming GetMaxScanArea() top-left is always (0, 0)
        auto maxScanArea = m_App->GetDeviceOptions()->GetMaxScanArea();
        scaleW = static_cast<double>(widgetWidth) / maxScanArea.width;
        scaleH = static_cast<double>(widgetHeight) / maxScanArea.height;
    }
    else
    {
        scaleW = 1.0;
        scaleH = 1.0;
    }

    outPixelArea.x = scanArea.x * scaleW;
    outPixelArea.y = scanArea.y * scaleH;
    outPixelArea.width = scanArea.width * scaleW;
    outPixelArea.height = scanArea.height * scaleH;
}

ScanAreaCursorRegions GetScanAreaCursorRegion(const ZooScan::Rect<double> &pixelArea, double x, double y)
{
    static constexpr double tolerance = 3.0;

    if (x > pixelArea.x + tolerance && x < pixelArea.x + pixelArea.width - tolerance)
    {
        if (y > pixelArea.y + tolerance && y < pixelArea.y + pixelArea.height - tolerance)
        {
            return ScanAreaCursorRegions::Inside;
        }
    }

    if (x >= pixelArea.x - tolerance && x <= pixelArea.x + tolerance)
    {
        if (y >= pixelArea.y - tolerance && y <= pixelArea.y + tolerance)
        {
            return ScanAreaCursorRegions::TopLeft;
        }
        if (y >= pixelArea.y + pixelArea.height - tolerance && y <= pixelArea.y + pixelArea.height + tolerance)
        {
            return ScanAreaCursorRegions::BottomLeft;
        }
        if (y > pixelArea.y + tolerance && y < pixelArea.y + pixelArea.height - tolerance)
        {
            return ScanAreaCursorRegions::Left;
        }
    }

    if (x >= pixelArea.x + pixelArea.width - tolerance && x <= pixelArea.x + pixelArea.width + tolerance)
    {
        if (y >= pixelArea.y - tolerance && y <= pixelArea.y + tolerance)
        {
            return ScanAreaCursorRegions::TopRight;
        }
        if (y >= pixelArea.y + pixelArea.height - tolerance && y <= pixelArea.y + pixelArea.height + tolerance)
        {
            return ScanAreaCursorRegions::BottomRight;
        }
        if (y > pixelArea.y + tolerance && y < pixelArea.y + pixelArea.height - tolerance)
        {
            return ScanAreaCursorRegions::Right;
        }
    }

    if (y >= pixelArea.y - tolerance && y <= pixelArea.y + tolerance)
    {
        if (x > pixelArea.x + tolerance && x < pixelArea.x + pixelArea.width - tolerance)
        {
            return ScanAreaCursorRegions::Top;
        }
    }

    if (y >= pixelArea.y + pixelArea.height - tolerance && y <= pixelArea.y + pixelArea.height + tolerance)
    {
        if (x > pixelArea.x + tolerance && x < pixelArea.x + pixelArea.width - tolerance)
        {
            return ScanAreaCursorRegions::Bottom;
        }
    }

    return ScanAreaCursorRegions::Outside;
}

void ZooScan::PreviewPanel::OnPreviewDragBegin(GtkGestureDrag *dragController)
{
    m_IsDragging = true;

    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(dragController));

    double w, h;
    gtk_gesture_drag_get_start_point(dragController, &m_SelectionStartX, &m_SelectionStartY);

    if (auto deviceOptions = m_App->GetDeviceOptions(); deviceOptions != nullptr)
    {
        ScanAreaToPixels(widget, deviceOptions->GetScanArea(), m_OriginalScanArea);

        auto cursorRegion = GetScanAreaCursorRegion(m_OriginalScanArea, m_SelectionStartX, m_SelectionStartY);
        auto modifiers = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(dragController));

        switch (cursorRegion)
        {
            case ScanAreaCursorRegions::Top:
                m_DragMode = ScanAreaDragMode::Top;
                break;
            case ScanAreaCursorRegions::Bottom:
                m_DragMode = ScanAreaDragMode::Bottom;
                break;
            case ScanAreaCursorRegions::Left:
                m_DragMode = ScanAreaDragMode::Left;
                break;
            case ScanAreaCursorRegions::Right:
                m_DragMode = ScanAreaDragMode::Right;
                break;
            case ScanAreaCursorRegions::TopLeft:
                m_DragMode = ScanAreaDragMode::TopLeft;
                break;
            case ScanAreaCursorRegions::TopRight:
                m_DragMode = ScanAreaDragMode::TopRight;
                break;
            case ScanAreaCursorRegions::BottomLeft:
                m_DragMode = ScanAreaDragMode::BottomLeft;
                break;
            case ScanAreaCursorRegions::BottomRight:
                m_DragMode = ScanAreaDragMode::BottomRight;
                break;
            case ScanAreaCursorRegions::Inside:
                if (modifiers & GDK_ALT_MASK)
                {
                    m_DragMode = ScanAreaDragMode::Move;
                }
                else
                {
                    m_DragMode = ScanAreaDragMode::Rect;
                }
                break;
            default:
                m_DragMode = ScanAreaDragMode::Rect;
                break;
        }
    }
    else
    {
        m_OriginalScanArea = Rect<double>{0, 0, 0, 0};
        m_DragMode = ScanAreaDragMode::Rect;
    }

    gtk_gesture_drag_get_offset(dragController, &w, &h);

    Rect<double> scanArea;
    ComputeScanArea(widget, w, h, scanArea);
    m_Dispatcher.Dispatch(SetScanAreaCommand(scanArea));

    gtk_widget_queue_draw(widget);
}

void ZooScan::PreviewPanel::OnPreviewDragUpdate(GtkGestureDrag *dragController)
{
    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(dragController));

    double w, h;
    gtk_gesture_drag_get_offset(dragController, &w, &h);

    Rect<double> scanArea;
    ComputeScanArea(widget, w, h, scanArea);
    m_Dispatcher.Dispatch(SetScanAreaCommand(scanArea));

    gtk_widget_queue_draw(widget);
}

void ZooScan::PreviewPanel::OnPreviewDragEnd(GtkGestureDrag *dragController)
{
    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(dragController));

    double w, h;
    gtk_gesture_drag_get_offset(dragController, &w, &h);

    Rect<double> scanArea;
    ComputeScanArea(widget, w, h, scanArea);
    m_Dispatcher.Dispatch(SetScanAreaCommand(scanArea));

    gtk_widget_queue_draw(widget);

    m_IsDragging = false;
}

void ZooScan::PreviewPanel::OnMouseMove(GtkEventControllerMotion *motionController, gdouble x, gdouble y)
{
    if (m_IsDragging)
    {
        return;
    }

    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        return;
    }

    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(motionController));
    auto modifiers = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(motionController));

    Rect<double> scanArea = m_App->GetDeviceOptions()->GetScanArea();
    Rect<double> pixelArea;
    ScanAreaToPixels(GTK_WIDGET(widget), scanArea, pixelArea);

    switch (GetScanAreaCursorRegion(pixelArea, x, y))
    {
        case ScanAreaCursorRegions::Top:
            gtk_widget_set_cursor_from_name(widget, "n-resize");
            break;
        case ScanAreaCursorRegions::Bottom:
            gtk_widget_set_cursor_from_name(widget, "s-resize");
            break;
        case ScanAreaCursorRegions::Left:
            gtk_widget_set_cursor_from_name(widget, "w-resize");
            break;
        case ScanAreaCursorRegions::Right:
            gtk_widget_set_cursor_from_name(widget, "e-resize");
            break;
        case ScanAreaCursorRegions::TopLeft:
            gtk_widget_set_cursor_from_name(widget, "nw-resize");
            break;
        case ScanAreaCursorRegions::TopRight:
            gtk_widget_set_cursor_from_name(widget, "ne-resize");
            break;
        case ScanAreaCursorRegions::BottomLeft:
            gtk_widget_set_cursor_from_name(widget, "sw-resize");
            break;
        case ScanAreaCursorRegions::BottomRight:
            gtk_widget_set_cursor_from_name(widget, "se-resize");
            break;
        case ScanAreaCursorRegions::Inside:
            if (modifiers & GDK_ALT_MASK)
            {
                gtk_widget_set_cursor_from_name(widget, "move");
            }
            else
            {
                gtk_widget_set_cursor_from_name(widget, "default");
            }
            break;
        default:
            gtk_widget_set_cursor_from_name(widget, "default");
            break;
    }
}

void ZooScan::PreviewPanel::OnPreviewDraw(GtkDrawingArea *widget, cairo_t *cr, int width, int height) const
{
    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        return;
    }

    Rect<double> scanArea = m_App->GetDeviceOptions()->GetScanArea();
    Rect<double> pixelArea;
    ScanAreaToPixels(GTK_WIDGET(widget), scanArea, pixelArea);

    gdk_cairo_set_source_pixbuf(cr, m_PreviewPixBuf, 0, 0);
    cairo_paint(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_EXCLUSION);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, pixelArea.x, pixelArea.y, pixelArea.width, pixelArea.height);
    cairo_stroke(cr);
}

void ZooScan::PreviewPanel::Update(u_int64_t lastSeenVersion) const
{
    if (const auto image = m_PreviewState->GetFullImage(); image != nullptr)
    {
        auto width = m_PreviewState->GetPixelsPerLine();
        auto height = m_PreviewState->GetImageHeight();
        auto scaleW = static_cast<double>(k_PreviewWidth) / width;
        auto scaleH = static_cast<double>(k_PreviewHeight) / height;
        auto scale = std::min(scaleW, scaleH);
        auto previewWidth = k_PreviewWidth * scale / scaleW;
        auto previewHeight = k_PreviewHeight * scale / scaleH;

        auto *pixbuf = gdk_pixbuf_new_from_data(
                m_PreviewState->GetFullImage(), GDK_COLORSPACE_RGB, false, 8, width, height,
                m_PreviewState->GetBytesPerLine(), nullptr, nullptr);
        gdk_pixbuf_fill(m_PreviewPixBuf, 0);
        gdk_pixbuf_scale(
                pixbuf, m_PreviewPixBuf, 0, 0, static_cast<int>(previewWidth), static_cast<int>(previewHeight), 0., 0.,
                scale, scale, GDK_INTERP_NEAREST);
        g_object_unref(pixbuf);
    }

    gtk_widget_queue_draw(m_PreviewImage);
}
