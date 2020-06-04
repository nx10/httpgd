// [[Rcpp::plugins("cpp11")]]

#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#include <string>

#include "PlotHistory.h"

namespace httpgd
{
    const int GROW_STEP = 4;

    void PlotHistory::m_write_data() const
    {
        Rcpp::Environment::global_env()[m_rvname] = m_vdl;
    }
    bool PlotHistory::m_read_data()
    {
        const Rcpp::Environment &e = Rcpp::Environment::global_env();
        if (!e.exists(m_rvname))
        {
            return false;
        }
        else
        {
            m_vdl = e[m_rvname];
            return true;
        }
    }
    void PlotHistory::m_remove_data() const
    {
        Rcpp::Environment e = Rcpp::Environment::global_env();
        if (e.exists(m_rvname))
        {
            e.remove(m_rvname);
        }
    }
    void PlotHistory::m_empty(int size)
    {
        m_vdl = Rcpp::List(size, R_NilValue);
    }

    void PlotHistory::m_grow(int new_size)
    {
        Rcpp::List new_vdl = Rcpp::List(new_size, R_NilValue);
        for (int i = 0; i < m_vdl.size(); i++)
        {
            new_vdl[i] = m_vdl[i];
        }
        m_vdl = new_vdl;
    }

    PlotHistory::PlotHistory(const std::string &t_rvname)
        : m_rvname(t_rvname)
    {
    }
    PlotHistory::~PlotHistory()
    {
        m_remove_data();
    }
    void PlotHistory::set(int index, SEXP snap)
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
    bool PlotHistory::set_current(int index, pDevDesc dd)
    {
        pGEDevDesc gdd = desc2GEDesc(dd);
        if (gdd->displayList != R_NilValue)
        {
            set(index, GEcreateSnapshot(gdd));
            return true;
        }
        return false;
    }
    void PlotHistory::set_last(int index, pDevDesc dd)
    {
        set(index, desc2GEDesc(dd)->savedSnapshot);
    }
    void PlotHistory::clear()
    {
        m_remove_data();
    }
    bool PlotHistory::play(int index, pDevDesc dd)
    {
        SEXP snap = nullptr;
        if (get(index, &snap))
        {
            GEplaySnapshot(snap, desc2GEDesc(dd));
            return true;
        }
        return false;
    }
    bool PlotHistory::get(int index, SEXP *snapshot)
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

    bool PlotHistory::remove(int index)
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
            m_vdl[i] = m_vdl[i + 1];
        }
        m_write_data();
        return true;
    }

} // namespace httpgd