#include "PreviewPanel.hpp"
#include "App.hpp"
#include "Commands/SetPanCommand.hpp"
#include "Commands/SetScanAreaCommand.hpp"
#include "Commands/SetZoomCommand.hpp"
#include "ZooLib/Gettext.hpp"
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

Gorfector::PreviewPanel::PreviewPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app)
    : m_App(app)
    , m_Dispatcher(parentDispatcher)
{
    m_RootWidget = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(m_RootWidget), 0);
    gtk_grid_set_row_spacing(GTK_GRID(m_RootWidget), 5);

    auto box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_grid_attach(GTK_GRID(m_RootWidget), box, 0, 0, 1, 1);

    m_PreviewPixBuf = nullptr;
    m_PreviewImage = gtk_drawing_area_new();
    gtk_widget_add_css_class(GTK_WIDGET(m_PreviewImage), "preview-panel");
    gtk_widget_set_hexpand(m_PreviewImage, true);
    gtk_widget_set_vexpand(m_PreviewImage, true);
    gtk_box_append(GTK_BOX(box), m_PreviewImage);

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(m_RootWidget), box, 0, 1, 1, 1);
    auto label = gtk_label_new(_("Zoom:"));
    gtk_box_append(GTK_BOX(box), label);

    m_ZoomDropDown = gtk_drop_down_new_from_strings(PreviewState::k_ZoomValueStrings);
    gtk_widget_set_size_request(m_ZoomDropDown, 100, -1);
    gtk_box_append(GTK_BOX(box), m_ZoomDropDown);

    m_ProgressBar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(m_ProgressBar), true);
    gtk_widget_set_visible(m_ProgressBar, false);
    gtk_widget_set_hexpand(m_ProgressBar, true);
    gtk_box_append(GTK_BOX(box), m_ProgressBar);

    auto dragHandler = gtk_gesture_drag_new();
    gtk_widget_add_controller(m_PreviewImage, GTK_EVENT_CONTROLLER(dragHandler));
    auto mouseHandler = gtk_event_controller_motion_new();
    gtk_widget_add_controller(m_PreviewImage, GTK_EVENT_CONTROLLER(mouseHandler));
    auto scrollHandler = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
    gtk_widget_add_controller(m_PreviewImage, GTK_EVENT_CONTROLLER(scrollHandler));
    gtk_drawing_area_set_draw_func(
            GTK_DRAWING_AREA(m_PreviewImage),
            [](GtkDrawingArea *widget, cairo_t *cr, int width, int height, gpointer data) {
                auto previewPanel = static_cast<PreviewPanel *>(data);
                previewPanel->OnPreviewDraw(cr);
            },
            this, nullptr);

    ConnectGtkSignal(this, &PreviewPanel::OnResized, m_PreviewImage, "resize");
    ConnectGtkSignal(this, &PreviewPanel::OnZoomDropDownChanged, m_ZoomDropDown, "notify::selected");
    ConnectGtkSignal(this, &PreviewPanel::OnPreviewDragBegin, dragHandler, "drag-begin");
    ConnectGtkSignal(this, &PreviewPanel::OnPreviewDragUpdate, dragHandler, "drag-update");
    ConnectGtkSignal(this, &PreviewPanel::OnPreviewDragEnd, dragHandler, "drag-end");
    ConnectGtkSignal(this, &PreviewPanel::OnMouseMove, mouseHandler, "motion");
    ConnectGtkSignal(this, &PreviewPanel::OnMouseScroll, scrollHandler, "scroll");

    m_PreviewState = new PreviewState(m_App->GetState());
    m_ViewUpdateObserver = new ViewUpdateObserver(this, m_PreviewState);
    m_App->GetObserverManager()->AddObserver(m_ViewUpdateObserver);

    m_Dispatcher.RegisterHandler(SetPanCommand::Execute, m_PreviewState);
    m_Dispatcher.RegisterHandler(SetZoomCommand::Execute, m_PreviewState);
}

