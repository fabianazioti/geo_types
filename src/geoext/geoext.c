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

/*!
 *
 * \file src/gbox_operations.c
 *
 * \brief Operations for GeoExt algorithms using in INDEX b-tree/r-tree.
 *
 * \author Gilberto Ribeiro de Queiroz
 * \author Fabiana Zioti
 *
 * \date 2017
 *
 * \copyright GNU Lesser Public License version 3
 */
/* PostgreSQL */
#include <postgres.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>

/* C Standard Library */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

/* GeoExtension */
#include "geoext.h"

#define GEO_POINT_SIZE (sizeof(geo_point) - sizeof(int))
#define HEXSTR_POINT_LEN ((2 * GEO_POINT_SIZE) + 1)

#define WKT_POINT_TOKEN "POINT"
#define WKT_POINT_TOKEN_MAXLEN 5

#define WKT_POLYGON_TOKEN "POLYGON"
#define WKT_POLYGON_TOKEN_MAXLEN 7

#define WKT_SRID_TOKEN "SRID"
#define WKT_SRID_TOKEN_MAXLEN 4

#define LDELIM '('
#define RDELIM ')'
#define DELIM	 ','

#define DOUBLE_MAXLEN (DBL_DIG + extra_float_digits)
#define INTEGER_MAXLEN 11
#define EWK_POINT_MAX_LEN (WKT_POINT_TOKEN_MAXLEN + \
                          sizeof(LDELIM) + \
                          DOUBLE_MAXLEN + \
                          sizeof(' ')+ \
                          DOUBLE_MAXLEN + \
                          sizeof(RDELIM)+ \
                          WKT_SRID_TOKEN_MAXLEN + \
                          sizeof('=') + \
                          INTEGER_MAXLEN + 1)


/* auxiliary functions */
inline int polygon_check(geo_polygon *polygon)
{
  int final_position = polygon->npts - 1;

  if (polygon->coords[0].x == polygon->coords[final_position].x && polygon->coords[0].y == polygon->coords[final_position].y) {
    return TRUE;
  }
  else
    return FALSE;
}
inline
long srid_decode(char *str, char *cp)
{
  int srid;

  /*Procura pela substring SRID */
  str = strcasestr(cp, WKT_SRID_TOKEN);
  if(!PointerIsValid(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("Invalid geo_point string: '%s', Missing SRID keyword.", cp)));


  /*avanca a substring SRID e possiveis caracteres em branco*/
  str += WKT_SRID_TOKEN_MAXLEN;
  while(isspace((unsigned char) *str))
    str++;

  /*temos que alcancar um = */
  if(*str != '=')
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                errmsg("Invalid geo_point string: '%s', Missing SRID separator = .", str)));

  /*avanca = e possiveis caracteres em brancos*/
  str++;
  while(isspace((unsigned char) *str))
    str++;

  srid = strtol(str, &cp, 10);

  if(cp <= str)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
              errmsg("Invalid string: '%s', could not parse integer value for SRID.", str)));

  return srid;
}
inline
void point_decode(char **str, coord2d *coord, char **cp)
{

  /* Encontra o delimitador ( */
  if(**cp != LDELIM)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                    errmsg("Invalid geo_point string: '%s', missing (.", *cp)));
  (*cp)++;

  while(isspace((unsigned char) **cp))
    (*cp)++;


  coord->x = strtod(*cp, str);

  if(*str <= *cp)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("Invalid geo_point string: '%s',could not parse for double value for x-cords.", *cp)));

  while(isspace((unsigned char) **str))
    (*str)++;

  coord->y = strtod(*str, cp);


  if(*cp <= *str)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("Invalid geo_point string: '%s',could not parse for double value for y-cords.", *str)));

  /*avanca os espacos em brancos apos as coordenadas*/
  while(isspace((unsigned char) **cp))
    (*cp)++;


  /*atinge o delimitaror )*/
  if(**cp != RDELIM)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("Invalid geo_point string: '%s', Missing ).", *cp)));

}
inline
inline int dimension_count(char *s)
{
	int			ndelim = 0;

	while ((s = strchr(s, DELIM)) != NULL)
	{
		ndelim++;
		s++;
	}
  return (ndelim + 1);
}

inline double euclidian_distance(geo_point *pt1, geo_point *pt2)
{
  double dx = pt1->coord.x - pt2->coord.x;

  double dy = pt1->coord.y - pt2->coord.y;

  double result = sqrt((dx * dx) + (dy * dy));

  return result;
}

static char ct[]={"0123456789ABCDEF" };

inline void
char2hex(unsigned char c, char *r)
{
  int h;  // high byte
  int l;  // low byte

  h = (c >> 4);
  l = (c & 0x0F);

  r[0] = ct[h];
  r[1] = ct[l];
}

