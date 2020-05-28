#ifndef HTTPGD_PLOT_HISTORY_H
#define HTTPGD_PLOT_HISTORY_H

#include <Rcpp.h>
#include <string>

namespace httpgd
{
    class PlotHistory
    {

    public:
        PlotHistory(int t_magic, int t_grow, const std::string &t_rvname);
        ~PlotHistory();

        void push(SEXP snap);
        bool push_current(pDevDesc dd);
        void push_last(pDevDesc dd);
        bool get(int index, SEXP &snapshot);
        void clear();
        bool play(int index, pDevDesc dd);
        int size();

    private:
        int m_magic;
        int m_grow;
        std::string m_rvname;
        Rcpp::List m_vdl;

        int m_r_get_magic() const;
        // number of saved plots
        int m_r_get_size() const;
        void m_r_set_size(const int &n);
        // capacity
        int m_r_get_capacity() const;
        void m_r_set_capacity(const int &capacity);
        // list of snapshots
        Rcpp::List m_r_get_snaps() const;
        void m_r_set_snaps(const Rcpp::List &snaps);

        void m_r_write();
        bool m_r_read();
        void m_r_remove();
        void m_r_empty();
        void m_r_grow();
        void m_r_add_snap(const SEXP &snap);
    };

} // namespace httpgd
#endif