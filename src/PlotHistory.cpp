// [[Rcpp::plugins("cpp11")]]

#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#include <string>

#include "PlotHistory.h"

const int PlotHistory::get_magic()
{
    return m_vdl[0];
}
// number of saved plots
const int PlotHistory::get_size()
{
    return m_vdl[1];
}
void PlotHistory::set_size(const int &n)
{
    m_vdl[1] = n;
}
// capacity
const int PlotHistory::get_capacity()
{
    return m_vdl[2];
}
void PlotHistory::set_capacity(const int &capacity)
{
    m_vdl[2] = capacity;
}
// list of snapshots
Rcpp::List PlotHistory::get_snaps()
{
    return m_vdl[3];
}
void PlotHistory::set_snaps(const Rcpp::List &snaps)
{
    m_vdl[3] = snaps;
}

void PlotHistory::write()
{
    Rcpp::Environment::global_env()[m_rvname] = m_vdl;
}
bool PlotHistory::read()
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
void PlotHistory::remove()
{
    Rcpp::Environment e = Rcpp::Environment::global_env();
    if (e.exists(m_rvname)) {
        e.remove(m_rvname);
    }
}
void PlotHistory::empty()
{
    m_vdl = Rcpp::List::create(
        m_magic,
        0,      // number of plots
        m_grow, // max. number of plots
        Rcpp::List(m_grow, R_NilValue));
}
void PlotHistory::grow()
{
    const int new_capacity = get_capacity() + m_grow;
    Rcpp::List new_snaps = Rcpp::List(new_capacity, R_NilValue);
    for (int i = 0; i < get_capacity(); i++)
    {
        new_snaps[i] = get_snaps()[i];
    }
    set_capacity(new_capacity);
    set_snaps(new_snaps);
}
void PlotHistory::add_snap(const SEXP &snap)
{
    int n = get_size();
    if (n == get_capacity())
    {
        grow();
    }
    get_snaps()[n] = snap;
    set_size(n + 1);
}

PlotHistory::PlotHistory(const int &magic, const int &grow, const std::string &rvname)
    : m_magic(magic), m_grow(grow), m_rvname(rvname)
{
}
PlotHistory::~PlotHistory()
{
    remove();
}
void PlotHistory::push(SEXP snap) 
{
    if (!read())
    {
        empty();
        write();
    }
    add_snap(snap);
    write();
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
    if (!read() || index < 0 || index >= get_size())
    {
        return false;
    }
    snapshot = get_snaps()[index];
    return true;
}
void PlotHistory::clear()
{
    remove();
}
bool PlotHistory::play(int index, pDevDesc dd)
{
    SEXP snap;
    if (get(index, snap)) {
        GEplaySnapshot(snap, desc2GEDesc(dd));
        return true;
    }
    return false;
}
int PlotHistory::size()
{
    return read() ? get_size() : 0;
}

