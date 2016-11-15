#include "dockconstellation.h"
#include "ui_dockconstellation.h"

#include <iostream>

QStringList DockConstellation::PSKTypes;

DockConstellation::DockConstellation(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DockConstellation)
{
    ui->setupUi(this);

    if (PSKTypes.size() == 0) {
        PSKTypes.append("BPSK");
        PSKTypes.append("QPSK");
        PSKTypes.append("8PSK");
    }

    ui->pskTypeCombo->addItems(PSKTypes);
    ui->clockAlphaEdit->setValidator( new QDoubleValidator(0, 8, 6, this) );
    ui->pllAlphaEdit->setValidator( new QDoubleValidator(0, 8, 6, this) );
    ui->rrcAlphaEdit->setValidator( new QDoubleValidator(0, 1, 2, this) );
    ui->symbolRateEdit->setValidator( new QDoubleValidator(1, 200e5, 6, this) );

    connect(ui->pskTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPSKTypeSelectedIndexChange(int)));
    connect(ui->pllAlphaEdit, SIGNAL(returnPressed()), this, SLOT(onPllAlphaUpdate()));
    connect(ui->clockAlphaEdit, SIGNAL(returnPressed()), this, SLOT(onClockAlphaUpdate()));
    connect(ui->rrcAlphaEdit, SIGNAL(returnPressed()), this, SLOT(onRrcAlphaUpdate()));
    connect(ui->symbolRateEdit, SIGNAL(returnPressed()), this, SLOT(onSymbolRateUpdate()));

    ui->constellation->draw();
}

DockConstellation::~DockConstellation()
{
    delete ui;
}

void DockConstellation::setNewSymbolData(gr_complex *data, int length)
{
    ui->constellation->setNewSymbolData(data, length);
}

void DockConstellation::onPSKTypeSelectedIndexChange(int index)
{
    switch(index) {
        case 0: emit pskTypeChange(2); ui->constellation->setMode(2); break; // BPSK
        case 1: emit pskTypeChange(4); ui->constellation->setMode(4); break; // QPSK
        case 2: emit pskTypeChange(8); ui->constellation->setMode(8); break; // 8PSK
    }
}

void DockConstellation::onSymbolRateUpdate()
{
    emit symbolRateChange(ui->symbolRateEdit->text().toDouble());
}

void DockConstellation::onClockAlphaUpdate()
{
    emit clockAlphaChange(ui->clockAlphaEdit->text().toDouble());
}

void DockConstellation::onPllAlphaUpdate()
{
    emit pllAlphaChange(ui->pllAlphaEdit->text().toDouble());
}

void DockConstellation::onRrcAlphaUpdate()
{
    std::cout << "RRC ALpha Change" << std::endl;
    emit rrcAlphaChange(ui->rrcAlphaEdit->text().toDouble());
}
