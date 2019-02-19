
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2006 Mike Gashler <gashlerm@yahoo.com>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// TODO: Clarence's code review

#include "kpEffectToneEnhance.h"

#include <QImage>

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"


#define RED_WEIGHT 77
#define GREEN_WEIGHT 150
#define BLUE_WEIGHT 29

#define MAX_TONE_VALUE ((RED_WEIGHT + GREEN_WEIGHT + BLUE_WEIGHT) * 255)
#define TONE_DROP_BITS 5
#define TONE_MAP_SIZE ((MAX_TONE_VALUE >> TONE_DROP_BITS) + 1)
#define MAX_GRANULARITY 25
#define MIN_IMAGE_DIM 3

//---------------------------------------------------------------------

inline unsigned int ComputeTone(unsigned int color)
{
  return RED_WEIGHT * static_cast<unsigned int> (qRed(color)) +
          GREEN_WEIGHT * static_cast<unsigned int> (qGreen(color)) +
          BLUE_WEIGHT * static_cast<unsigned int> (qBlue(color));
}

//---------------------------------------------------------------------

inline unsigned int AdjustTone(unsigned int color, unsigned int oldTone, unsigned int newTone, double amount)
{
  return qRgba(
      qMax(0, qMin(255, static_cast<int> (amount * qRed(color) * newTone / oldTone + (1.0 - amount) * qRed(color)))),
      qMax(0, qMin(255, static_cast<int> (amount * qGreen(color) * newTone / oldTone + (1.0 - amount) * qGreen(color)))),
      qMax(0, qMin(255, static_cast<int> (amount * qBlue(color) * newTone / oldTone + (1.0 - amount) * qBlue(color)))),
      qAlpha(color)
    );
}

//---------------------------------------------------------------------

class kpEffectToneEnhanceApplier
{
  public:
    kpEffectToneEnhanceApplier ();
    ~kpEffectToneEnhanceApplier ();

    void BalanceImageTone(QImage* pImage, double granularity, double amount);

  protected:
    int m_nToneMapGranularity, m_areaWid, m_areaHgt;
    unsigned int m_nComputedWid, m_nComputedHgt;
    // LOTODO: Use less error-prone QTL containers instead.
    unsigned int* m_pHistogram;
    unsigned int** m_pToneMaps;

    void DeleteToneMaps();
    unsigned int* MakeToneMap(QImage* pImage, int x, int y, int nGranularity);
    void ComputeToneMaps(QImage* pImage, int nGranularity);
    unsigned int InterpolateNewTone(QImage* pImage, unsigned int oldTone, int x, int y, int nGranularity);
};

//---------------------------------------------------------------------

kpEffectToneEnhanceApplier::kpEffectToneEnhanceApplier ()
{
  m_nToneMapGranularity = 0;
  m_areaWid = 0;
  m_areaHgt = 0;
  m_nComputedWid = 0;
  m_nComputedHgt = 0;
  m_pHistogram = new unsigned int[TONE_MAP_SIZE];
  m_pToneMaps = nullptr;
}

//---------------------------------------------------------------------

kpEffectToneEnhanceApplier::~kpEffectToneEnhanceApplier ()
{
  DeleteToneMaps();
  delete[] m_pHistogram;
}

//---------------------------------------------------------------------

// protected
void kpEffectToneEnhanceApplier::DeleteToneMaps()
{
  int nToneMaps = m_nToneMapGranularity * m_nToneMapGranularity;
  for(int i = 0; i < nToneMaps; i++) {
      delete[] m_pToneMaps[i];
  }
  delete[] m_pToneMaps;
  m_pToneMaps = nullptr;
  m_nToneMapGranularity = 0;
}

//---------------------------------------------------------------------

// protected
unsigned int* kpEffectToneEnhanceApplier::MakeToneMap(QImage* pImage, int u, int v, int nGranularity)
{
    // Compute the region to make the tone map for
    int xx, yy;
    if(nGranularity > 1)
    {
        xx = u * (pImage->width() - 1) / (nGranularity - 1) - m_areaWid / 2;
        if(xx < 0) {
            xx = 0;
        }
        else if(xx + m_areaWid > pImage->width()) {
            xx = pImage->width() - m_areaWid;
        }

        yy = v * (pImage->width() - 1) / (nGranularity - 1) - m_areaHgt / 2;

        if(yy < 0) {
            yy = 0;
        }
        else if(yy + m_areaHgt > pImage->height()) {
            yy = pImage->height() - m_areaHgt;
        }
    }
    else
    {
        xx = 0;
        yy = 0;
    }

  // Make a tone histogram for the region
  memset(m_pHistogram, '\0', sizeof(unsigned int) * TONE_MAP_SIZE);
  int x, y;
  unsigned int tone;
  for(y = 0; y < m_areaHgt; y++)
  {
    for(x = 0; x < m_areaWid; x++)
    {
      tone = ComputeTone(pImage->pixel(xx + x, yy + y));
      m_pHistogram[tone >> TONE_DROP_BITS]++;
    }
  }

  // Forward sum the tone histogram
  int i{};
  for(i = 1; i < TONE_MAP_SIZE; i++) {
      m_pHistogram[i] += m_pHistogram[i - 1];
  }

  // Compute the forward contribution to the tone map
  auto total = m_pHistogram[i - 1];
  auto *pToneMap = new unsigned int[TONE_MAP_SIZE];
  for(i = 0; i < TONE_MAP_SIZE; i++) {
      pToneMap[i] = static_cast<uint> (static_cast<unsigned long long int> (m_pHistogram[i] * MAX_TONE_VALUE / total));
  }
/*
  // Undo the forward sum and reverse sum the tone histogram
  m_pHistogram[TONE_MAP_SIZE - 1] -= m_pHistogram[TONE_MAP_SIZE - 2];
  for(i = TONE_MAP_SIZE - 2; i > 0; i--)
  {
    m_pHistogram[i] -= m_pHistogram[i - 1];
    m_pHistogram[i] += m_pHistogram[i + 1];
  }
  m_pHistogram[0] += m_pHistogram[1];
*/
  return pToneMap;
}

