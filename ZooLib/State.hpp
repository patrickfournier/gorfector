#pragma once

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <filesystem>
#include <fstream>
#include <vector>

#include "json/single_include/nlohmann/json.hpp"

namespace ZooLib
{
    class StateComponent;

    /**
     * \brief Manages the state of the application, which is partitioned into components.
     *
     * The `State` class provides functionality to manage a collection of state components,
     * serialize/deserialize their state to/from a preferences file, and retrieve components by type.
     */
    class State
    {
        std::filesystem::path m_PreferencesFilePath{};
        std::vector<StateComponent *> m_StateComponents;

        TEST_FRIENDS;

    public:
        ~State();

        /**
         * \brief Adds a state component to the collection if it is not already present.
         * \param stateComponent A pointer to the state component to add.
         */
        void AddStateComponent(StateComponent *stateComponent)
        {
            if (std::ranges::find(m_StateComponents, stateComponent) == m_StateComponents.end())
            {
                m_StateComponents.push_back(stateComponent);
            }
        }

        /**
         * \brief Removes a state component from the collection if it exists.
         * \param stateComponent A pointer to the state component to remove.
         */
        void RemoveStateComponent(StateComponent *stateComponent)
        {
            if (const auto it = std::ranges::find(m_StateComponents, stateComponent); it != m_StateComponents.end())
            {
                m_StateComponents.erase(it);
            }
        }

        /**
         * \brief Retrieves a state component by its type.
         * \tparam TStateComponent The type of the state component to retrieve.
         * \return A pointer to the state component if found, otherwise nullptr.
         */
        template<typename TStateComponent>
        TStateComponent *GetStateComponentByType() const
        {
            for (const auto &stateComponent: m_StateComponents)
            {
                auto sc = dynamic_cast<TStateComponent *>(stateComponent);
                if (sc != nullptr)
                {
                    return sc;
                }
            }

            return nullptr;
        }

        /**
         * \brief Sets the file path for the preferences file. This file is used to serialize the components.
         * \param filePath The file path to set.
         */
        void SetPreferencesFilePath(const std::filesystem::path &filePath)
        {
            m_PreferencesFilePath = filePath;
        }

        /**
         * \brief Loads the state of a component from the preferences file.
         * \tparam TStateComponent The type of the state component to load.
         * \param stateComponent A pointer to the state component to load.
         */
        template<typename TStateComponent>
        void LoadFromPreferencesFile(TStateComponent *stateComponent)
        {
            if (stateComponent == nullptr)
            {
                return;
            }

            const std::string &key = stateComponent->GetSerializationKey();
            if (key.empty())
            {
                return;
            }

            std::ifstream f(m_PreferencesFilePath);
            if (!f.good())
            {
                return;
            }

            try
            {
                if (auto jsonData = nlohmann::json::parse(f); jsonData.contains(key))
                {
                    auto updater = typename TStateComponent::Updater(stateComponent);
                    updater.LoadFromJson(jsonData[key]);
                }
            }
            catch (const std::exception &)
            {
            }

            f.close();
        }

        /**
         * \brief Saves the state of a component to the preferences file.
         * \tparam TStateComponent The type of the state component to save.
         * \param stateComponent A pointer to the state component to save.
         */
        template<typename TStateComponent>
        void SaveToFile(TStateComponent *stateComponent)
        {
            if (stateComponent == nullptr)
            {
                return;
            }

            const std::string &key = stateComponent->GetSerializationKey();
            if (key.empty())
            {
                return;
            }

            std::ifstream f(m_PreferencesFilePath);
            nlohmann::json jsonData;
            if (f.good())
            {
                try
                {
                    jsonData = nlohmann::json::parse(f);
                }
                catch (const std::exception &)
                {
                }

                f.close();
            }

            nlohmann::json componentJson;
            to_json(componentJson, *stateComponent);
            jsonData[key] = componentJson;

            auto dirs = m_PreferencesFilePath.parent_path();
            create_directories(dirs);

            std::ofstream outFile(m_PreferencesFilePath);
            outFile << jsonData.dump(4);
            outFile.close();
        }
    };
}
