

#include <cpp11/R.hpp>
#include <cpp11/protect.hpp>
#define R_NO_REMAP
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>
#include <string>

#include "PlotHistory.h"

namespace httpgd
{
    PlotHistory::PlotHistory()
        : m_items()
    {
    }
    void PlotHistory::put(R_xlen_t t_index, SEXP t_snapshot)
    {
        if (m_items.size() <= t_index)
        {
            m_items.resize(t_index + 1);
        }
        m_items[t_index] = t_snapshot;
    }
    bool PlotHistory::put_current(R_xlen_t t_index, pDevDesc dd)
    {
        pGEDevDesc gdd = desc2GEDesc(dd);
        if (gdd->displayList != R_NilValue)
        {
            put(t_index, cpp11::safe[GEcreateSnapshot](gdd));
            return true;
        }
        return false;
    }
    void PlotHistory::put_last(R_xlen_t t_index, pDevDesc dd)
    {
        put(t_index, desc2GEDesc(dd)->savedSnapshot);
    }
    void PlotHistory::clear()
    {
        m_items.clear();
    }
    bool PlotHistory::play(R_xlen_t t_index, pDevDesc dd)
    {
        SEXP snap = R_NilValue;
        if (get(t_index, &snap))
        {
            cpp11::safe[GEplaySnapshot](snap, desc2GEDesc(dd));
            return true;
        }
        return false;
    }
    bool PlotHistory::get(R_xlen_t t_index, SEXP *t_snapshot)
    {
        if (m_items.size() <= t_index)
        {
            *t_snapshot = R_NilValue;
            return false;
        }
        *t_snapshot = m_items[t_index];
        return true;
    }

    bool PlotHistory::remove(R_xlen_t t_index)
    {
        if (m_items.size() <= t_index)
        {
            return false;
        }
        m_items.erase(t_index);
        return true;
    }

} // namespace httpgd