//---------------------------------------------------------------------

// protected
void kpEffectToneEnhanceApplier::ComputeToneMaps(QImage* pImage, int nGranularity)
{
  if(nGranularity == m_nToneMapGranularity && pImage->width() ==
          static_cast<int> (m_nComputedWid) && pImage->height() == static_cast<int> (m_nComputedHgt))
  {
    return; // We've already computed tone maps for this granularity
  }
  DeleteToneMaps();
  m_pToneMaps = new unsigned int*[nGranularity * nGranularity];
  m_nToneMapGranularity = nGranularity;
  m_nComputedWid = static_cast<unsigned int> (pImage->width());
  m_nComputedHgt = static_cast<unsigned int> (pImage->height());
  int u, v;
  for(v = 0; v < nGranularity; v++)
  {
      for(u = 0; u < nGranularity; u++) {
          m_pToneMaps[nGranularity * v + u] = MakeToneMap(pImage, u, v, nGranularity);
      }
  }
}

//---------------------------------------------------------------------

// protected
unsigned int kpEffectToneEnhanceApplier::InterpolateNewTone(QImage* pImage, unsigned int oldTone, int x, int y, int nGranularity)
{
  oldTone = (oldTone >> TONE_DROP_BITS);
  if(m_nToneMapGranularity <= 1) {
      return m_pToneMaps[0][oldTone];
  }
  auto u = x * (nGranularity - 1) / pImage->width();
  auto v = y * (nGranularity - 1) / pImage->height();
  auto x1y1 = m_pToneMaps[m_nToneMapGranularity * v + u][oldTone];
  auto x2y1 = m_pToneMaps[m_nToneMapGranularity * v + u + 1][oldTone];
  auto x1y2 = m_pToneMaps[m_nToneMapGranularity * (v + 1) + u][oldTone];
  auto x2y2 = m_pToneMaps[m_nToneMapGranularity * (v + 1) + u + 1][oldTone];
  auto hFac = x - (u * (pImage->width() - 1) / (nGranularity - 1));
  if(hFac > m_areaWid) {
      hFac = m_areaWid;
  }
  unsigned int y1 = (x1y1 * (static_cast<unsigned int> (m_areaWid) - static_cast<unsigned int> (hFac))
                     + x2y1 * static_cast<unsigned int> (hFac)) / static_cast<unsigned int> (m_areaWid);

  unsigned int y2 = (x1y2 * (static_cast<unsigned int> (m_areaWid) - static_cast<unsigned int> (hFac))
                     + x2y2 * static_cast<unsigned int> (hFac)) / static_cast<unsigned int> (m_areaWid);

  int vFac = y - (v * (pImage->height() - 1) / (nGranularity - 1));
  if(vFac > m_areaHgt) {
      vFac = m_areaHgt;
  }
  return (y1 * (static_cast<unsigned int> (m_areaHgt) - static_cast<unsigned int> (vFac))
          + y2 * static_cast<unsigned int> (vFac)) / static_cast<unsigned int> (m_areaHgt);
}

//---------------------------------------------------------------------

// public
void kpEffectToneEnhanceApplier::BalanceImageTone(QImage* pImage, double granularity, double amount)
{
    if(pImage->width() < MIN_IMAGE_DIM || pImage->height() < MIN_IMAGE_DIM) {
        return; // the image is not big enough to perform this operation
    }
  int nGranularity = static_cast<int> (granularity * (MAX_GRANULARITY - 2)) + 1;
  m_areaWid = pImage->width() / nGranularity;
  if(m_areaWid < MIN_IMAGE_DIM) {
      m_areaWid = MIN_IMAGE_DIM;
  }
  m_areaHgt = pImage->height() / nGranularity;
  if(m_areaHgt < MIN_IMAGE_DIM) {
      m_areaHgt = MIN_IMAGE_DIM;
  }
  ComputeToneMaps(pImage, nGranularity);
  int x, y;
  unsigned int oldTone, newTone, col;
  for(y = 0; y < pImage->height(); y++)
  {
      for(x = 0; x < pImage->width(); x++)
      {
          col = pImage->pixel(x, y);
          oldTone = ComputeTone(col);
          newTone = InterpolateNewTone(pImage, oldTone, x, y, nGranularity);
          pImage->setPixel(x, y, AdjustTone(col, oldTone, newTone, amount));
      }
  }
}

//---------------------------------------------------------------------

// public static
kpImage kpEffectToneEnhance::applyEffect (const kpImage &image,
                                          double granularity, double amount)
{
  if (amount == 0.0) {
      return image;
  }

  QImage qimage(image);

  // OPT: Cache the calculated values?
  kpEffectToneEnhanceApplier applier;
  applier.BalanceImageTone (&qimage, granularity, amount);

  return qimage;
}

//---------------------------------------------------------------------
