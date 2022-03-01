
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
          []() { return std::make_unique<dc::RendererSVG>(boost::none); },
          "Scalable Vector Graphics (SVG)."
        });
        
        manager.add(BinaryRendererInfo{
          "svgz",
          "image/svg+xml",
          ".svgz",
          "SVGZ",
          "plot",
          []() { return std::make_unique<dc::RendererSVGZ>(boost::none); },
          "Compressed Scalable Vector Graphics (SVGZ)."
        });
        
        manager.add({
          "svgp",
          "image/svg+xml",
          ".svg",
          "Portable SVG",
          "plot",
          []() { return std::make_unique<dc::RendererSVGPortable>(); },
          "Version of the SVG renderer that produces portable SVGs."
        });
        
        manager.add(BinaryRendererInfo{
          "svgzp",
          "image/svg+xml",
          ".svgz",
          "Portable SVGZ",
          "plot",
          []() { return std::make_unique<dc::RendererSVGZPortable>(); },
          "Version of the SVG renderer that produces portable SVGZs."
        });
        
#ifndef HTTPGD_NO_CAIRO
        
        manager.add({
          "png",
          "image/png",
          ".png",
          "PNG",
          "plot",
          []() { return std::make_unique<dc::RendererCairoPng>(); },
          "Portable Network Graphics (PNG)."
        });
        
        manager.add({
          "pdf",
          "application/pdf",
          ".pdf",
          "PDF",
          "plot",
          []() { return std::make_unique<dc::RendererCairoPdf>(); },
          "Adobe Portable Document Format (PDF)."
        });
        
        manager.add({
          "ps",
          "application/postscript",
          ".ps",
          "PS",
          "plot",
          []() { return std::make_unique<dc::RendererCairoPs>(); },
          "PostScript (PS)."
        });

        manager.add({
          "eps",
          "application/postscript",
          ".eps",
          "EPS",
          "plot",
          []() { return std::make_unique<dc::RendererCairoEps>(); },
          "Encapsulated PostScript (EPS)."
        });
        
        manager.add({
          "tiff",
          "image/tiff",
          ".tiff",
          "TIFF",
          "plot",
          []() { return std::make_unique<dc::RendererCairoTiff>(); },
          "Tagged Image File Format (TIFF)."
        });
        
#endif
        
        manager.add({
          "json",
          "application/json",
          ".json",
          "JSON",
          "plot",
          []() { return std::make_unique<dc::RendererJSON>(); },
          "Plot data serialized to JSON format."
        });
        
        manager.add({
          "tikz",
          "text/plain",
          ".tex",
          "TikZ",
          "plot",
          []() { return std::make_unique<dc::RendererTikZ>(); },
          "LaTeX TikZ code."
        });
        
        manager.add({
          "strings",
          "text/plain",
          ".txt",
          "Strings",
          "data",
          []() { return std::make_unique<dc::RendererStrings>(); },
          "List of strings contained in plot."
        });
        
        manager.add({
          "meta",
          "application/json",
          ".json",
          "Meta",
          "data",
          []() { return std::make_unique<dc::RendererMeta>(); },
          "Plot meta information."
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