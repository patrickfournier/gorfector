#pragma once

#include <filesystem>

namespace ZooLib
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

    /**
     * \brief Relocates the path to the installation directory.
     *
     * \param path The path to be relocated.
     * \return The relocated path.
     */
    std::filesystem::path RelocatePath(const std::filesystem::path &path);

    /**
     * \brief Unrelocates the path to the root directory (/).
     *
     * \param path The path to be unrelocated.
     * \return The unrelocated path.
     */
    std::filesystem::path UnrelocatePath(const std::filesystem::path &path);
}
