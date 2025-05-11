#pragma once

#include <filesystem>

namespace Gorfector
{
    /**
     * \brief Increments the path by appending a counter to the filename.
     *
     * This function modifies the given path by appending a counter to the filename
     * if a file with the same name already exists. The counter is incremented until
     * a unique filename is found.
     *
     * \param path The path to be modified.
     */
    void IncrementPath(std::filesystem::path &path);
}
