#include <Rcpp.h>
#include <string>

class PlotHistory
{
    int m_magic;
    int m_grow;
    std::string m_rvname;
    Rcpp::List m_vdl;

    const int get_magic();
    // number of saved plots
    const int get_size();
    void set_size(const int &n);
    // capacity
    const int get_capacity();
    void set_capacity(const int &capacity);
    // list of snapshots
    Rcpp::List get_snaps();
    void set_snaps(const Rcpp::List &snaps);

    void write();
    bool read();
    void remove();
    void empty();
    void grow();
    void add_snap(const SEXP &snap);

public:
    PlotHistory(const int &magic, const int &grow, const std::string &rvname);
    ~PlotHistory();
    void push(SEXP snap);
    bool push_current(pDevDesc dd);
    void push_last(pDevDesc dd);
    bool get(int index, SEXP &snapshot);
    void clear();
    bool play(int index, pDevDesc dd);
    int size();
};