#include "pskrx.h"
#include <QDebug>

SymbolBuffer_sptr make_SymbolBuffer(int length, double period)
{
    return gnuradio::get_initial_sptr(new SymbolBuffer(length, period));
}

SymbolBuffer::SymbolBuffer(int length, double period)
    : gr::sync_block("SymbolBuffer",
        gr::io_signature::make (1, 1, sizeof (gr_complex)),
        gr::io_signature::make (0, 0, 0)),
      period(period),
      curPos(0)
{
    for (int i=0; i<length; i++) {
        this->buffer.push_back(gr_complex(0,0));
    }
    lastUpdate = std::chrono::system_clock::now();
}

int SymbolBuffer::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &)
{
    std::chrono::time_point<std::chrono::system_clock> curTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedTime = curTime - lastUpdate;
    if (elapsedTime.count() > period) {
        const gr_complex *in = (const gr_complex *) input_items[0];
        int copySize = this->buffer.size() - curPos - 1;
        if ( noutput_items < copySize) {
            copySize = noutput_items;
        }

        memcpy(&this->buffer[curPos], in, sizeof(gr_complex) * copySize);
        curPos += copySize;

        for (unsigned int i=curPos; i<this->buffer.size(); i++) {
            this->buffer[i] = gr_complex(0, 0);
        }

        curPos = 0;

        lastUpdate = std::chrono::system_clock::now();
    }

    return this->buffer.size();
}

int SymbolBuffer::get_constellation_symbols(gr_complex *data, int length)
{
    int n = (int) this->buffer.size() > length ? length : this->buffer.size();
    if (n > 0) {
        memcpy(data, &this->buffer[0], sizeof(gr_complex) * n);
    }

    return n;
}

pskrx_sptr make_pskrx(float quad_rate)
{
    return gnuradio::get_initial_sptr(new pskrx(quad_rate));
}

pskrx::pskrx(float quad_rate)
    : receiver_base_cf("PSK")
{
    this->symbolBuffer = make_SymbolBuffer(1024, 1.0 / 60);
    this->pllAlpha = 1.99e-3;
    this->loopOrder = 2;
    this->symbolRate = 293883;
    this->clockAlpha = 3.7e-3;
    this->rrcAlpha = 0.5;
    this->quadRate = quad_rate;
    this->hasFileSink = false;
    this->hasUdpSink = false;

    initialize();
}

int pskrx::get_constelation_symbols(gr_complex *data, int length)
{
    return symbolBuffer.get()->get_constellation_symbols(data, length);
}

void pskrx::initialize()
{
    float sps = quadRate / symbolRate;

    this->costasLoop = gr::digital::costas_loop_cc::make(pllAlpha, loopOrder);
    this->mmcr = gr::digital::clock_recovery_mm_cc::make(sps, (clockAlpha*clockAlpha)/4.0, 0.5, clockAlpha, 0.005);
    this->nullSink = gr::blocks::null_sink::make(sizeof(gr_complex));
    this->nullSource = gr::blocks::null_source::make(sizeof(gr_complex));
    this->rrcFilter = gr::filter::fir_filter_ccf::make(1, gr::filter::firdes::root_raised_cosine(1, quadRate, symbolRate, rrcAlpha, 361));
    this->agc = gr::analog::agc_cc::make(100e-4, 0.5, 0.5);
    this->agc->set_max_gain(4000);
    this->meter = make_rx_meter_c(DETECTOR_TYPE_RMS);
    this->complexToChar = gr::blocks::complex_to_interleaved_char::make();

    connect(self(), 0, rrcFilter, 0);
    connect(rrcFilter, 0, agc, 0);
    connect(agc, 0, costasLoop, 0);
    connect(rrcFilter, 0, meter, 0);
    connect(costasLoop, 0, mmcr, 0);
    connect(mmcr, 0, symbolBuffer, 0);
    connect(mmcr, 0, complexToChar, 0);
    connect(complexToChar, 0, nullSink, 0);

    // No Audio Output
    connect(nullSource, 0, self(), 0);
    connect(nullSource, 0, self(), 1);

    std::cout << "Initialized PSK RX with " << quadRate << " IQ Rate and PSK Mode: " << loopOrder << std::endl;
}

void pskrx::set_symbol_rate(float symbolrate) {
    this->symbolRate = symbolrate;
    float sps = quadRate / symbolrate;
    lock();
    qDebug() << "Setting symbolrate to " << symbolrate;
    this->mmcr->set_omega(sps);
    this->rrcFilter->set_taps(gr::filter::firdes::root_raised_cosine(1, quadRate, symbolRate, rrcAlpha, 361));
    unlock();
}

void pskrx::set_psk_order(int n) {
    if (n == 2 || n == 4 || n == 8) {
        lock();
        qDebug() << "Changing Costas Loop Order to " << n;
        this->loopOrder = n;
        disconnect_all();
        initialize();
        unlock();
    }
}

void pskrx::set_pll_alpha(float pllalpha) {
    this->pllAlpha = pllalpha;
    lock();
    qDebug() << "Setting Costas Loop PLL Alpha to " << pllalpha;
    this->costasLoop->set_alpha(pllAlpha);
    unlock();
}

void pskrx::set_clock_alpha(float clockalpha) {
    this->clockAlpha = clockalpha;
    lock();
    qDebug() << "Setting Clock Alpha to " << clockalpha;
    this->mmcr->set_omega((clockAlpha*clockAlpha)/4.0);
    this->mmcr->set_gain_mu(clockAlpha);
    unlock();
}

void pskrx::set_rrc_alpha(float rrcalpha) {
    this->rrcAlpha = rrcalpha;
    lock();
    qDebug() << "Setting RRC Alpha to " << rrcalpha;
    this->rrcFilter->set_taps(gr::filter::firdes::root_raised_cosine(1, quadRate, symbolRate, rrcalpha, 361));
    unlock();
}

bool pskrx::start()
{
    return true;
}

bool pskrx::stop()
{
    return true;
}

void pskrx::set_quad_rate(float quad_rate) {
    this->quadRate = quad_rate;
    lock();
    disconnect_all();
    initialize();
    unlock();
}

void pskrx::set_audio_rate(float)
{
    // Do Nothing
}

void pskrx::set_filter(double, double, double)
{
    // Do Nothing
}

void pskrx::set_cw_offset(double)
{
    // Do Nothing
}

float pskrx::get_signal_level(bool dbfs)
{
    if (dbfs)
        return meter->get_level_db();
    else
        return meter->get_level();
}

void pskrx::set_demod(int)
{
    // Do Nothing
}


void pskrx::set_file_output(std::string &output_file)
{
    lock();
    if (hasFileSink) {
        disconnect(complexToChar, 0, fileSink, 0);
    }

    fileSink = gr::blocks::file_sink::make(1, output_file.c_str());
    connect(complexToChar, 0, fileSink, 0);
    hasFileSink = true;
    unlock();
}

void pskrx::set_udp_output(std::string &hostname, int port)
{
    lock();
    if (hasUdpSink) {
        disconnect(complexToChar, 0, udpSink, 0);
    }
    udpSink = gr::blocks::udp_sink::make(1, hostname, port, 16, false);
    connect(complexToChar, 0, udpSink, 0);
    hasUdpSink = true;
    unlock();
}
