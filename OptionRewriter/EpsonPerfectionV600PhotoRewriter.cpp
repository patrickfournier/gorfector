#include "EpsonPerfectionV600PhotoRewriter.hpp"

const std::map<uint32_t, ZooScan::OptionInfos> ZooScan::EpsonPerfectionV600PhotoRewriter::k_OptionInfos{
        {1, {"Color Adjustments", "", {}, 0x0}}, // <no name> ()
        {2, {"Scan Mode", "Selects the scan mode.", {"Black and White", "Gray", "Color", nullptr}, 0x0}}, // mode ()
        {3, {"Bit Depth", "Number of bits per sample.", {}, 0x0}}, // depth ()
        {4,
         {"Halftoning",
          "Selects the halftone screen.",
          {"None", "Halftone A (Hard Tone)", "Halftone B (Soft Tone)", "Halftone C (Net Screen)", nullptr},
          0x0}}, // halftoning ()
        {5, {"Dropout", "Selects the dropout.", {"None", "Red", "Green", "Blue", nullptr}, 0x0}}, // dropout (advanced)
        {6,
         {"Brightness Method",
          "Selects a method to change the brightness of the acquired image.",
          {"iscan", "gimp", nullptr},
          0x0}}, // brightness-method (advanced)
        {7, {"Brightness", "Controls the brightness of the acquired image.", {}, 0x0}}, // brightness ()
        {8, {"Contrast", "Controls the contrast of the acquired image.", {}, 0x0}}, // contrast ()
        {9, {"Sharpness", "", {}, 0x0}}, // sharpness ()
        {10,
         {"Gamma Correction", "Selects the gamma correction value.", {"1.0", "1.8", nullptr}, 0x0}}, // gamma-correction
                                                                                                     // ()
        {11,
         {"Color correction",
          "Sets the color correction table for the selected output device.",
          {"User defined", nullptr},
          0x0}}, // color-correction (advanced)
        {12, {"Scan Resolution", "Sets the resolution of the scanned image.", {}, 0x0}}, // resolution ()
        {13,
         {"Horizontal Resolution", "Sets the horizontal resolution of the scanned image.", {}, 0x0}}, // x-resolution
                                                                                                      // (advanced)
        {14, {"Vertical Resolution", "Sets the vertical resolution of the scanned image.", {}, 0x0}}, // y-resolution
                                                                                                      // (advanced)
        {15, {"Threshold", "Select minimum-brightness to get a white point", {}, 0x0}}, // threshold ()
        {16, {"Advanced", "", {}, 0x0}}, // <no name> (advanced)
        {17, {"Mirror image", "Mirror the image.", {}, 0x0}}, // mirror ()
        {18,
         {"Scan speed",
          "Determines the speed at which the scan proceeds.",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // speed ()
        {19, {"Automatic Area Segmentation", "", {}, 0x0}}, // auto-area-segmentation ()
        {20,
         {"Short resolution list",
          "Display short resolution list",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // short-resolution ()
        {21, {"Zoom", "Defines the zoom factor the scanner will use", {}, 0x0}}, // zoom ()
        {22, {"Red intensity", "Gamma-correction table for the red band.", {}, 0x0}}, // red-gamma-table ()
        {23, {"Green intensity", "Gamma-correction table for the green band.", {}, 0x0}}, // green-gamma-table ()
        {24, {"Blue intensity", "Gamma-correction table for the blue band.", {}, 0x0}}, // blue-gamma-table ()
        {25,
         {"Wait for Button",
          "After sending the scan command, wait until the button on the scanner is pressed to actually start the scan "
          "process.",
          {},
          0x0}}, // wait-for-button (advanced)
        {26,
         {"Monitor Button",
          "Indicates whether a button on the scanner has been pressed.",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // monitor-button (advanced, display only)
        {27,
         {"Polling Time",
          "Time between queries when waiting for device state changes.",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // polling-time (advanced, display only)
        {28,
         {"Needs Polling",
          "Indicates whether the scanner needs to poll.",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // needs-polling (advanced, display only)
        {29,
         {"Color Correction Coefficients", "Matrix multiplication of RGB values.", {}, 0x0}}, // <no name> (advanced)
        {30, {"Red", "Controls red level", {}, 0x0}}, // cct-1 (advanced)
        {31, {"Shift Green to Red", "Adds to red based on green level", {}, 0x0}}, // cct-2 (advanced)
        {32, {"Shift Blue to Red", "Adds to red based on blue level", {}, 0x0}}, // cct-3 (advanced)
        {33, {"Shift Red to Green", "Adds to green based on red level", {}, 0x0}}, // cct-4 (advanced)
        {34, {"Green", "Controls green level", {}, 0x0}}, // cct-5 (advanced)
        {35, {"Shift Blue to Green", "Adds to green based on blue level", {}, 0x0}}, // cct-6 (advanced)
        {36, {"Shift Red to Blue", "Adds to blue based on red level", {}, 0x0}}, // cct-7 (advanced)
        {37, {"Shift Green to Blue", "Adds to blue based on green level", {}, 0x0}}, // cct-8 (advanced)
        {38, {"Blue", "Control blue level", {}, 0x0}}, // cct-9 (advanced)
        {39, {"Preview", "", {}, 0x0}}, // <no name> (advanced)
        {40, {"Preview", "Request a preview-quality scan.", {}, 0x0}}, // preview ()
        {41, {"Speed", "", {}, static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // preview-speed ()
        {42, {"Geometry", "", {}, 0x0}}, // <no name> (advanced)
        {43,
         {"Scan Area",
          "Set the scan area based on well-known media sizes.",
          {"Maximum", "A4", "A5 Landscape", "A5 Portrait", "B5", "Letter", "Executive", "CD", nullptr},
          0x0}}, // scan-area ()
        {44, {"Top Left X", "Top-left x position of scan area.", {}, 0x0}}, // tl-x ()
        {45, {"Top Left Y", "Top-left y position of scan area.", {}, 0x0}}, // tl-y ()
        {46, {"Bottom Right X", "Bottom-right x position of scan area.", {}, 0x0}}, // br-x ()
        {47, {"Bottom Right Y", "Bottom-right y position of scan area.", {}, 0x0}}, // br-y ()
        {48,
         {"Quick format",
          "Select an area to scan based on well-known media sizes. (DEPRECATED)",
          {"Maximum", "A4", "A5 Landscape", "A5 Portrait", "B5", "Letter", "Executive", "CD", nullptr},
          0x0}}, // quick-format ()
        {49, {"Optional Equipment", "", {}, 0x0}}, // <no name> (advanced)
        {50, {"Scan Source", "Selects the scan source.", {"Flatbed", "Transparency Unit", nullptr}, 0x0}}, // source ()
        {51, {"Auto eject", "Eject document after scanning", {}, 0x0}}, // auto-eject ()
        {52, {"Film Type", "", {"Positive Film", "Negative Film", nullptr}, 0x0}}, // film-type ()
        {53,
         {"Focus Position",
          "Sets the focus position to either the glass or 2.5mm above the glass",
          {"Focus on glass", "Focus 2.5mm above glass", nullptr},
          0x0}}, // focus-position (advanced)
        {54, {"Bay", "Select bay to scan", {" 1 ", " 2 ", " 3 ", " 4 ", " 5 ", " 6 ", nullptr}, 0x0}}, // bay ()
        {55, {"Eject", "Eject the sheet in the ADF", {}, 0x0}}, // eject ()
        {56, {"ADF Mode", "Selects the ADF mode (simplex/duplex)", {"Simplex", "Duplex", nullptr}, 0x0}}, // adf-mode ()
        {57,
         {"Auto-detect document size",
          "Activates document size auto-detection.  The scan area will be set to match the detected document size.",
          {},
          0x0}}, // detect-doc-size (advanced)
        {58,
         {"Scan Area Is Valid",
          "Indicates whether the current scan area settings are valid.",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // scan-area-is-valid (advanced, display only)
        {59,
         {"ADF Auto Scan", "Skips per sheet device setup for faster throughput.", {}, 0x0}}, // adf-auto-scan (advanced)
        {60,
         {"Double Feed Detection Sensitivity",
          "Sets the sensitivity with which multi-sheet page feeds are detected and reported as errors.",
          {"None", "Low", "High", nullptr},
          0x0}}, // double-feed-detection-sensitivity (advanced)
        {61,
         {"Extended SANE Status",
          "Ugly kludge to provide additional status message strings to a frontend.",
          {},
          static_cast<uint32_t>(OptionFlags::e_ForceHidden)}}, // ext-sane-status (advanced, display only)
        {62,
         {"ADF Duplex Direction Matches",
          "Indicates whether the device's ADF duplex mode, if available, scans in the same direction for the front and "
          "back.",
          {},
          0x0}}, // adf-duplex-direction-matches (advanced, display only)
        {63, {"Deskew", "Rotate image so it appears upright.", {}, 0x0}}, // deskew (advanced)
        {64,
         {"Trim image to paper size",
          "Determines empty margins in the scanned image and removes them.  This normally reduces the image to the "
          "size of the original document but may remove more.",
          {},
          0x0}}, // autocrop (advanced)
        {65,
         {"Calibrate",
          "Performs color matching to make sure that the document's color tones are scanned correctly.",
          {},
          0x0}}, // calibrate (advanced)
        {66, {"Clean", "Cleans the scanners reading section.", {}, 0x0}}, // clean (advanced)
};