Gorfector::PreviewPanel::~PreviewPanel()
{
    m_Dispatcher.UnregisterHandler<SetPanCommand>();
    m_Dispatcher.UnregisterHandler<SetZoomCommand>();

    m_App->GetObserverManager()->RemoveObserver(m_ViewUpdateObserver);

    delete m_ViewUpdateObserver;
    delete m_PreviewState;

    m_ViewUpdateObserver = nullptr;
    m_PreviewState = nullptr;
}

void Gorfector::PreviewPanel::OnResized(GtkWidget *widget, void *data, void *)
{
    if (widget != m_PreviewImage)
        return;

    auto width = gtk_widget_get_size(widget, GTK_ORIENTATION_HORIZONTAL);
    auto height = gtk_widget_get_size(widget, GTK_ORIENTATION_VERTICAL);

    auto currentWidth = m_PreviewPixBuf != nullptr ? gdk_pixbuf_get_width(m_PreviewPixBuf) : 0;
    auto currentHeight = m_PreviewPixBuf != nullptr ? gdk_pixbuf_get_height(m_PreviewPixBuf) : 0;

    if (width == currentWidth && height == currentHeight)
    {
        return;
    }

    if (m_PreviewPixBuf != nullptr)
    {
        g_object_unref(m_PreviewPixBuf);
        m_PreviewPixBuf = nullptr;
    }

    Redraw();

    auto updater = PreviewState::Updater(m_PreviewState);
    updater.SetPreviewWindowSize(width, height);
}

void Gorfector::PreviewPanel::OnZoomDropDownChanged(GtkDropDown *dropDown, void *data)
{
    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        return;
    }

    auto zoomValue = gtk_drop_down_get_selected(dropDown);
    double zoomFactor = PreviewState::k_ZoomValues[zoomValue];
    m_Dispatcher.Dispatch(SetZoomCommand(zoomFactor));
}

/**
 * Computes a scan area from display pixel values.
 * @param deltaX The change in X direction, in display pixels.
 * @param deltaY The change in Y direction, in display pixels.
 * @param outScanArea The new scan area in mm.
 */
