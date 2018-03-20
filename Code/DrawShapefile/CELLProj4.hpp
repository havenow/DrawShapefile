#pragma once
#include    "proj4.8/proj_api.h"
namespace   CELL
{
    class   CELLProj4
    {
    protected:
        projPJ      _pj;
    public:
        CELLProj4()
        {
            char google[]    =   "+proj=merc +a=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +no_defs";
            _pj =   pj_init_plus(google);
        }

        ~CELLProj4()
        {
            if (_pj)
            {
                pj_free(_pj);
            }
        }
        double2   lonLatToMeters(double lon,double lat)
        {
            projUV  p;
            p.u     =   lon * DEG_TO_RAD;
            p.v     =   lat * DEG_TO_RAD;
            p       =   pj_fwd(p,_pj);
            return  double2(p.u,p.v);
        }
        double2   metersToLontLat(double x,double y)
        {
            projUV  p;
            p.u     =   x ;
            p.v     =   y ;
            p       =   pj_inv(p,_pj);
            return  double2(p.u * RAD_TO_DEG,p.v * RAD_TO_DEG);
        }
    };
}