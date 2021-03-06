/*
  Copyright (C) 2017 National Institute For Space Research (INPE) - Brazil.
  This file is part of pg_geoext, a simple PostgreSQL extension for 
  for teaching spatial database classes.
  pg_geoext is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 3 as
  published by the Free Software Foundation.
  pg_geoext is distributed  "AS-IS" in the hope that it will be useful,
  but WITHOUT ANY WARRANTY OF ANY KIND; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public License
  along with pg_geoext. See LICENSE. If not, write to
  Gilberto Ribeiro de Queiroz at <gribeiro@dpi.inpe.br>.
 */

#ifndef __GEOEXT_H__
#define __GEOEXT_H__

/* PostgreSQL */
#include <fmgr.h>

typedef struct coord2d
{
  double x;
  double y;
} coord2d;

/* Um geo_point é um tipo de tamanho fixo passado por referência. */
typedef struct geo_point
{
  coord2d coord;  /* par de coordenadas                       */
  int32 srid;     /* código do sistema de referência espacial */
  int32 dummy;    /* alinhamento de bytes                     */
} geo_point;

/* Um geo_polygon é um tipo de tamanho variável passado por referência. */
typedef struct geo_polygon
{
  int32 vl_len_;                           /*   Nao tocar neste campo diretamente        */
  int32 npts;                              /*   numero de pontos do geo_polygon          */
  int32 srid;                              /*   código do sistema de referência espacial */
  int32 dummy;                             /*   alinhamento de bytes                     */
  coord2d coords[FLEXIBLE_ARRAY_MEMBER];   /*   lista com o par de coordenadas           */
} geo_polygon;

/* Macros para manipulação de dados do tipo geo_point com a interface fmgr. */
#define DatumGetGeoPointP(X)     ((geo_point*) DatumGetPointer(X))
#define GeoPointPGetDatum(X)     PointerGetDatum(X)
#define PG_GETARG_GEOPOINT_P(n)  DatumGetGeoPointP(PG_GETARG_DATUM(n))
#define PG_RETURN_GEOPOINT_P(x)  return GeoPointPGetDatum(x)

/* Macros para manipulacao de dados do tipo geo_polygon com a interface fmgr.*/
#define DatumGetPolygonP(X) ((geo_polygon*) PG_DETOAST_DATUM(X))
#define GeoPolygonPGetDatum(X) PointerGetDatum(X)
#define PG_GETARG_GEOPOLYGON_P(n) DatumGetPolygonP(PG_GETARG_DATUM(n))
#define PG_RETURN_GEOPOLYGON_P(x) return GeoPolygonPGetDatum(x)

/* Operações suportadas pelo tipo geo_point */
PG_FUNCTION_INFO_V1(geo_point_in);
PG_FUNCTION_INFO_V1(geo_point_out);

PG_FUNCTION_INFO_V1(geo_point_distance);
PG_FUNCTION_INFO_V1(geo_point_from_text);
PG_FUNCTION_INFO_V1(geo_point_to_str);

/*Operacoes suportadas pelo tipo geo_polygon*/
PG_FUNCTION_INFO_V1(geo_polygon_in);
PG_FUNCTION_INFO_V1(geo_polygon_out);

PG_FUNCTION_INFO_V1(geo_polygon_area);


#endif  /* __GEOEXT_H__ */
