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
