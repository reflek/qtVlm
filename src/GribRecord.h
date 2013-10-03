/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

***********************************************************************/


#ifndef GRIBRECORD_H
#define GRIBRECORD_H

#include "class_list.h"
#include "dataDef.h"

#define MUST_INTERPOLATE_VALUE true

class GribRecord {
    public:
        GribRecord();

        static void init_conversionMatrix(void) { }

        bool  isOk()  const   {return ok;}
        bool  isDataKnown()  const   {return knownData;}

        FCT_GET(int,editionNumber)
        FCT_GET(int,idCenter)
        FCT_GET(int,idModel)
        FCT_GET(int,idGrid)

        FCT_GET(time_t,refDate)
        FCT_GET(time_t,curDate)


        FCT_GET(int,dataType)
        void set_dataType(int t=DATA_NOTDEF);
        FCT_GET(int,levelType)
        FCT_GET(int,levelValue)


        FCT_GET(long int,dataKey)
        static long int makeKey(int dataType,int levelType,int levelValue);

        // Nombre de points de la grille
        FCT_GET(int,Ni)
        FCT_GET(int,Nj)
        FCT_GET(double,Di)
        FCT_GET(double,Dj)

        FCT_GET(bool,isFull)

        FCT_GET_CST(double,latMin)
        FCT_GET_CST(double,latMax)
        FCT_GET_CST(double,lonMin)
        FCT_GET_CST(double,lonMax)

        FCT_GET(int,dataSize)
        FCT_GET(int,bmapSize)

        // coordonnees d'un point de la grille
        inline double  getX(int i) const   { return ok ? Lo1+i*Di : GRIB_NOTDEF;}
        inline double  getY(int j) const   { return ok ? La1+j*Dj : GRIB_NOTDEF;}

        // Valeur pour un point de la grille
        virtual bool hasValue(int i, int j) const =0;
        inline double getValue(int i, int j) const  { return ok ? data[j*Ni+i] : GRIB_NOTDEF;}
        void setValue(unsigned int i, unsigned int j, double v) { if (i<Ni && j<Nj) data[j*Ni+i] = v; }

        // interpolation:
        double getInterpolatedValue(double px, double py, bool numericalInterpolation=MUST_INTERPOLATE_VALUE);
        bool getValue_TWSA(double px, double py,
                                       double * a00,double * a01,double * a10,double * a11,bool debug);

        // Le point est-il a  l'interieur de la grille ?
        inline bool   isPointInMap(double x, double y) const;
        inline bool   isXInMap(double x) const;
        inline bool   isYInMap(double y) const;

        void print_bitmap(void);

    protected:
        bool ok;

        int editionNumber;
        int idCenter;
        int idModel;
        int idGrid;

        int dataType;
        int levelType;
        int levelValue;
        long int dataKey;

        int gridType;
        unsigned int Ni, Nj;
        double La1, Lo1, La2, Lo2;
        double latMin, lonMin, latMax, lonMax;
        double Di, Dj;
        unsigned char resolFlags, scanFlags;
        bool hasDiDj;
        bool isScanIpositive;
        bool isScanJpositive;
        bool isAdjacentI;
        bool isFull;

        double  *data;
        bool knownData;


        int dataSize;
        int bmapSize;

        unsigned int refYear, refMonth, refDay, refHour, refMinute, refSecond;
        time_t refDate;      // C reference date
        time_t curDate;      // C current date
        time_t makeDate(unsigned int year,unsigned int month,
                        unsigned int day,unsigned int hour,unsigned int min,unsigned int sec);


};

inline long int GribRecord::makeKey(int dataType,int levelType,int levelValue) {
    return levelValue*10e6+dataType*10e3+levelType;
}

inline bool GribRecord::isPointInMap(double x, double y) const {
    return isXInMap(x) && isYInMap(y);
}

inline bool GribRecord::isXInMap(double x) const {
    double a=0;
    if(isFull) a=Di;

    if (Di > 0) return x>=Lo1 && x<=(Lo2+a);
    else        return x>=(Lo2+a) && x<=Lo1;
}

inline bool GribRecord::isYInMap(double y) const {
    if (Dj < 0) return y<=La1 && y>=La2;
    else        return y>=La1 && y<=La2;
}

#endif



