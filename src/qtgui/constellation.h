#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include <QtGui>
#include <QFont>
#include <QFrame>
#include <QImage>
#include <vector>
#include <QMap>
#include <gnuradio/gr_complex.h>


class CConstellation : public QFrame
{
    Q_OBJECT

public:
    explicit CConstellation(QWidget *parent = 0);
    ~CConstellation();

    void draw();
    void setNewSymbolData(gr_complex *symbols, int size);
    void setScale(float x, float y);
    void setMode(int n);

public slots:
    void updateOverlay();

protected:
    //re-implemented widget event handlers
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);

private:
    QPixmap m_OverlayPixmap;
    QPixmap m_ConstellationPixmap;
    gr_complex *m_symbols;
    int m_symbolLength;
    int mode;
    QSize       m_Size;
    float m_scaleX, m_scaleY;
};

#endif // CONSTELLATION_H
