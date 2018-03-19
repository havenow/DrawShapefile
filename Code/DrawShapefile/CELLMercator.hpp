#pragma once

namespace   CELL
{
    const static    double  originShift         =   2 * PI * 6378137 / 2.0;;

    class   CELLMercator
    {
    public:
        CELLMercator()
        {}

        static  double  lonToMeter(double lon)
        { 
            return  lon * originShift / 180.0 ;
        }
        static  double  latToMeter(double lat)
        { 
            double  my =    log( tan((90 + lat) *PI / 360.0 )) / (PI / 180.0);
            return  my =    my * originShift / 180.0;
        }
        /**
        *   经纬度转化为米
        */
        static  double2 lonLatToMeters(double lon,double lat )
        {
            // Converts given lat/lon in WGS84 Datum to XY in Spherical Mercator EPSG:900913  
            double  mx =    lon * originShift / 180.0 ;
            double  my =    log( tan((90 + lat) *PI / 360.0 )) / (PI / 180.0);
                    my =    my * originShift / 180.0;
            return  double2(mx, my);
        }
        /**
        *   米转为经纬度
        */
        static  double2 metersToLonLat(double mx,double my )
        {
            double  lon =   (mx / originShift) * 180.0;
            double  lat =   (my / originShift) * 180.0;
                    lat =   180 / PI * (2 * atan( exp( lat * PI / 180.0)) - PI / 2.0);
            return  double2(lon,lat);
        }
    };
}