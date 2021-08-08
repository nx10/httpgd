#include "RendererMeta.h"

namespace httpgd::dc
{
    void RendererMeta::render(const Page &t_page, double t_scale)
    {
        m_scale = t_scale;
        page(t_page);
    }

    std::string RendererMeta::get_string() const
    {
        return fmt::to_string(os);
    }

    void RendererMeta::page(const Page &t_page)
    {
        fmt::format_to(os, "{{\n " R""("id": "{}", "w": {:.2f}, "h": {:.2f}, "scale": {:.2f}, clips: {}, draw_calls: {})"" "\n}}",
        t_page.id, t_page.size.x, t_page.size.y, m_scale, t_page.cps.size(), t_page.dcs.size());
    }
    
} // namespace httpgd::dc
