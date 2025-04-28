#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class PngWriterState : public ZooLib::StateComponent
    {
        int m_CompressionLevel{};

        friend void to_json(nlohmann::json &j, const PngWriterState &p);
        friend void from_json(const nlohmann::json &j, PngWriterState &p);

    public:
        explicit PngWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_CompressionLevel(1)
        {
        }

        ~PngWriterState() override
        {
            m_State->SaveToFile(this);
        }

        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "PngWriterState";
        }

        [[nodiscard]] int GetCompressionLevel() const
        {
            return m_CompressionLevel;
        }

        class Updater final : public StateComponent::Updater<PngWriterState>
        {
        public:
            explicit Updater(PngWriterState *state)
                : StateComponent::Updater<PngWriterState>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
            }

            void SetCompressionLevel(int compressionLevel)
            {
                m_StateComponent->m_CompressionLevel = compressionLevel;
            }
        };
    };

    inline void to_json(nlohmann::json &j, const PngWriterState &p)
    {
        j = nlohmann::json{{"CompressionLevel", p.m_CompressionLevel}};
    }

    inline void from_json(const nlohmann::json &j, PngWriterState &p)
    {
        j.at("CompressionLevel").get_to(p.m_CompressionLevel);
    }
}
