/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _PANNER_H_
#define _PANNER_H_

#include <QWidget>

class Panner : public QWidget
{
    Q_OBJECT

public:
    Panner(QWidget *parent = 0);
    virtual ~Panner();

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void paintEvent(QPaintEvent *e);

    virtual QSize sizeHint() const;

signals:
    /**
     * Emitted when the panned rectangle is dragged or otherwise
     * moved.  Arguments are x0, y0, width and height of the rectangle
     * in the range 0 -> 1 as proportions of the width and height of
     * the whole widget.
     */
    void rectExtentsChanged(float, float, float, float);

    /**
     * Emitted when the rectangle is dragged or otherwise moved (as
     * well as extentsChanged).  Arguments are the centre coordinates
     * of the rectangle in the range 0 -> 1 as proportions of the
     * width and height of the whole widget.
     */
    void rectCentreMoved(float, float);

public slots:
    /** 
     * Set the extents of the panned rectangle within the overall
     * panner widget.  Coordinates are in the range 0 -> 1 in both axes,
     * with 0 at the top in the y axis.
     */
    void setRectExtents(float x0, float y0, float width, float height);

    /**
     * Set the width of the panned rectangle as a fraction (0 -> 1) of
     * that of the whole panner widget.
     */
    void setRectWidth(float width);

    /**
     * Set the height of the panned rectangle as a fraction (0 -> 1)
     * of that of the whole panner widget.
     */
    void setRectHeight(float height);

    /**
     * Set the location of the centre of the panned rectangle on the x
     * axis, as a proportion (0 -> 1) of the width of the whole panner
     * widget.
     */
    void setRectCentreX(float x);
    
    /**
     * Set the location of the centre of the panned rectangle on the y
     * axis, as a proportion (0 -> 1) of the height of the whole panner
     * widget.
     */
    void setRectCentreY(float y);

protected:
    void normalise();
    void emitAndUpdate();

    float m_rectX;
    float m_rectY;
    float m_rectWidth;
    float m_rectHeight;
};

#endif

