#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class JpegWriterState : public ZooLib::StateComponent
    {
        int m_Quality;

        friend void to_json(nlohmann::json &j, const JpegWriterState &p);
        friend void from_json(const nlohmann::json &j, JpegWriterState &p);

    public:
        explicit JpegWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_Quality(75)
        {
        }

        ~JpegWriterState() override
        {
            m_State->SaveToFile(this);
        }

        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "JpegWriterState";
        }

        int GetQuality() const
        {
            return m_Quality;
        }

        class Updater final : public StateComponent::Updater<JpegWriterState>
        {
        public:
            explicit Updater(JpegWriterState *state)
                : StateComponent::Updater<JpegWriterState>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
            }

            void SetQuality(int quality)
            {
                m_StateComponent->m_Quality = quality;
            }
        };
    };

    inline void to_json(nlohmann::json &j, const JpegWriterState &p)
    {
        j = nlohmann::json{{"Quality", p.m_Quality}};
    }

    inline void from_json(const nlohmann::json &j, JpegWriterState &p)
    {
        j.at("Quality").get_to(p.m_Quality);
    }
}
