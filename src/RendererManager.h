#ifndef RENDERER_MANAGER_H
#define RENDERER_MANAGER_H

#include "DrawData.h"

#include <boost/optional.hpp>
#include <functional>
#include <string>
#include <unordered_map>

namespace httpgd
{
    template <typename T>
    struct RendererManagerInfo
    {
        std::string id;
        std::string mime;
        std::string fileext;
        std::string name;
        std::string type;
        std::function<std::unique_ptr<T>()> renderer;
    };
    
    using StringRendererInfo = RendererManagerInfo<dc::StringRenderingTarget>;
    using BinaryRendererInfo = RendererManagerInfo<dc::BinaryRenderingTarget>;

    class RendererManager
    {
    public:
        static const RendererManager generate_default();

        [[nodiscard]] const std::unordered_map<std::string, StringRendererInfo> &string_renderers() const;
        [[nodiscard]] const std::unordered_map<std::string, BinaryRendererInfo> &binary_renderers() const;

        void add(const StringRendererInfo &renderer);
        void add(const BinaryRendererInfo &renderer);
        
        boost::optional<StringRendererInfo &> find_string(const std::string &id);
        boost::optional<BinaryRendererInfo &> find_binary(const std::string &id);

    private:
        std::unordered_map<std::string, StringRendererInfo> m_string_renderers;
        std::unordered_map<std::string, BinaryRendererInfo> m_binary_renderers;
    };

} // namespace httpgd

#endif // RENDERER_MANAGER_H