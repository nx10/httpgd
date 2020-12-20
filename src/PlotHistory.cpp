
#include <cpp11/environment.hpp>
#include <cpp11/function.hpp>
#include <cpp11/list.hpp>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#include <string>

#include "PlotHistory.h"

namespace httpgd
{
    const int GROW_STEP = 4;

    void PlotHistory::m_write_data() const
    {
        m_env[m_rvname] = m_vdl;
    }
    bool PlotHistory::m_read_data()
    {
        if (!m_env.exists(m_rvname.c_str()))
        {
            return false;
        }
        else
        {
            m_vdl = cpp11::writable::list(m_env[m_rvname]);
            return true;
        }
    }
    void PlotHistory::m_remove_data() const
    {
        if (m_env.exists(m_rvname.c_str()))
        {
            //TODO m_env.remove(m_rvname.c_str());
        }
    }
    void PlotHistory::m_empty(R_xlen_t size)
    {
        m_vdl = cpp11::writable::list(size);
    }

    void PlotHistory::m_grow(R_xlen_t new_size)
    {
        auto new_vdl = cpp11::writable::list(new_size);
        for (int i = 0; i < m_vdl.size(); i++)
        {
            new_vdl.insert(i, m_vdl[i]);
        }
        m_vdl = new_vdl;
    }

    PlotHistory::PlotHistory(const std::string &t_rvname, const cpp11::environment &t_env)
        : m_rvname(t_rvname), m_env(t_env)
    {
    }
    PlotHistory::~PlotHistory()
    {
        m_remove_data();
    }
    void PlotHistory::set(R_xlen_t index, SEXP snap)
    {
        if (!m_read_data())
        {
            m_empty(index + GROW_STEP - (index % GROW_STEP));
        }
        else if (index >= m_vdl.size())
        {
            m_grow(index + GROW_STEP - (index % GROW_STEP));
        }
        m_vdl[index] = snap;
        m_write_data();
    }
    bool PlotHistory::set_current(R_xlen_t index, pDevDesc dd)
    {
        pGEDevDesc gdd = desc2GEDesc(dd);
        if (gdd->displayList != R_NilValue)
        {
            set(index, GEcreateSnapshot(gdd));
            return true;
        }
        return false;
    }
    void PlotHistory::set_last(R_xlen_t index, pDevDesc dd)
    {
        set(index, desc2GEDesc(dd)->savedSnapshot);
    }
    void PlotHistory::clear()
    {
        m_remove_data();
    }
    bool PlotHistory::play(R_xlen_t index, pDevDesc dd)
    {
        SEXP snap = nullptr;
        if (get(index, &snap))
        {
            GEplaySnapshot(snap, desc2GEDesc(dd));
            return true;
        }
        return false;
    }
    bool PlotHistory::get(R_xlen_t index, SEXP *snapshot)
    {
        if (!m_read_data())
        {
            return false;
        }
        else if (index >= m_vdl.size())
        {
            return false;
        }
        *snapshot = m_vdl[index];
        return true;
    }

    bool PlotHistory::remove(R_xlen_t index)
    {
        if (!m_read_data())
        {
            return false;
        }
        else if (index >= m_vdl.size())
        {
            return false;
        }
        m_vdl[index] = R_NilValue;
        for (int i = index; i < m_vdl.size() - 1; i++)
        {
            m_vdl.insert(i, m_vdl[i + 1]);
        }
        m_vdl[m_vdl.size() - 1] = R_NilValue;
        m_write_data();
        return true;
    }

} // namespace httpgd