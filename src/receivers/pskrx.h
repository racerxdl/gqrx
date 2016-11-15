#ifndef PSKRX_H
#define PSKRX_H
#include <gnuradio/hier_block2.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/digital/costas_loop_cc.h>
#include <gnuradio/digital/clock_recovery_mm_cc.h>
#include <gnuradio/filter/fir_filter_ccf.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/analog/agc_cc.h>

#include <boost/circular_buffer.hpp>
#include "receiver_base.h"
#include "dsp/rx_meter.h"

class pskrx;
class SymbolBuffer;

typedef boost::shared_ptr<pskrx> pskrx_sptr;
typedef boost::shared_ptr<SymbolBuffer> SymbolBuffer_sptr;

pskrx_sptr make_pskrx(float quad_rate);
SymbolBuffer_sptr make_SymbolBuffer(int length);

class SymbolBuffer: public gr::sync_block
{
    friend SymbolBuffer_sptr make_SymbolBuffer(int length);

protected:
    SymbolBuffer(int length);
public:

    int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &);

    int get_constellation_symbols(gr_complex *data, int length);

    void clear() { this->buffer.clear(); }

private:
    boost::circular_buffer<gr_complex> buffer;
};


class pskrx : public receiver_base_cf
{
    friend pskrx_sptr make_pskrx(float quad_rate);
protected:
    pskrx(float quad_rate);

public:

    bool start();
    bool stop();
    void set_quad_rate(float quad_rate);
    void set_audio_rate(float audio_rate);
    void set_filter(double low, double high, double tw);
    void set_cw_offset(double offset);
    float get_signal_level(bool dbfs);
    void set_demod(int demod);

    bool has_nb() { return false; }
    bool has_sql() { return false; }
    bool has_agc() { return false; }
    bool has_fm() { return false; }
    bool has_am() { return false; }

    /* PSK parameters */
    bool has_psk() { return true; }
    void set_symbol_rate(float symbolrate);
    void set_psk_order(int n);
    void set_pll_alpha(float pllalpha);
    void set_clock_alpha(float clockalpha);
    void set_rrc_alpha(float rrcalpha);
    int get_constelation_symbols(gr_complex *data, int length);

private:
    gr::digital::costas_loop_cc::sptr costasLoop;
    gr::digital::clock_recovery_mm_cc::sptr mmcr;
    gr::blocks::null_sink::sptr nullSink;
    gr::blocks::null_source::sptr nullSource;
    gr::filter::fir_filter_ccf::sptr rrcFilter;
    gr::analog::agc_cc::sptr agc;
    rx_meter_c_sptr meter;

    SymbolBuffer_sptr symbolBuffer;
    float pllAlpha;
    int loopOrder;
    float symbolRate;
    float clockAlpha;
    float rrcAlpha;
    float quadRate;

    void initialize();
};

#endif // PSKRX_H