void Gorfector::PreviewPanel::ComputeScanArea(double deltaX, double deltaY, Rect<double> &outScanArea) const
{
    auto resolution = m_PreviewState->GetPreviewResolution();
    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr || resolution <= 1.)
    {
        outScanArea = Rect<double>{0, 0, 0, 0};
        return;
    }

    auto scale = 25.4 / (resolution * m_ZoomFactor);
    auto pan = m_PreviewState->GetPreviewPanOffset();
    auto originalScanArea = m_PixelScanArea - pan;

    auto maxScanArea = m_App->GetDeviceOptions()->GetMaxScanArea();
    auto maxWidth = maxScanArea.width / scale;
    auto maxHeight = maxScanArea.height / scale;

    switch (m_DragMode)
    {
        case DragMode::Top:
        {
            deltaY = std::min(deltaY, originalScanArea.height);
            deltaY = std::max(deltaY, -originalScanArea.y);

            outScanArea.x = originalScanArea.x * scale;
            outScanArea.y = (originalScanArea.y + deltaY) * scale;
            outScanArea.width = originalScanArea.width * scale;
            outScanArea.height = (originalScanArea.height - deltaY) * scale;
            break;
        }
        case DragMode::Bottom:
        {
            deltaY = std::max(deltaY, -originalScanArea.height);
            deltaY = std::min(deltaY, maxHeight - originalScanArea.y - originalScanArea.height);

            outScanArea.x = originalScanArea.x * scale;
            outScanArea.y = originalScanArea.y * scale;
            outScanArea.width = originalScanArea.width * scale;
            outScanArea.height = (originalScanArea.height + deltaY) * scale;
            break;
        }
        case DragMode::Left:
        {
            deltaX = std::min(deltaX, originalScanArea.width);
            deltaX = std::max(deltaX, -originalScanArea.x);

            outScanArea.x = (originalScanArea.x + deltaX) * scale;
            outScanArea.y = originalScanArea.y * scale;
            outScanArea.width = (originalScanArea.width - deltaX) * scale;
            outScanArea.height = originalScanArea.height * scale;
            break;
        }
        case DragMode::Right:
        {
            deltaX = std::max(deltaX, -originalScanArea.width);
            deltaX = std::min(deltaX, maxWidth - originalScanArea.x - originalScanArea.width);

            outScanArea.x = originalScanArea.x * scale;
            outScanArea.y = originalScanArea.y * scale;
            outScanArea.width = (originalScanArea.width + deltaX) * scale;
            outScanArea.height = originalScanArea.height * scale;
            break;
        }
        case DragMode::TopLeft:
        {
            deltaX = std::min(deltaX, originalScanArea.width);
            deltaX = std::max(deltaX, -originalScanArea.x);
            deltaY = std::min(deltaY, originalScanArea.height);
            deltaY = std::max(deltaY, -originalScanArea.y);

            outScanArea.x = (originalScanArea.x + deltaX) * scale;
            outScanArea.y = (originalScanArea.y + deltaY) * scale;
            outScanArea.width = (originalScanArea.width - deltaX) * scale;
            outScanArea.height = (originalScanArea.height - deltaY) * scale;
            break;
        }
        case DragMode::TopRight:
        {
            deltaX = std::max(deltaX, -originalScanArea.width);
            deltaX = std::min(deltaX, maxWidth - originalScanArea.x - originalScanArea.width);
            deltaY = std::min(deltaY, originalScanArea.height);
            deltaY = std::max(deltaY, -originalScanArea.y);

            outScanArea.x = originalScanArea.x * scale;
            outScanArea.y = (originalScanArea.y + deltaY) * scale;
            outScanArea.width = (originalScanArea.width + deltaX) * scale;
            outScanArea.height = (originalScanArea.height - deltaY) * scale;
            break;
        }
        case DragMode::BottomLeft:
        {
            deltaX = std::min(deltaX, originalScanArea.width);
            deltaX = std::max(deltaX, -originalScanArea.x);
            deltaY = std::max(deltaY, -originalScanArea.height);
            deltaY = std::min(deltaY, maxHeight - originalScanArea.y - originalScanArea.height);

            outScanArea.x = (originalScanArea.x + deltaX) * scale;
            outScanArea.y = originalScanArea.y * scale;
            outScanArea.width = (originalScanArea.width - deltaX) * scale;
            outScanArea.height = (originalScanArea.height + deltaY) * scale;
            break;
        }
        case DragMode::BottomRight:
        {
            deltaX = std::max(deltaX, -originalScanArea.width);
            deltaX = std::min(deltaX, maxWidth - originalScanArea.x - originalScanArea.width);
            deltaY = std::max(deltaY, -originalScanArea.height);
            deltaY = std::min(deltaY, maxHeight - originalScanArea.y - originalScanArea.height);

            outScanArea.x = originalScanArea.x * scale;
            outScanArea.y = originalScanArea.y * scale;
            outScanArea.width = (originalScanArea.width + deltaX) * scale;
            outScanArea.height = (originalScanArea.height + deltaY) * scale;
            break;
        }
        case DragMode::Move:
        {
            deltaX = std::max(deltaX, -originalScanArea.x);
            deltaX = std::min(deltaX, maxWidth - originalScanArea.x - originalScanArea.width);
            deltaY = std::max(deltaY, -originalScanArea.y);
            deltaY = std::min(deltaY, maxHeight - originalScanArea.y - originalScanArea.height);

            outScanArea.x = (originalScanArea.x + deltaX) * scale;
            outScanArea.y = (originalScanArea.y + deltaY) * scale;
            outScanArea.width = originalScanArea.width * scale;
            outScanArea.height = originalScanArea.height * scale;
            break;
        }
        case DragMode::Draw:
        default:
        {
            deltaX = std::min(deltaX, maxWidth - m_DragStartX);
            deltaX = std::max(deltaX, -m_DragStartX);
            deltaY = std::min(deltaY, maxHeight - m_DragStartY);
            deltaY = std::max(deltaY, -m_DragStartY);

            auto dragStartX = m_DragStartX - pan.x;
            auto dragStartY = m_DragStartY - pan.y;

            outScanArea.x = std::min(dragStartX, dragStartX + deltaX) * scale;
            outScanArea.y = std::min(dragStartY, dragStartY + deltaY) * scale;
            outScanArea.width = std::max(dragStartX, dragStartX + deltaX) * scale - outScanArea.x;
            outScanArea.height = std::max(dragStartY, dragStartY + deltaY) * scale - outScanArea.y;
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

/**
 * Maps a scan area in mm to display pixel coordinates.
 * @param scanArea The scan area to be mapped, in SA units.
 * @param outPixelArea The output pixel area.
 * @returns True if the mapping was successful, false otherwise.
 */
bool Gorfector::PreviewPanel::ScanAreaToPixels(const Rect<double> &scanArea, Rect<double> &outPixelArea) const
{
    auto resolution = m_PreviewState->GetPreviewResolution();
    if (resolution > 1. && m_ZoomFactor != 0.0)
    {
        auto panOffset = m_PreviewState->GetPreviewPanOffset();
        auto scale = m_ZoomFactor * resolution / 25.4;

        outPixelArea.x = scanArea.x * scale + panOffset.x;
        outPixelArea.y = scanArea.y * scale + panOffset.y;
        outPixelArea.width = scanArea.width * scale;
        outPixelArea.height = scanArea.height * scale;

        return true;
    }

    return false;
}

ScanAreaCursorRegions GetScanAreaCursorRegion(const Gorfector::Rect<double> &pixelArea, double x, double y)
{
    static constexpr double tolerance = 5.0;

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

void Gorfector::PreviewPanel::OnPreviewDragBegin(GtkGestureDrag *dragController)
{
    if (m_PreviewState->GetScannedPixelsPerLine() == 0 || m_PreviewState->GetScannedImageHeight() == 0)
        return;

    m_IsDragging = true;

    double x, y;
    gtk_gesture_drag_get_start_point(dragController, &m_DragStartX, &m_DragStartY);
    gtk_gesture_drag_get_offset(dragController, &x, &y);

    auto modifiers = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(dragController)) & gdkModifiers;
    auto isShiftAndMaybeAlt = ((modifiers ^ (GDK_SHIFT_MASK | GDK_ALT_MASK)) | GDK_ALT_MASK) == GDK_ALT_MASK;

    if (isShiftAndMaybeAlt)
    {
        if (auto deviceOptions = m_App->GetDeviceOptions();
            deviceOptions != nullptr && ScanAreaToPixels(deviceOptions->GetScanArea(), m_PixelScanArea))
        {
            auto cursorRegion = GetScanAreaCursorRegion(m_PixelScanArea, m_DragStartX, m_DragStartY);
            switch (cursorRegion)
            {
                case ScanAreaCursorRegions::Top:
                    m_DragMode = DragMode::Top;
                    break;
                case ScanAreaCursorRegions::Bottom:
                    m_DragMode = DragMode::Bottom;
                    break;
                case ScanAreaCursorRegions::Left:
                    m_DragMode = DragMode::Left;
                    break;
                case ScanAreaCursorRegions::Right:
                    m_DragMode = DragMode::Right;
                    break;
                case ScanAreaCursorRegions::TopLeft:
                    m_DragMode = DragMode::TopLeft;
                    break;
                case ScanAreaCursorRegions::TopRight:
                    m_DragMode = DragMode::TopRight;
                    break;
                case ScanAreaCursorRegions::BottomLeft:
                    m_DragMode = DragMode::BottomLeft;
                    break;
                case ScanAreaCursorRegions::BottomRight:
                    m_DragMode = DragMode::BottomRight;
                    break;
                case ScanAreaCursorRegions::Inside:
                    if (modifiers & GDK_ALT_MASK)
                    {
                        m_DragMode = DragMode::Move;
                    }
                    else
                    {
                        m_DragMode = DragMode::Draw;
                    }
                    break;
                default:
                    m_DragMode = DragMode::Draw;
                    break;
            }
        }
        else
        {
            m_PixelScanArea = Rect<double>{0, 0, 0, 0};
            m_DragMode = DragMode::Draw;
        }

        Rect<double> scanArea;
        ComputeScanArea(x, y, scanArea);
        m_Dispatcher.Dispatch(SetScanAreaCommand(scanArea));
    }
    else if (modifiers == 0)
    {
        m_OriginalPan = m_PreviewState->GetPreviewPanOffset();
        m_DragMode = DragMode::Pan;
        m_Dispatcher.Dispatch(SetPanCommand(m_OriginalPan + Point{x, y}));
    }
}

void Gorfector::PreviewPanel::OnPreviewDragUpdate(GtkGestureDrag *dragController)
{
    if (m_DragMode == DragMode::None || m_PreviewState->GetScannedPixelsPerLine() == 0 ||
        m_PreviewState->GetScannedImageHeight() == 0)
        return;

    double x, y;
    gtk_gesture_drag_get_offset(dragController, &x, &y);

    if (m_DragMode == DragMode::Pan)
    {
        m_Dispatcher.Dispatch(SetPanCommand(m_OriginalPan + Point{x, y}));
    }
    else
    {
        Rect<double> scanArea;
        ComputeScanArea(x, y, scanArea);
        m_Dispatcher.Dispatch(SetScanAreaCommand(scanArea));
    }
}

void Gorfector::PreviewPanel::OnPreviewDragEnd(GtkGestureDrag *dragController)
{
    m_IsDragging = false;

    if (m_DragMode == DragMode::None || m_PreviewState->GetScannedPixelsPerLine() == 0 ||
        m_PreviewState->GetScannedImageHeight() == 0)
        return;

    double x, y;
    gtk_gesture_drag_get_offset(dragController, &x, &y);

    if (m_DragMode == DragMode::Pan)
    {
        m_Dispatcher.Dispatch(SetPanCommand(m_OriginalPan + Point{x, y}));
    }
    else
    {
        Rect<double> scanArea;
        ComputeScanArea(x, y, scanArea);
        m_Dispatcher.Dispatch(SetScanAreaCommand(scanArea));
    }

    m_DragMode = DragMode::None;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Gorfector::PreviewPanel::OnMouseMove(GtkEventControllerMotion *motionController, gdouble x, gdouble y)
{
    m_LastMousePosition = Point{x, y};

    if (m_IsDragging)
    {
        return;
    }

    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        return;
    }

    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(motionController));
    auto modifiers =
            gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(motionController)) & gdkModifiers;
    auto isShiftAndMaybeAlt = ((modifiers ^ (GDK_SHIFT_MASK | GDK_ALT_MASK)) | GDK_ALT_MASK) == GDK_ALT_MASK;

    if (isShiftAndMaybeAlt)
    {
        Rect<double> scanArea = m_App->GetDeviceOptions()->GetScanArea();
        Rect<double> pixelArea;
        if (!ScanAreaToPixels(scanArea, pixelArea))
        {
            gtk_widget_set_cursor_from_name(widget, "default");
            return;
        }

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
    else if (modifiers == 0)
    {
        gtk_widget_set_cursor_from_name(widget, "default");
    }
}

void Gorfector::PreviewPanel::OnMouseScroll(GtkEventControllerScroll *scrollController, gdouble deltaX, gdouble deltaY)
{
    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        return;
    }

    auto modifiers =
            gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(scrollController)) & gdkModifiers;

    if (modifiers & GDK_CONTROL_MASK)
    {
        auto currentZoomIndex = gtk_drop_down_get_selected(GTK_DROP_DOWN(m_ZoomDropDown));
        if (deltaY < 0)
        {
            if (currentZoomIndex < std::size(PreviewState::k_ZoomValues) - 1)
            {
                m_Dispatcher.Dispatch(
                        SetZoomCommand(PreviewState::k_ZoomValues[currentZoomIndex + 1], m_LastMousePosition));
            }
        }
        else
        {
            if (currentZoomIndex > 0)
            {
                m_Dispatcher.Dispatch(
                        SetZoomCommand(PreviewState::k_ZoomValues[currentZoomIndex - 1], m_LastMousePosition));
            }
        }
    }
    else
    {
        if (modifiers & GDK_SHIFT_MASK)
        {
            std::swap(deltaX, deltaY);
        }
        auto pan = m_PreviewState->GetPreviewPanOffset();
        pan.x -= deltaX * 100;
        pan.y -= deltaY * 100;
        m_Dispatcher.Dispatch(SetPanCommand(pan));
    }
}

