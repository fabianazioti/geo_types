
#ifndef GEO_DECLS_H
#define GEO_DECLS_H

#include <math.h>
#include <fmgr.h>
#include "postgres.h"

typedef struct Geo_Point{
  double x;
  double y;
} Geo_Point;

typedef struct Geo_Polygon{
  int32 v_len_;
  int32 n_points;
  Geo_Point points[FLEXIBLE_ARRAY_MEMBER];
} Geo_Polygon;

#endif


#define DatumGetGeoPolygon(X)	 ((Geo_Polygon *) DatumGetPointer(X))
#define PG_GETARG_GEO_POLY(n) DatumGetGeoPolygon(PG_GETARG_DATUM(n))

extern PG_FUNCTION_INFO_V1(geo_point_in);
extern PG_FUNCTION_INFO_V1(geo_point_out);
extern PG_FUNCTION_INFO_V1(geo_point_recv);
extern PG_FUNCTION_INFO_V1(geo_point_send);

extern PG_FUNCTION_INFO_V1(geo_polygon_in);
extern PG_FUNCTION_INFO_V1(geo_polygon_out);


extern PG_FUNCTION_INFO_V1(adiciona_pontos);
