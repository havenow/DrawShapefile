Shapefile文件绘制

- # Shapefile文件的解析

http://shapelib.maptools.org/

.bdf 矢量    
.proj 投影     
.shp 真正的矢量数据    

shapelib是以源代码形式提供使用的     

XXX.shp - holds the actual vertices.    
XXX.shx - hold index data pointing to the structures in the .shp file.     
XXX.dbf - holds the attributes in xBase (dBase) format.     

- # shapefile文件格式

要素文件：存储矢量数据点数据     
要素属性表：与要素一一对应（描述属性）    
投影参数：描述数据矢量数据的显示方式    

```c++
SHPInfo

typedef	struct
{
    SAHooks     sHooks;
    SAFile          fpSHP;
    SAFile          fpSHX;
    int               nShapeType; 
    unsigned int nFileSize;  
    int               nRecords;
    int               nMaxRecords;
    unsigned int *panRecOffset;
    unsigned int *panRecSize;
    double          adBoundsMin[4];
    double          adBoundsMax[4];
    int              bUpdated;
    unsigned char*  pabyRec;
    int             nBufSize;
} SHPInfo;



SHPObject

typedef struct
{
    int     nSHPType;
    int     nShapeId;
    int     nParts;
    int     *panPartStart;
    int     *panPartType;

    int     nVertices;
    double  *padfX;
    double  *padfY;
    double  *padfZ;
    double  *padfM;

    double  dfXMin;
    double  dfYMin;
    double  dfZMin;
    double  dfMMin;

    double  dfXMax;
    double  dfYMax;
    double  dfZMax;
    double  dfMMax;
    int     bMeasureIsUsed;
} SHPObject;
```

- # 绘制文字  

- # 墨卡托投影
墨卡托投影以整个世界范围，赤道作为标准纬线，本初子午线作为中央经线，两者交点为坐标原点，向东向北为正，向西向南为负。南北极在地图的正下、上方，而东西方向处于地图的正右、左。    

由于赤道半径为6378137米，则赤道周长为2*PI*r = 20037508， 因此X轴的取值范围：[-20037508,20037508]。当纬度φ接近两极，即90°时，Y值趋向于无穷。因此通常把Y轴的取值范围也限定在[-20037508,20037508]之间。因此在墨卡托投影坐标系（米）下的坐标范围是：最小为(-20037508, -20037508)到最大 坐标为(20037508, 20037508)     


- # proj4
http://proj4.org/


