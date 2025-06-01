# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Ability to update the presets.

### Changed

- UI that changes the scan parameters is now disabled when a scan is in progress.
- Scan data reception now occurs in a separate thread, preventing the scanner from stalling.
- Preview image is cleared when a new preview is started.

### Fixed

- Fixed a crash when applying a preset that tried to set a hidden parameter.
- Cancel Scans button from the scan list section now can cancel the preview scan.
- PNG writer is now working.
- Various crashes with the SANE test backend have been fixed.


## [0.1.0] - 2025-05-24

### Added

- Initial release of **Gorfector**.
- User defined presets.
- Output to file (TIFF, PNG, JPEG), to email or to printer.
- Pan and zoom of the preview image.
- Define scan area by dragging a rectangle on the preview image.
- Batch scanning of multiple areas in a single document.
- Parameter rewriting for Epson Perfection V600 Photo scanner.
- French translation of the user interface and help files.

[unreleased]: https://github.com/patrickfournier/gorfector/compare/v0.1...HEAD
[0.1.0]: https://github.com/patrickfournier/gorfector/releases/tag/v0.1.0
