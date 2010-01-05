/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#include <iostream>
#include <cmath>
#include <QDebug>

#include "Projection.h"

//-----------------------------
// Constructeur
//-----------------------------
Projection::Projection(int w, int h, double cx, double cy) {
    scalemax = 50000;
    scale = -1;
    my_setScreenSize(w, h);
    my_setCenterInMap(cx,cy);
    this->timer = new QTimer(this);
    assert(timer);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SIGNAL(projectionUpdated()));
}

void Projection::setScreenSize(int w, int h)
{
    my_setScreenSize(w,h);
    emit projectionUpdated();
}

/* zoom and scale */
void Projection::zoomOnZone(double x0, double y0, double x1, double y1)
{
    // security
    if (x1 == x0)
        x1 = x0+0.1;
    if (y1 == y0)
        y1 = y0+0.1;

    xW=x0;
    xE=x1;
    yN=y0;
    yS=y1;

    // compute scale;
    double sX,sY,sYN,sYS;
    sX=W/fabs(xE-xW);
    sYN=log(tan(degToRad(yN)/2 + M_PI_4));
    sYS=log(tan(degToRad(yS)/2 + M_PI_4));
    sY=H/fabs(radToDeg(sYN-sYS));
    scale=sX<sY?sY:sX;

    if (scale > scalemax)
        scale = scalemax;

    // Nouvelle position du centre
    CX=(xE+xW)/2;
    CY=(yN+yS)/2;
    PX=CX;
    PY=radToDeg(log(tan(degToRad(CY)/2 + M_PI_4)));

    if((getW()*getH())!=0)
        coefremp = 10000.0*fabs( ((xE-xW)*(yN-yS)) / (getW()*getH()) );
    else
        coefremp = 10000.0;

    emit newZoom(scale);
    emit_projectionUpdated();
}

void Projection::zoom(float k)
{    
    my_setScale(scale*k);
    emit_projectionUpdated();
}

void Projection::zoomAll(void)
{
    my_setScale(scaleall);
    emit_projectionUpdated();
}

void Projection::setScale(float sc)
{
    my_setScale(sc);
    emit_projectionUpdated();
}

/**************************/
/* Move & center position */
/**************************/

void Projection::move(double dx, double dy)
{
    my_setCenterInMap(CX - dx*(xE-xW),CY - dy*(yN-yS));
    emit_projectionUpdated();
}

void Projection::setCentralPixel(int i, int j)
{
    double x, y;
    screen2map(i, j, &x, &y);
    my_setCenterInMap(x,y);
    emit_projectionUpdated();
}

void Projection::setCenterInMap(double x, double y)
{
    my_setCenterInMap(x,y);
    emit_projectionUpdated();
}

void Projection::setScaleAndCenterInMap(float sc,double x, double y)
{
    bool mod=false;
    if(sc!=-1)
    {
        my_setScale(sc);
        mod=true;
    }

    if(x!=0 && y!=0)
    {
        my_setCenterInMap(x,y);
        mod=true;
    }

    if(mod)
        emit_projectionUpdated();
}

/**********************/
/* internal functions */
/**********************/

void Projection::my_setScale(float sc)
{
    scale = sc;
    //qWarning() << "my setScale " << sc;
    if (scale < scaleall)
        scale = scaleall;
    if (scale > scalemax)
        scale = scalemax;
    updateBoundaries();
    emit newZoom(scale);
}

void Projection::my_setCenterInMap(double x, double y)
{
    while (x > 180.0) {
        x -= 360.0;
    }
    while (x < -180.0) {
        x += 360.0;
    }
    CX = x;
    CY = y;

    /* compute projection */
    PX=CX;
    PY=radToDeg(log(tan(degToRad(CY)/2 + M_PI_4)));

    updateBoundaries();
}

void Projection::my_setScreenSize(int w, int h)
{
    W = w;
    H = h;

    /* conpute scaleall */
    double sx, sy,sy1,sy2;
    sx = W/360.0;
    sy1=log(tan(degToRad(89.9)/2 + M_PI_4));
    sy2=log(tan(degToRad(-89.9)/2 + M_PI_4));
    sy = H/fabs(radToDeg(sy1-sy2));
    scaleall = (sx<sy) ? sy : sx;

    if (scale < scaleall)
    {
        scale = scaleall;
        emit newZoom(scale);
    }

    updateBoundaries();
}

void Projection::updateBoundaries() {
    /* lat */
    yS=radToDeg(2*atan(exp((double)(degToRad(PY-H/(2*scale)))))-M_PI_2);
    yN=radToDeg(2*atan(exp((double)(degToRad(PY+H/(2*scale)))))-M_PI_2);

    /* lon */
    xW=PX-W/(2*scale);
    xE=PX+W/(2*scale);

    /* xW and yN => upper corner */

    if((getW()*getH())!=0)
        coefremp = 10000.0*fabs( ((xE-xW)*(yN-yS)) / (getW()*getH()) );
    else
        coefremp = 10000.0;
}
void Projection::emit_projectionUpdated()
{
     timer->start(200);
}
