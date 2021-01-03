#ifndef HTTPGD_PLOT_HISTORY_H
#define HTTPGD_PLOT_HISTORY_H

#include <cpp11/list.hpp>
#define R_NO_REMAP
#include <R_ext/GraphicsEngine.h>
#include <string>

namespace httpgd
{
    class PlotHistory
    {
    public:
        // Replay the current graphics device state
        static bool replay_current(pDevDesc dd);

        PlotHistory();

        void put(R_xlen_t index, SEXP snapshot);
        bool put_current(R_xlen_t index, pDevDesc dd);
        void put_last(R_xlen_t index, pDevDesc dd);
        bool get(R_xlen_t index, SEXP *snapshot);

        bool remove(R_xlen_t index);

        void clear();
        bool play(R_xlen_t index, pDevDesc dd);

    private:
        cpp11::writable::list m_items;
    };

} // namespace httpgd
#endif