inline void
binary2hex(const char *byte_str, int size, char *hex_str)
{
  int i = 0;

  for(; i < size; ++i)
    char2hex(byte_str[i], hex_str + (i * 2));

  hex_str[i * 2] = '\0';
}

inline void
hex2binary(const char *hex, int h_size, char *byte_str)
{
  int size = h_size / 2;

  char h, l;

  assert((h_size % 2) == 0);

  for(int i = 0; i < size; ++i)
  {
    char c = hex[i * 2];

    if(c >= '0' && c <= '9')
      h = c - 48;
    else
      h = c + 10 - 65;

    c = hex[i * 2 + 1];

    if((c >= '0') && (c <= '9'))
      l = c - 48;
    else
      l = c + 10 - 65;

    byte_str[i] = (h << 4) + l;
  }
}


/* GeoExtension Implementation */


PG_MODULE_MAGIC;

void _PG_init()
{
  elog(NOTICE, "GeoExtension initialized!");
}

void _PG_fini()
{
  elog(NOTICE, "GeoExtension finalized!");
}

Datum
geo_point_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);

  int size = strlen(str);

  geo_point *pt = (geo_point*) palloc(sizeof(geo_point));

  elog(NOTICE, "geo_point_in: %s", str);

  assert(size == 40);

  hex2binary(str, size, (char*)(pt));

  elog(NOTICE, "geo_point_in: hex2binary OK!");

  PG_RETURN_POINTER(pt);
}

Datum
geo_point_out(PG_FUNCTION_ARGS)
{
  geo_point *pt = PG_GETARG_GEOPOINT_P(0);

  char *hex_str = palloc(HEXSTR_POINT_LEN);

  binary2hex((char*)(pt), GEO_POINT_SIZE, hex_str);

  elog(NOTICE, "geo_point_out: %s", hex_str);

  PG_RETURN_CSTRING(hex_str);
}

Datum
geo_point_distance(PG_FUNCTION_ARGS)
{
	geo_point* pt1 = PG_GETARG_GEOPOINT_P(0);

  geo_point* pt2 = PG_GETARG_GEOPOINT_P(1);

  float8 dist;

  if(!PointerIsValid(pt1))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                   errmsg("First argument for distance between points is not valid.")));

  if(!PointerIsValid(pt2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                   errmsg("Second argument for distance between points is not valid.")));

  if(pt1->srid != pt2->srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                   errmsg("The point arguments have different SRIDs: %d e %d .", pt1->srid, pt2->srid)));

  elog(NOTICE, "geo_point_distance chamado");

  dist = euclidian_distance(pt1, pt2);

  elog(NOTICE, "geo_point_distance: %f", dist);

  PG_RETURN_FLOAT8(dist);
}

