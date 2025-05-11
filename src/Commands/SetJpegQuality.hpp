#pragma once

#include "Writers/JpegWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetJpegQuality
     * \brief Command class to set the JPEG quality in the `JpegWriterState`.
     *
     * This class encapsulates the logic for updating the JPEG quality setting
     * in the `JpegWriterState`.
     */
    class SetJpegQuality : public ZooLib::Command
    {
        /**
         * \brief The desired JPEG quality value.
         */
        const int m_JpegQuality{};

    public:
        /**
         * \brief Constructor for the SetJpegQuality command.
         * \param jpegQuality The desired JPEG quality value to set.
         */
        explicit SetJpegQuality(int jpegQuality)
            : m_JpegQuality(jpegQuality)
        {
        }

        /**
         * \brief Executes the command to set the JPEG quality.
         * \param command The `SetJpegQuality` instance containing the desired quality value.
         * \param jpegWriterState Pointer to the `JpegWriterState` where the quality will be updated.
         */
        static void Execute(const SetJpegQuality &command, JpegWriterState *jpegWriterState)
        {
            auto updater = JpegWriterState::Updater(jpegWriterState);
            updater.SetQuality(command.m_JpegQuality);
        }
    };
}
