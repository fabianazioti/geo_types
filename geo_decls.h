
#ifndef GEO_DECLS_H
#define GEO_DECLS_H


#include <fmgr.h>

typedef struct Geo_Point{
  double x;
  double y;
} Geo_Point;

typedef struct Geo_Polygon{
  int32 v_len_;
  int32 n_points;
  Geo_Point points[FLEXIBLE_ARRAY_MEMBER];
} Geo_Polygon;


#define DatumGetGeoPolygon(X)	 ((Geo_Polygon *) DatumGetPointer(X))
#define PG_GETARG_GEO_POLY(n) DatumGetGeoPolygon(PG_GETARG_DATUM(n))

PG_FUNCTION_INFO_V1(geo_point_in);
PG_FUNCTION_INFO_V1(geo_point_out);
PG_FUNCTION_INFO_V1(geo_point_recv);
PG_FUNCTION_INFO_V1(geo_point_send);

PG_FUNCTION_INFO_V1(geo_polygon_in);
PG_FUNCTION_INFO_V1(geo_polygon_out);


PG_FUNCTION_INFO_V1(adiciona_pontos);

PG_FUNCTION_INFO_V1(geo_point_in);
PG_FUNCTION_INFO_V1(geo_point_out);
PG_FUNCTION_INFO_V1(geo_point_recv);
PG_FUNCTION_INFO_V1(geo_point_send);

PG_FUNCTION_INFO_V1(adiciona_pontos);
PG_FUNCTION_INFO_V1(distancia_euclidiana);
PG_FUNCTION_INFO_V1(polygon_area);

PG_FUNCTION_INFO_V1(geo_polygon_in);
PG_FUNCTION_INFO_V1(geo_polygon_out);
PG_FUNCTION_INFO_V1(geo_polygon_recv);
PG_FUNCTION_INFO_V1(geo_polygon_send);

PG_FUNCTION_INFO_V1(polygon_has_point);
PG_FUNCTION_INFO_V1(polygon_has_points);


int dimension_count(char *s, char delim);
bool polygon_has_point_ve(Geo_Point *point, Geo_Polygon *geo_polygon);

#endif