void Gorfector::PreviewPanel::OnPreviewDraw(cairo_t *cr) const
{
    gdk_cairo_set_source_pixbuf(cr, m_PreviewPixBuf, 0, 0);
    cairo_paint(cr);

    if (m_App == nullptr || m_App->GetDeviceOptions() == nullptr)
    {
        return;
    }

    Rect<double> scanArea = m_App->GetDeviceOptions()->GetScanArea();
    Rect<double> pixelArea;
    if (!ScanAreaToPixels(scanArea, pixelArea))
    {
        return;
    }

    cairo_set_operator(cr, CAIRO_OPERATOR_EXCLUSION);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, .50);
    cairo_rectangle(cr, pixelArea.x, pixelArea.y, pixelArea.width, pixelArea.height);
    cairo_stroke(cr);
}

void Gorfector::PreviewPanel::Update(const std::vector<uint64_t> &lastSeenVersions)
{
    auto changeset = m_PreviewState->GetAggregatedChangeset(lastSeenVersions[0]);
    if (changeset == nullptr || !changeset->HasAnyChange())
    {
        return;
    }

    if (changeset->IsChanged(PreviewStateChangeset::TypeFlag::Image))
    {
        if (const auto image = m_PreviewState->GetScannedImage(); image != nullptr)
        {
            auto width = m_PreviewState->GetScannedPixelsPerLine();
            auto bytesPerLine = m_PreviewState->GetScannedBytesPerLine();
            auto height = m_PreviewState->GetScannedImageHeight();

            if (m_PreviewState->GetScannedImageBitDepth() == 8 &&
                m_PreviewState->GetScannedImagePixelFormat() == SANE_FRAME_RGB)
            {
                if (m_UnderlyingBuffer != m_PreviewState->GetScannedImage())
                {
                    m_UnderlyingBuffer = m_PreviewState->GetScannedImage();

                    if (m_ScannedImage != nullptr)
                    {
                        g_object_unref(m_ScannedImage);
                        m_ScannedImage = nullptr;
                    }
                }

                if (m_ScannedImage == nullptr)
                {
                    m_ScannedImage = gdk_pixbuf_new_from_data(
                            m_UnderlyingBuffer, GDK_COLORSPACE_RGB, false, 8, width, height, bytesPerLine, nullptr,
                            nullptr);
                }

                delete[] m_ConvertedImage;
                m_ConvertedImageSize = 0;
                m_ConvertedImage = nullptr;
            }
            else
            {
                int lastLineToConvert = changeset->GetLastLine();
                if (lastLineToConvert < 0)
                {
                    m_LastLineConverted = -1;
                    return;
                }
                if (lastLineToConvert < m_LastLineConverted || m_LastLineConverted >= height)
                {
                    // Assume this is a new image.
                    m_LastLineConverted = -1;
                }

                if (m_ConvertedImage == nullptr || m_ConvertedImageSize != width * height * 3)
                {
                    if (m_ScannedImage != nullptr)
                    {
                        g_object_unref(m_ScannedImage);
                        m_ScannedImage = nullptr;
                    }

                    delete[] m_ConvertedImage;
                    m_ConvertedImageSize = width * height * 3;
                    m_ConvertedImage = new unsigned char[m_ConvertedImageSize];
                    m_UnderlyingBuffer = m_ConvertedImage;

                    m_ScannedImage = gdk_pixbuf_new_from_data(
                            m_UnderlyingBuffer, GDK_COLORSPACE_RGB, false, 8, width, height, 3 * width, nullptr,
                            nullptr);
                }

                if (m_PreviewState->GetScannedImageBitDepth() == 1)
                {
                    // Convert 1-bit black and white (min is white) to 8-bit RGB
                    for (int y = m_LastLineConverted + 1; y <= lastLineToConvert; ++y)
                    {
                        auto line = &image[y * bytesPerLine];
                        for (int x = 0; x < width; ++x)
                        {
                            int dstPixelIndex = 3 * (y * width + x);
                            unsigned char pixelValue = line[x / 8] & (1 << (7 - (x % 8)));
                            m_ConvertedImage[dstPixelIndex] = pixelValue ? 0 : 255;
                            m_ConvertedImage[dstPixelIndex + 1] = pixelValue ? 0 : 255;
                            m_ConvertedImage[dstPixelIndex + 2] = pixelValue ? 0 : 255;
                        }
                    }
                }
                else if (
                        m_PreviewState->GetScannedImageBitDepth() == 8 &&
                        m_PreviewState->GetScannedImagePixelFormat() == SANE_FRAME_GRAY)
                {
                    // Convert 8-bit grayscale to 8-bit RGB
                    for (int y = m_LastLineConverted + 1; y <= lastLineToConvert; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            int dstPixelIndex = 3 * (y * width + x);
                            int srcPixelIndex = y * bytesPerLine + x;
                            unsigned char pixelValue = image[srcPixelIndex];
                            m_ConvertedImage[dstPixelIndex] = pixelValue;
                            m_ConvertedImage[dstPixelIndex + 1] = pixelValue;
                            m_ConvertedImage[dstPixelIndex + 2] = pixelValue;
                        }
                    }
                }
                else if (m_PreviewState->GetScannedImageBitDepth() == 16)
                {
                    constexpr auto offset = std::endian::native == std::endian::little ? 1 : 0;

                    if (m_PreviewState->GetScannedImagePixelFormat() == SANE_FRAME_GRAY)
                    {
                        // Convert 16-bit grayscale to 8-bit RGB
                        for (int y = m_LastLineConverted + 1; y <= lastLineToConvert; ++y)
                        {
                            auto line = &image[y * bytesPerLine];
                            for (int x = 0; x < width; ++x)
                            {
                                int dstPixelIndex = 3 * (y * width + x);
                                unsigned char pixelValue = line[2 * x + offset];
                                m_ConvertedImage[dstPixelIndex] = pixelValue;
                                m_ConvertedImage[dstPixelIndex + 1] = pixelValue;
                                m_ConvertedImage[dstPixelIndex + 2] = pixelValue;
                            }
                        }
                    }
                    else if (m_PreviewState->GetScannedImagePixelFormat() == SANE_FRAME_RGB)
                    {
                        // Convert 16-bit RGB to 8-bit RGB
                        for (int y = m_LastLineConverted + 1; y <= lastLineToConvert; ++y)
                        {
                            auto line = &image[y * bytesPerLine];
                            for (int x = 0; x < width; ++x)
                            {
                                int dstPixelIndex = 3 * (y * width + x);
                                m_ConvertedImage[dstPixelIndex] = line[6 * x + offset];
                                m_ConvertedImage[dstPixelIndex + 1] = line[6 * x + 2 + offset];
                                m_ConvertedImage[dstPixelIndex + 2] = line[6 * x + 4 + offset];
                            }
                        }
                    }
                }

                m_LastLineConverted = lastLineToConvert;
            }
        }
    }

    bool updateZoomDropDown = false;
    if (changeset->IsChanged(PreviewStateChangeset::TypeFlag::ZoomFactor))
    {
        m_ZoomFactor = m_PreviewState->GetPreviewZoomFactor();
        updateZoomDropDown = true;
    }

    // If zoom factor is 0, it will be computed in Redraw().
    updateZoomDropDown |= (m_ZoomFactor == 0.0);

    Redraw();

    if (updateZoomDropDown)
    {
        for (auto i = 0UZ; i < std::size(PreviewState::k_ZoomValues); ++i)
        {
            auto zoomValue = PreviewState::k_ZoomValues[i];
            if (std::abs(m_ZoomFactor - zoomValue) < 0.0001)
            {
                gtk_drop_down_set_selected(GTK_DROP_DOWN(m_ZoomDropDown), i);
                break;
            }
        }
    }

    if (changeset->IsChanged(PreviewStateChangeset::TypeFlag::Progress))
    {
        auto min = static_cast<double>(m_PreviewState->GetProgressMin());
        auto max = static_cast<double>(m_PreviewState->GetProgressMax());
        if (min == max)
        {
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(m_ProgressBar), nullptr);
            gtk_widget_set_visible(m_ProgressBar, false);
        }
        else
        {
            gtk_widget_set_visible(m_ProgressBar, true);
            auto current = static_cast<double>(m_PreviewState->GetProgressCurrent());
            current = (current - min) / (max - min);

            auto text = m_PreviewState->GetProgressText();
            if (text.empty())
            {
                text = _("Progress: ");
            }
            else
            {
                text += ": ";
            }

            text += std::to_string(static_cast<int>(current * 100.0)) + "%";

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_ProgressBar), current);
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(m_ProgressBar), text.c_str());
        }
    }

    gtk_widget_queue_draw(m_PreviewImage);
}

