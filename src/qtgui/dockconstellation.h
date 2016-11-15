#ifndef DOCKCONSTELLATION_H
#define DOCKCONSTELLATION_H

#include <QDockWidget>
#include <gnuradio/gr_complex.h>

namespace Ui {
class DockConstellation;
}

class DockConstellation : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockConstellation(QWidget *parent = 0);
    ~DockConstellation();
    void setNewSymbolData(gr_complex *data, int length);

signals:
    void pskTypeChange(int n);
    void symbolRateChange(float symbolRate);
    void pllAlphaChange(float pllAlpha);
    void clockAlphaChange(float clockAlpha);
    void rrcAlphaChange(float rrcAlpha);

private slots:
    void onPSKTypeSelectedIndexChange(int index);
    void onSymbolRateUpdate();
    void onPllAlphaUpdate();
    void onClockAlphaUpdate();
    void onRrcAlphaUpdate();

private:
    Ui::DockConstellation *ui;
    static QStringList PSKTypes;

};

#endif // DOCKCONSTELLATION_H
