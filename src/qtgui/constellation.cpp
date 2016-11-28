
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QtGlobal>
#include <QToolTip>
#include "constellation.h"

#include <iostream>

CConstellation::CConstellation(QWidget *parent): QFrame(parent)
{
    this->m_ConstellationPixmap = QPixmap(0, 0);
    this->m_OverlayPixmap = QPixmap(0, 0);
    this->m_symbols = NULL;
    this->m_symbolLength = 0;
    this->m_scaleX = 0.8;
    this->m_scaleY = 0.8;
    m_Size = QSize(0, 0);
    mode = 2;
}

void CConstellation::setScale(float x, float y)
{
    this->m_scaleX = x;
    this->m_scaleY = y;
}

CConstellation::~CConstellation()
{
    if (this->m_symbols != NULL) {
        delete this->m_symbols;
    }
}

void CConstellation::setNewSymbolData(gr_complex *symbols, int size) {
    if (this->m_symbolLength < size) {
        if (this->m_symbols != NULL)
            delete this->m_symbols;
        this->m_symbols = new gr_complex[size];
    }
    memcpy(this->m_symbols, symbols, sizeof(gr_complex) * size);
    m_symbolLength = size;
    draw();
}

void CConstellation::resizeEvent(QResizeEvent*)
{
    if (!size().isValid())
        return;

    if (m_Size.width() != size().width() || m_Size.height() != size().height())
    {
        m_Size = size();
        this->m_ConstellationPixmap = QPixmap(m_Size.width(), m_Size.height());
        this->m_OverlayPixmap = QPixmap(m_Size.width(), m_Size.height());

        this->m_ConstellationPixmap.fill(Qt::black);
        this->m_OverlayPixmap.fill(Qt::transparent);
        updateOverlay();
    }

    draw();
}

void CConstellation::setMode(int n)
{
    this->mode = n;
    updateOverlay();
    update();
}

void CConstellation::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_ConstellationPixmap);
    painter.drawPixmap(0, 0, m_OverlayPixmap);
}

void CConstellation::draw()
{
    if (m_Size.width() > 0 && m_Size.height() > 0) {
        QPainter paint(&this->m_ConstellationPixmap);
        this->m_ConstellationPixmap.fill(Qt::black);
        paint.setPen(QColor(255, 255, 255, 255));
        paint.setBrush(QBrush(QColor(255,255,255,255), Qt::SolidPattern));
        for (int i=0; i<m_symbolLength; i++) {
            if (m_symbols[i].real() != 0 &&  m_symbols[i].imag() != 0) {
                float x = m_symbols[i].real() * this->m_scaleX * (m_Size.width() / 2) + (m_Size.width() / 2);
                float y = m_symbols[i].imag() * this->m_scaleY * (m_Size.height() / 2) + (m_Size.height() / 2);
                paint.drawEllipse(QPoint(x,y), 1, 1);
            }
        }
        update();
    }
}

void CConstellation::updateOverlay() {
    if (m_Size.width() > 0 && m_Size.height() > 0) {
        QPainter paint(&this->m_OverlayPixmap);
        this->m_OverlayPixmap.fill(Qt::transparent);
        paint.setPen(QColor(127, 127, 127, 255));

        paint.drawLine(QPointF(m_Size.width() / 2.0, 0), QPointF(m_Size.width() / 2.0, m_Size.height()));
        if (mode > 2)
            paint.drawLine(QPointF(0, m_Size.height() / 2.0), QPointF(m_Size.width(), m_Size.height() / 2.0));
        if (mode > 4) {
            paint.drawLine(QPointF(0,0), QPointF(m_Size.width(), m_Size.height()));
            paint.drawLine(QPointF(m_Size.width(),0), QPointF(0, m_Size.height()));
        }
    }
}