void Gorfector::PreviewPanel::Redraw()
{
    if (m_PreviewPixBuf == nullptr)
    {
        auto width = gtk_widget_get_size(m_PreviewImage, GTK_ORIENTATION_HORIZONTAL);
        auto height = gtk_widget_get_size(m_PreviewImage, GTK_ORIENTATION_VERTICAL);
        if (width == 0 || height == 0)
        {
            return;
        }

        m_PreviewPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, width, height);
    }

    if (m_ScannedImage != nullptr)
    {
        auto displayWidth = m_PreviewPixBuf != nullptr ? gdk_pixbuf_get_width(m_PreviewPixBuf) : 0.0;
        auto displayHeight = m_PreviewPixBuf != nullptr ? gdk_pixbuf_get_height(m_PreviewPixBuf) : 0.0;
        auto scannedWidth = gdk_pixbuf_get_width(m_ScannedImage);
        auto scannedHeight = gdk_pixbuf_get_height(m_ScannedImage);

        auto pan = m_PreviewState->GetPreviewPanOffset();
        if (m_ZoomFactor == 0.0)
        {
            auto scaleW = displayWidth / scannedWidth;
            auto scaleH = displayHeight / scannedHeight;

            m_ZoomFactor = PreviewState::FloorZoomFactor(std::min(scaleW, scaleH));
        }

        auto previewWidth = std::min(scannedWidth * m_ZoomFactor, displayWidth - pan.x);
        auto previewHeight = std::min(scannedHeight * m_ZoomFactor, displayHeight - pan.y);

        auto destX = static_cast<int>(std::max(0.0, pan.x));
        auto destY = static_cast<int>(std::max(0.0, pan.y));

        previewWidth -= (destX - pan.x);
        previewHeight -= (destY - pan.y);

        FillWithEmptyPattern();
        gdk_pixbuf_scale(
                m_ScannedImage, m_PreviewPixBuf, destX, destY, static_cast<int>(previewWidth),
                static_cast<int>(previewHeight), pan.x, pan.y, m_ZoomFactor, m_ZoomFactor, GDK_INTERP_NEAREST);
    }
    else
    {
        FillWithEmptyPattern();
    }
}

