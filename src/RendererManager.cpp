
#include "RendererManager.h"

#include "RendererSvg.h"
#include "RendererJson.h"
#include "RendererCairo.h"


namespace httpgd
{
    const RendererManager RendererManager::generate_default() 
    {
        RendererManager manager;

        manager.add({
          "svg",
          "image/svg+xml",
          ".svg",
          "SVG",
          "plot",
          []() { return std::make_unique<dc::RendererSVG>(boost::none); }
        });
        
#ifndef HTTPGD_NO_CAIRO
        
        manager.add({
          "png",
          "image/png",
          ".png",
          "PNG",
          "plot",
          []() { return std::make_unique<dc::RendererCairoPng>(); }
        });
        
        manager.add({
          "pdf",
          "application/pdf",
          ".pdf",
          "PDF",
          "plot",
          []() { return std::make_unique<dc::RendererCairoPdf>(); }
        });
        
#endif
        
        manager.add({
          "json",
          "application/json",
          ".json",
          "JSON",
          "plot",
          []() { return std::make_unique<dc::RendererJSON>(); }
        });

        return manager;
    }

    const std::unordered_map<std::string, StringRendererInfo>& RendererManager::string_renderers() const
    {
        return m_string_renderers;
    }
    
    const std::unordered_map<std::string, BinaryRendererInfo>& RendererManager::binary_renderers() const
    {
        return m_binary_renderers;
    }
    
    void RendererManager::add(const StringRendererInfo &renderer) 
    {
        m_string_renderers[renderer.id] = renderer;
    }
    
    void RendererManager::add(const BinaryRendererInfo &renderer) 
    {
        m_binary_renderers[renderer.id] = renderer;
    }
    
    boost::optional<StringRendererInfo &> RendererManager::find_string(const std::string &id) 
    {
        auto it = m_string_renderers.find(id);
        if (it != m_string_renderers.end())
        {
            return it->second;
        }
        return boost::none;
    }
    
    boost::optional<BinaryRendererInfo &> RendererManager::find_binary(const std::string &id) 
    {
        auto it = m_binary_renderers.find(id);
        if (it != m_binary_renderers.end())
        {
            return it->second;
        }
        return boost::none;
    }
    
    std::size_t RendererManager::size() const
    {
        return m_string_renderers.size() + m_binary_renderers.size();
    }
    

} // namespace httpgd