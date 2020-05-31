#ifndef HTTPGD_PLOT_HISTORY_H
#define HTTPGD_PLOT_HISTORY_H

#include <Rcpp.h>
#include <string>

namespace httpgd
{
    class PlotHistory
    {

    public:
        PlotHistory(const std::string &t_rvname);
        ~PlotHistory();

        void set(int index, SEXP snap);
        bool set_current(int index, pDevDesc dd);
        void set_last(int index, pDevDesc dd);
        bool get(int index, SEXP *snapshot);

        void clear();
        bool play(int index, pDevDesc dd);

    private:
        std::string m_rvname;
        Rcpp::List m_vdl;

        void m_write_data() const;
        bool m_read_data();
        void m_remove_data() const;
        void m_empty(int size);
        void m_grow(int target_size);
    };

} // namespace httpgd
#endif