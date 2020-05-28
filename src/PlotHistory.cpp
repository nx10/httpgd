// [[Rcpp::plugins("cpp11")]]

#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#include <string>

#include "PlotHistory.h"

namespace httpgd
{

    int PlotHistory::m_r_get_magic() const
    {
        return m_vdl[0];
    }
    // number of saved plots
    int PlotHistory::m_r_get_size() const
    {
        return m_vdl[1];
    }
    void PlotHistory::m_r_set_size(const int &n)
    {
        m_vdl[1] = n;
    }
    // capacity
    int PlotHistory::m_r_get_capacity() const
    {
        return m_vdl[2];
    }
    void PlotHistory::m_r_set_capacity(const int &capacity)
    {
        m_vdl[2] = capacity;
    }
    // list of snapshots
    Rcpp::List PlotHistory::m_r_get_snaps() const
    {
        return m_vdl[3];
    }
    void PlotHistory::m_r_set_snaps(const Rcpp::List &snaps)
    {
        m_vdl[3] = snaps;
    }

    void PlotHistory::m_r_write()
    {
        Rcpp::Environment::global_env()[m_rvname] = m_vdl;
    }
    bool PlotHistory::m_r_read()
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
    void PlotHistory::m_r_remove()
    {
        Rcpp::Environment e = Rcpp::Environment::global_env();
        if (e.exists(m_rvname))
        {
            e.remove(m_rvname);
        }
    }
    void PlotHistory::m_r_empty()
    {
        m_vdl = Rcpp::List::create(
            m_magic,
            0,      // number of plots
            m_grow, // max. number of plots
            Rcpp::List(m_grow, R_NilValue));
    }
    void PlotHistory::m_r_grow()
    {
        const int new_capacity = m_r_get_capacity() + m_grow;
        Rcpp::List new_snaps = Rcpp::List(new_capacity, R_NilValue);
        for (int i = 0; i < m_r_get_capacity(); i++)
        {
            new_snaps[i] = m_r_get_snaps()[i];
        }
        m_r_set_capacity(new_capacity);
        m_r_set_snaps(new_snaps);
    }
    void PlotHistory::m_r_add_snap(const SEXP &snap)
    {
        int n = m_r_get_size();
        if (n == m_r_get_capacity())
        {
            m_r_grow();
        }
        m_r_get_snaps()[n] = snap;
        m_r_set_size(n + 1);
    }

    PlotHistory::PlotHistory(int t_magic, int t_grow, const std::string &t_rvname)
        : m_magic(t_magic), m_grow(t_grow), m_rvname(t_rvname)
    {
    }
    PlotHistory::~PlotHistory()
    {
        m_r_remove();
    }
    void PlotHistory::push(SEXP snap)
    {
        if (!m_r_read())
        {
            m_r_empty();
            m_r_write();
        }
        m_r_add_snap(snap);
        m_r_write();
    }
    bool PlotHistory::push_current(pDevDesc dd)
    {
        pGEDevDesc gdd = desc2GEDesc(dd);
        if (gdd->displayList != R_NilValue)
        {
            push(GEcreateSnapshot(gdd));
            return true;
        }
        return false;
    }
    void PlotHistory::push_last(pDevDesc dd)
    {
        push(desc2GEDesc(dd)->savedSnapshot);
    }
    bool PlotHistory::get(int index, SEXP &snapshot)
    {
        if (!m_r_read() || index < 0 || index >= m_r_get_size())
        {
            return false;
        }
        snapshot = m_r_get_snaps()[index];
        return true;
    }
    void PlotHistory::clear()
    {
        m_r_remove();
    }
    bool PlotHistory::play(int index, pDevDesc dd)
    {
        SEXP snap = nullptr;
        if (get(index, snap))
        {
            GEplaySnapshot(snap, desc2GEDesc(dd));
            return true;
        }
        return false;
    }
    int PlotHistory::size()
    {
        return m_r_read() ? m_r_get_size() : 0;
    }

} // namespace httpgd