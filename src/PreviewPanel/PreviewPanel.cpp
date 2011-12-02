/**
 * This file is a part of Luminance HDR package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2011 Franco Comida
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ----------------------------------------------------------------------
 *
 * @author Franco Comida <fcomida@users.sourceforge.net>
 */

#include <QDebug>
#include <QtConcurrentMap>
#include <QSharedPointer>

#include "PreviewPanel/PreviewPanel.h"
#include "Filter/pfssize.h"
#include "Filter/pfscut.h"
#include "Core/TMWorker.h"
#include "Fileformat/pfsoutldrimage.h"
#include "PreviewPanel/PreviewLabel.h"
#include "TonemappingEngine/TonemapOperator.h"

namespace // anoymous namespace
{
const int PREVIEW_WIDTH = 128;
const int PREVIEW_WIDTH_TM = 500;

//! \note It is not the most efficient way to do this thing, but I will fix it later
//! this function get calls multiple time
void resetTonemappingOptions(TonemappingOptions* tm_options)
{
    tm_options->origxsize          = PREVIEW_WIDTH;
    tm_options->xsize              = PREVIEW_WIDTH;
    tm_options->pregamma           = 1.0f;
    tm_options->tonemapSelection   = false;
    tm_options->tonemapOriginal    = false;
}

class PreviewLabelUpdater
{
public:
    PreviewLabelUpdater(QSharedPointer<pfs::Frame> reference_frame):
        m_ReferenceFrame(reference_frame)
    {}

    //! \brief QRunnable::run() definition
    //! \caption I use shared pointer in this function, so I don't have to worry about memory allocation
    //! in case something wrong happens, it shouldn't leak
    void operator()(PreviewLabel* to_update)
    {
#ifdef QT_DEBUG
        //qDebug() << QThread::currentThread() << "running...";
#endif

        // retrieve TM parameters
        TonemappingOptions* tm_options = to_update->getTonemappingOptions();
        resetTonemappingOptions(tm_options);

        if ( m_ReferenceFrame.isNull() )
        {
#ifdef QT_DEBUG
            qDebug() << "operator()() for TM" << static_cast<int>(tm_options->tmoperator) << " received a NULL pointer";
            return;
#endif
        }

        ProgressHelper fake_progress_helper;

        // Copy Reference Frame
        QSharedPointer<pfs::Frame> temp_frame( pfs::pfscopy(m_ReferenceFrame.data()) );

        // Tone Mapping
        QScopedPointer<TonemapOperator> tm_operator( TonemapOperator::getTonemapOperator(tm_options->tmoperator));
        tm_operator->tonemapFrame(temp_frame.data(), tm_options, fake_progress_helper);

        // Create QImage from pfs::Frame into QSharedPointer, and I give it to the preview panel
        QSharedPointer<QImage> qimage(fromLDRPFStoQImage(temp_frame.data()));

        //! \note I cannot use these 2 functions, because setPixmap must run in the GUI thread
        //m_PreviewLabel->setPixmap( QPixmap::fromImage(*qimage) );
        //m_PreviewLabel->adjustSize();
        //! \note So I queue a SLOT request on the m_PreviewPanel
        QMetaObject::invokeMethod(to_update, "assignNewQImage", Qt::QueuedConnection,
                                  Q_ARG(QSharedPointer<QImage>, qimage));

#ifdef QT_DEBUG
        //qDebug() << QThread::currentThread() << "done!";
#endif
    }

private:
    QSharedPointer<pfs::Frame> m_ReferenceFrame;
};

}

PreviewPanel::PreviewPanel(QWidget *parent):
        QWidget(parent)
{
    //! \note I need to register the new object to pass this class as parameter inside invokeMethod()
    //! see run() inside PreviewLabelUpdater
    qRegisterMetaType< QSharedPointer<QImage> >("QSharedPointer<QImage>");

    setupUi(this);

    PreviewLabel * labelMantiuk06 = new PreviewLabel(frameMantiuk06, mantiuk06);
    labelMantiuk06->setText("Mantiuk '06");
    m_ListPreviewLabel.push_back(labelMantiuk06);
    connect(labelMantiuk06, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelMantiuk08 = new PreviewLabel(frameMantiuk08, mantiuk08);
    labelMantiuk08->setText("Mantiuk '08");
    m_ListPreviewLabel.push_back(labelMantiuk08);
    connect(labelMantiuk08, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelFattal = new PreviewLabel(frameFattal, fattal);
    labelFattal->setText("Fattal");
    m_ListPreviewLabel.push_back(labelFattal);
    connect(labelFattal, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelDrago = new PreviewLabel(frameDrago, drago);
    labelDrago->setText("Drago");
    m_ListPreviewLabel.push_back(labelDrago);
    connect(labelDrago, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelDurand = new PreviewLabel(frameDurand, durand);
    labelDurand->setText("Durand");
    m_ListPreviewLabel.push_back(labelDurand);
    connect(labelDurand, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelReinhard02= new PreviewLabel(frameReinhard02, reinhard02);
    labelReinhard02->setText("Reinhard '02");
    m_ListPreviewLabel.push_back(labelReinhard02);
    connect(labelReinhard02, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelReinhard05 = new PreviewLabel(frameReinhard05, reinhard05);
    labelReinhard05->setText("Reinhard '05");
    m_ListPreviewLabel.push_back(labelReinhard05);
    connect(labelReinhard05, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelAshikhmin = new PreviewLabel(frameAshikhmin, ashikhmin);
    labelAshikhmin->setText("Ashikhmin");
    m_ListPreviewLabel.push_back(labelAshikhmin);
    connect(labelAshikhmin, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));

    PreviewLabel * labelPattanaik = new PreviewLabel(framePattanaik, pattanaik);
    labelPattanaik->setText("Pattanaik");
    m_ListPreviewLabel.push_back(labelPattanaik);
    connect(labelPattanaik, SIGNAL(clicked(TonemappingOptions*)), this, SLOT(tonemapPreview(TonemappingOptions*)));
}

PreviewPanel::~PreviewPanel()
{
#ifdef QT_DEBUG
    qDebug() << "PreviewPanel::~PreviewPanel()";
#endif
}

void PreviewPanel::updatePreviews(pfs::Frame* frame)
{
    if ( frame == NULL ) return;

    original_width_frame = frame->getWidth();
    // 1. make a resized copy
    QSharedPointer<pfs::Frame> current_frame( pfs::resizeFrame(frame, PREVIEW_WIDTH));

    // 2. (non concurrent) for each PreviewLabel, call PreviewLabelUpdater::operator()
    foreach(PreviewLabel* current_label, m_ListPreviewLabel)
    {
        PreviewLabelUpdater updater(current_frame);
        updater(current_label);
    }
    // 2. (concurrent) for each PreviewLabel, call PreviewLabelUpdater::operator()
    //QtConcurrent::map (m_ListPreviewLabel, PreviewLabelUpdater(current_frame) );
}

void PreviewPanel::tonemapPreview(TonemappingOptions* opts)
{
#ifdef QT_DEBUG
    qDebug() << "void PreviewPanel::tonemapPreview()";
#endif

    opts->xsize = PREVIEW_WIDTH_TM;
    opts->origxsize = original_width_frame;

    emit startTonemapping(opts);
}