Datum
geo_point_from_text(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  geo_point *pt = (geo_point*) palloc(sizeof(geo_point));
  char *cp;

  if (!PointerIsValid(str))
    ereport(ERROR, (errcode (ERRCODE_INVALID_PARAMETER_VALUE),
                    errmsg("Missing geo_point string")));

  /*procura pela substring POINT*/
  cp = strcasestr(str, WKT_POINT_TOKEN);
  if (!PointerIsValid(cp))
    ereport(ERROR, (errcode (ERRCODE_INVALID_PARAMETER_VALUE),
                    errmsg("Invalid geo_point string '%s', Missing PONT keyword", str)));

  /*avanca substring POINT  e possiveis espacos*/
  cp += WKT_POINT_TOKEN_MAXLEN;
  while (isspace((unsigned char) *cp))
        cp++;

  //char *str, geo_point *p, char *cp
  point_decode(&str, &pt->coord, &cp);

  /*Avanca o ) e possiveis espacos*/
  cp++;
  while (isspace((unsigned char) *cp))
    cp++;

  /*Alcanca o ; e possiveis caracteres em branco*/
  if(*cp != ';')
    ereport(ERROR, (errcode (ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("Invalid geo_point string '%s', Missing SRID separator ;", cp)));
  cp++;

  while (isspace((unsigned char) *cp))
    cp++;

  pt->srid = srid_decode(str, cp);


  PG_RETURN_GEOPOINT_P(pt);
}

Datum
geo_point_to_str(PG_FUNCTION_ARGS)
{
  geo_point *pt = PG_GETARG_GEOPOINT_P(0);

  int ndig = DOUBLE_MAXLEN;

  char *str = palloc(EWK_POINT_MAX_LEN);

  int result = snprintf(str, EWK_POINT_MAX_LEN,
                        "POINT(%.*g %.*g);SRID=%d", ndig, pt->coord.x, ndig, pt->coord.y, pt->srid);

  if(result < 0)
    ereport(ERROR, (errcode (ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("geo_point_to_str produce a wrong text representation")));

  PG_RETURN_CSTRING(str);
}

Datum
geo_polygon_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  char *cp;
  geo_polygon *polyg;
  int npts, size, base_size, i = 0;

  /* verifica se o argumento informado é um endereço válido */
  if(!PointerIsValid(str))
    ereport(ERROR,
       (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Missing geo_polygon string")));

  // depois melhor a maneira de verificar a dimensao
  // no caso do usuario nao colocar nenhuma virgula mas o poligono ter mais de uma dim
  if ((npts = dimension_count(str)) <= 1)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			  errmsg("Invalid input syntax for geo_polygon: \"%s\"", str)));

  base_size = GEO_POINT_SIZE * npts;
  size = offsetof(geo_polygon, coords) + base_size;

  /* verifica overflow */
  if (base_size / npts != GEO_POINT_SIZE || size <= base_size)
    ereport(ERROR,
        (errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
        errmsg("Too many geo_points requested")));

  polyg = (geo_polygon *) palloc0(size);
  SET_VARSIZE(polyg, size);

  polyg->npts = npts;

  /*procura pela substring POLYGON*/
  cp = strcasestr(str, WKT_POLYGON_TOKEN);
  if (!PointerIsValid(cp))
    ereport(ERROR, (errcode (ERRCODE_INVALID_PARAMETER_VALUE),
                    errmsg("Invalid geo_polygon string '%s', missing POLYGON keyword", str)));

  /*avanca substring POLYGON  e possiveis espacos*/
  cp += WKT_POLYGON_TOKEN_MAXLEN;
  while (isspace((unsigned char) *cp))
    cp++;

  /* Encontra o delimitador ( */
  if(*cp != LDELIM)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                    errmsg("Invalid geo_polygon string: '%s', missing first ( ", cp)));

  /* avanca o primeiro delimitador */
  /* e encontra possiveis espacos entre  o primeiro e o segundo delimitador*/
  cp++;
  while (isspace((unsigned char) *cp))
    cp++;


  /*este while percorre o formato () , () */
  while (*cp != RDELIM) {

    //char *str, geo_point *p, char *cp
    point_decode(&str, &polyg->coords[i], &cp);

    /*Avanca o ) e possiveis espacos*/
    cp++;
    while (isspace((unsigned char) *cp))
      cp++;

    /*verifica se eh a vrigula e avanca*/
    if (*cp == DELIM){
      cp++;
    }
    /*avanca  possiveis espacos*/

    while (isspace((unsigned char) *cp))
      cp++;

    i++;
  }

  /*avanca o primeiro delimitador e possiveis espacos em branco */
  cp++;
  while (isspace((unsigned char) *cp))
    cp++;

  /*Alcanca o ; e possiveis caracteres em branco*/
  if(*cp != ';')
    ereport(ERROR, (errcode (ERRCODE_INVALID_PARAMETER_VALUE),
                  errmsg("Invalid geo_polygon string '%s', missing SRID separator ;", cp)));
  cp++;
  while (isspace((unsigned char) *cp))
    cp++;

  polyg->srid = srid_decode(str, cp);

  if (polygon_check(polyg) == FALSE) {
    pfree(polyg);
    polyg = NULL;
    elog(ERROR, "In a geo_polygon the first point must be the same as the last point");
  }

  elog(NOTICE, "geo_polygon in ok!");

  PG_RETURN_GEOPOLYGON_P(polyg);
}

Datum
geo_polygon_out(PG_FUNCTION_ARGS){

	geo_polygon *polyg = PG_GETARG_GEOPOLYGON_P(0);
  int i, nding;
  StringInfoData buf;

  initStringInfo(&buf);

  nding = DOUBLE_MAXLEN;

  appendStringInfoString(&buf, "POLYGON ");
  for (i = 0; i < polyg->npts; i++){
    appendStringInfoChar(&buf, '(');
    appendStringInfo(&buf, "%.*g", nding, polyg->coords[i].x);
    appendStringInfoString(&buf, " ");
    appendStringInfo(&buf, "%.*g", nding, polyg->coords[i].y);
    appendStringInfoString(&buf, ")");
  }
  appendStringInfo(&buf, ";SRID =  %d", polyg->srid);

  PG_RETURN_CSTRING(buf.data);

}

Datum
geo_polygon_area(PG_FUNCTION_ARGS){
  geo_polygon *polyg = PG_GETARG_GEOPOLYGON_P(0);
  int i;
  double area = 0 ;

  for (i = 0; i < polyg->npts; i++) {
    /* code */
    area += (polyg->coords[i].x * polyg->coords[i+1].y )- (polyg->coords[i].y * polyg->coords[i + 1].x);
  }

  PG_RETURN_FLOAT8(fabs(area/2));

}