void Gorfector::PreviewPanel::FillWithEmptyPattern() const
{
    g_assert(gdk_pixbuf_get_colorspace(m_PreviewPixBuf) == GDK_COLORSPACE_RGB);
    g_assert(gdk_pixbuf_get_bits_per_sample(m_PreviewPixBuf) == 8);
    g_assert(gdk_pixbuf_get_has_alpha(m_PreviewPixBuf) == false);
    g_assert(gdk_pixbuf_get_n_channels(m_PreviewPixBuf) == 3);

    auto *pixbuf = gdk_pixbuf_get_pixels(m_PreviewPixBuf);
    int height = gdk_pixbuf_get_height(m_PreviewPixBuf);
    int width = gdk_pixbuf_get_width(m_PreviewPixBuf);
    int rowStride = gdk_pixbuf_get_rowstride(m_PreviewPixBuf);

    constexpr guchar k_Light = 0xFF;
    constexpr guchar k_Dark = 0xEE;

    for (int y = 0; y < height; ++y)
    {
        auto *ptr = pixbuf + y * rowStride;
        for (int x = 0; x < width; ++x)
        {
            if (((x & 0x40) && !(y & 0x40)) || (!(x & 0x40) && (y & 0x40)))
            {
                *ptr++ = k_Dark;
                *ptr++ = k_Dark;
                *ptr++ = k_Dark;
            }
            else
            {
                *ptr++ = k_Light;
                *ptr++ = k_Light;
                *ptr++ = k_Light;
            }
        }
    }
}
