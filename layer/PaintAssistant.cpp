/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "PaintAssistant.h"

#include "LayerGeometryProvider.h"

#include "base/AudioLevel.h"
#include "base/Strings.h"
#include "base/Debug.h"

#include <QPaintDevice>
#include <QPainter>

#include <iostream>
#include <cmath>

void
PaintAssistant::paintVerticalLevelScale(QPainter &paint, QRect rect,
                                        double minVal, double maxVal,
                                        Scale scale, AudioLevel::Quantity sort,
                                        int &mult, std::vector<int> *vy)
{
    static double meterdbs[] = { -40, -30, -20, -15, -10,
                                -5, -3, -2, -1, -0.5, 0 };

    int h = rect.height(), w = rect.width();
    int textHeight = paint.fontMetrics().height();
    int toff = -textHeight/2 + paint.fontMetrics().ascent() + 1;

    int lastLabelledY = -1;

    int n = 10;
    if (vy) vy->clear();

    double step = 0;
    mult = 1;
    if (scale == LinearScale) {
        step = (maxVal - minVal) / n;
        int round = 0, limit = 10000000;
        do {
            round = int(minVal + step * mult);
            mult *= 10;
        } while (!round && mult < limit);
        if (round) {
            mult /= 10;
            step = double(round) / mult;
            n = int(lrint((maxVal - minVal) / step));
            if (mult > 1) {
                mult /= 10;
            }
        }
    }

    for (int i = 0; i <= n; ++i) {
        
        double val = 0.0, nval = 0.0;
        QString text = "";

        switch (scale) {
                
        case LinearScale:
            val = (minVal + (i * step));
            text = QString("%1").arg(mult * val);
            break;
            
        case MeterScale: // ... min, max
            val = AudioLevel::dB_to_quantity(meterdbs[i], sort);
            text = QString("%1").arg(meterdbs[i]);
            if (i == n) text = "0dB";
            if (i == 0) {
                text = Strings::minus_infinity;
                val = 0.0;
            }
            break;

        case dBScale: // ... min, max
        {   double step = (sort == AudioLevel::Quantity::Power ? 10 : 20);
            double dB = -(step * n) + i * step;
            val = AudioLevel::dB_to_quantity(dB, sort);
            text = QString("%1").arg(dB);
            if (i == n) text = "0dB";
            if (i == 0) {
                text = Strings::minus_infinity;
                val = 0.0;
            }
            break;
        }
        }

        if (val < minVal || val > maxVal) continue;

        int y = getYForValue(scale, sort, val, minVal, maxVal, rect.y(), h);
            
        int ny = y;
        if (nval != 0.0) {
            ny = getYForValue(scale, sort, nval, minVal, maxVal, rect.y(), h);
        }

//        SVDEBUG << "PaintAssistant::paintVerticalLevelScale: val = "
//                  << val << ", y = " << y << ", h = " << h << endl;

        bool spaceForLabel = (i == 0 ||
                              abs(y - lastLabelledY) >= textHeight - 1);
        
        if (spaceForLabel) {
            
            int tx = 3;
            if (paint.fontMetrics().horizontalAdvance(text) < w - 10) {
                tx = w - 10 - paint.fontMetrics().horizontalAdvance(text);
            }
            
            int ty = y;

            if (ty < paint.fontMetrics().ascent()) {
                ty = paint.fontMetrics().ascent();
            } else {
                ty += toff;
            }

            paint.drawText(tx, ty, text);
            
            lastLabelledY = ty - toff;

            paint.drawLine(w - 7, y, w, y);
            if (vy) vy->push_back(y);

            if (ny != y) {
                paint.drawLine(w - 7, ny, w, ny);
                if (vy) vy->push_back(ny);
            }
            
        } else {
            
            paint.drawLine(w - 4, y, w, y);
            if (vy) vy->push_back(y);

            if (ny != y) {
                paint.drawLine(w - 4, ny, w, ny);
                if (vy) vy->push_back(ny);
            }
        }
    }
}

static int
dBscale(AudioLevel::Quantity sort, double sample,
        int m, double maxVal, double minVal) 
{
    if (sample < 0.0) return dBscale(sort, -sample, m, maxVal, minVal);
    double dB = AudioLevel::quantity_to_dB(sample, sort);
    double mindB = AudioLevel::quantity_to_dB(minVal, sort);
    double maxdB = AudioLevel::quantity_to_dB(maxVal, sort);
    if (dB < mindB) return 0;
    if (dB > 0.0) return m;
    return int(((dB - mindB) * m) / (maxdB - mindB) + 0.1);
}

int
PaintAssistant::getYForValue(Scale scale, AudioLevel::Quantity sort,
                             double value, double minVal, double maxVal,
                             int minY, int height)
{
    int vy = 0;

    switch (scale) {

    case LinearScale:
        vy = minY + height - int(((value - minVal) / (maxVal - minVal)) * height);
        break;

    case MeterScale:
        vy = minY + height - AudioLevel::dB_to_fader
            (AudioLevel::quantity_to_dB((value - minVal) / (maxVal - minVal),
                                        sort),
             height,
             AudioLevel::Scale::Preview);
        break;

    case dBScale:
        vy = minY + height - dBscale(sort, value, height, maxVal, minVal);
        break;
    }

    return vy;
}

void
PaintAssistant::drawVisibleText(const LayerGeometryProvider *v,
                                QPainter &paint, int x, int y,
                                QString text, TextStyle style)
{
    if (style == OutlinedText || style == OutlinedItalicText) {

        paint.save();

        if (style == OutlinedItalicText) {
            QFont f(paint.font());
            f.setItalic(true);
            paint.setFont(f);
        }

        QColor penColour, surroundColour, boxColour;

        penColour = v->getForeground();
        surroundColour = v->getBackground();
        boxColour = surroundColour;
        boxColour.setAlpha(127);

        QRectF boundingRect = paint.boundingRect
            (QRectF(), Qt::AlignTop | Qt::AlignLeft, text);

        QRectF textRect = boundingRect.translated
            (QPointF(x, y - paint.fontMetrics().ascent()));

        QRectF boxRect(textRect.x() - 2, textRect.y() - 2,
                       textRect.width() + 4, textRect.height() + 4);

        paint.setPen(Qt::NoPen);
        paint.setBrush(boxColour);
        
        paint.drawRect(boxRect);
        paint.setBrush(Qt::NoBrush);

        paint.setPen(surroundColour);

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (!(dx || dy)) continue;
                paint.drawText(textRect.translated(QPointF(dx, dy)),
                               Qt::AlignTop | Qt::AlignLeft,
                               text);
            }
        }

        paint.setPen(penColour);

        paint.drawText(textRect,
                       Qt::AlignTop | Qt::AlignLeft,
                       text);

        paint.restore();

    } else {

        std::cerr << "ERROR: PaintAssistant::drawVisibleText: Boxed style not yet implemented!" << std::endl;
    }
}


