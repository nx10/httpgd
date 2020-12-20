#ifndef HTTPGD_PLOT_HISTORY_H
#define HTTPGD_PLOT_HISTORY_H

#include <cpp11/R.hpp>
#include <cpp11/list.hpp>
#include <cpp11/environment.hpp>
#include <string>

namespace httpgd
{
    class PlotHistory
    {

    public:
        PlotHistory(const std::string &t_rvname, const cpp11::environment &t_env);
        ~PlotHistory();

        void set(R_xlen_t index, SEXP snap);
        bool set_current(R_xlen_t index, pDevDesc dd);
        void set_last(R_xlen_t index, pDevDesc dd);
        bool get(R_xlen_t index, SEXP *snapshot);

        bool remove(R_xlen_t index);

        void clear();
        bool play(R_xlen_t index, pDevDesc dd);

    private:
        std::string m_rvname;
        cpp11::writable::list m_vdl;
        cpp11::environment m_env;

        void m_write_data() const;
        bool m_read_data();
        void m_remove_data() const;
        void m_empty(R_xlen_t size);
        void m_grow(R_xlen_t target_size);
    };

} // namespace httpgd
#endif