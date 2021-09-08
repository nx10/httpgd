
#include "RendererManager.h"

#include "RendererSvg.h"
#include "RendererJson.h"
#include "RendererCairo.h"
#include "RendererTikZ.h"
#include "RendererStrings.h"
#include "RendererMeta.h"


namespace httpgd
{
    const RendererManager RendererManager::defaults_ = RendererManager::generate_default();

    const RendererManager &RendererManager::defaults() 
    {
        return RendererManager::defaults_;
    }

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
        
        manager.add({
          "ps",
          "application/postscript",
          ".ps",
          "PS",
          "plot",
          []() { return std::make_unique<dc::RendererCairoPs>(); }
        });

        manager.add({
          "eps",
          "application/postscript",
          ".eps",
          "EPS",
          "plot",
          []() { return std::make_unique<dc::RendererCairoEps>(); }
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
        
        manager.add({
          "tikz",
          "text/plain",
          ".tex",
          "TikZ",
          "plot",
          []() { return std::make_unique<dc::RendererTikZ>(); }
        });
        
        manager.add({
          "strings",
          "text/plain",
          ".txt",
          "Strings",
          "data",
          []() { return std::make_unique<dc::RendererStrings>(); }
        });
        
        manager.add({
          "meta",
          "application/json",
          ".json",
          "Meta",
          "data",
          []() { return std::make_unique<dc::RendererMeta>(); }
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
    
    boost::optional<const StringRendererInfo &> RendererManager::find_string(const std::string &id) const
    {
        auto it = m_string_renderers.find(id);
        if (it != m_string_renderers.end())
        {
            return it->second;
        }
        return boost::none;
    }
    
    boost::optional<const BinaryRendererInfo &> RendererManager::find_binary(const std::string &id) const
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