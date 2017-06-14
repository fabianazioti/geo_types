// #include "geo_decls.h"

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"		/* precisa para as funcoes de entrada e saida */

#include"utils/builtins.h"
#include"access/gist.h"
#include "access/stratnum.h"

#include<float.h>
#include<math.h>
#include"utils/array.h"

PG_MODULE_MAGIC;

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

PG_FUNCTION_INFO_V1(adiciona_pontos);
PG_FUNCTION_INFO_V1(distancia_euclidiana);
PG_FUNCTION_INFO_V1(polygon_area);
PG_FUNCTION_INFO_V1(polygon_perimeter);

PG_FUNCTION_INFO_V1(geo_polygon_in);
PG_FUNCTION_INFO_V1(geo_polygon_out);
PG_FUNCTION_INFO_V1(geo_polygon_recv);
PG_FUNCTION_INFO_V1(geo_polygon_send);

PG_FUNCTION_INFO_V1(polygon_has_point);
PG_FUNCTION_INFO_V1(polygon_has_points);



int dimension_count(char *s, char delim);
bool polygon_has_point_ve(Geo_Point *point, Geo_Polygon *geo_polygon);
double dist_euclidiana(double x1, double x2, double y1, double y2);


int dimension_count(char *s, char delim)
{
	int			ndelim = 0;

	while ((s = strchr(s, delim)) != NULL)
	{
		ndelim++;
		s++;
	}
  return ndelim;
}

bool polygon_has_point_ve(Geo_Point *geo_point, Geo_Polygon *geo_polygon){
	int i;
	double amag = 1;

	for(i = 0; i < geo_polygon->n_points; i++){
		if((geo_polygon->points[i].x == geo_point->x && geo_polygon->points[i].y == geo_point->y)){
			return amag = 0;
		}
	}
	return amag;

}

double dist_euclidiana(double x1, double x2, double y1, double y2){
  return sqrt(pow((x1 - x2),2) + pow(( y1 - y2),2));
}

Datum
geo_point_in(PG_FUNCTION_ARGS){
  char *str = PG_GETARG_CSTRING(0);
  double x, y;
  Geo_Point *result;

  if(sscanf(str, " ( %lf , %lf )", &x, &y)!= 2)
  		ereport(ERROR,
  				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
  				 errmsg("entrada errada para o tipo geo_point: \"%s\"",
  						str)));
  result = (Geo_Point *) palloc (sizeof(Geo_Point));
  result->x = x;
  result->y = y;
  PG_RETURN_POINTER(result);

}

Datum
geo_point_out(PG_FUNCTION_ARGS){
  Geo_Point *geo_point = (Geo_Point *) PG_GETARG_POINTER(0);
  char *result;

  result = psprintf("(%g,%g)", geo_point->x, geo_point->y);
  snprintf(result, 100, "(%g,%g)", geo_point->x, geo_point->y);
  PG_RETURN_CSTRING(result);

}

Datum
geo_point_send(PG_FUNCTION_ARGS)
{
  Geo_Point *geo_point = (Geo_Point *) PG_GETARG_POINTER(0);

	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendfloat8(&buf, geo_point->x);
	pq_sendfloat8(&buf, geo_point->y);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

Datum
geo_point_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
  Geo_Point *result;

  result = (Geo_Point *) palloc(sizeof(Geo_Point));
	result->x = pq_getmsgfloat8(buf);
	result->y = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
}

Datum
adiciona_pontos(PG_FUNCTION_ARGS){
  // SOMAR O TIPO POINT
  Geo_Point *a = (Geo_Point *) PG_GETARG_POINTER(0);
  Geo_Point *b = (Geo_Point *) PG_GETARG_POINTER(1);
  Geo_Point *result;

  result = (Geo_Point *) palloc(sizeof(Geo_Point));
	result->x = a->x + b->x;
	result->y = a->y + b->y;

	PG_RETURN_POINTER(result);
}
Datum
distancia_euclidiana(PG_FUNCTION_ARGS){
	Geo_Point *a = (Geo_Point *) PG_GETARG_POINTER(0);
  Geo_Point *b = (Geo_Point *) PG_GETARG_POINTER(1);

	// double dist_eu = sqrt(pow((a->x - b->x),2) + pow((a->y - b->y),2));
  double dist_eu = dist_euclidiana(a->x, b->x, a->y, b->y);
	PG_RETURN_FLOAT8 (dist_eu);

}

Datum
polygon_area(PG_FUNCTION_ARGS){
	Geo_Polygon *geo_polygon = (Geo_Polygon *) PG_GETARG_POINTER(0);
	int i;
	double area = 0;

	for(i = 0; i < geo_polygon->n_points; i++){
		area += (geo_polygon->points[i].x * geo_polygon->points[i+1].y )- (geo_polygon->points[i].y * geo_polygon->points[i + 1].x);
	}

	PG_RETURN_FLOAT8(fabs(area/2));

}

Datum
polygon_perimeter(PG_FUNCTION_ARGS){
  Geo_Polygon *geo_polygon = (Geo_Polygon *) PG_GETARG_POINTER(0);
  int i;
  double perimeter = 0;

  for(i = 1; i < geo_polygon->n_points; i++){
    perimeter += dist_euclidiana(geo_polygon->points[i-1].x,geo_polygon->points[i].x ,geo_polygon->points[i-1].y, geo_polygon->points[i].y);
  }
  perimeter += dist_euclidiana(geo_polygon->points[i-1].x,geo_polygon->points[0].x ,geo_polygon->points[i-1].y, geo_polygon->points[0].y);

  PG_RETURN_FLOAT8(perimeter);
}

Datum
polygon_has_point(PG_FUNCTION_ARGS){
	Geo_Polygon *geo_polygon = (Geo_Polygon *) PG_GETARG_POINTER(0);
	Geo_Point *geo_point = (Geo_Point *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(polygon_has_point_ve(geo_point,geo_polygon) == 0);
}

Datum
polygon_has_points(PG_FUNCTION_ARGS){
  Geo_Polygon *geo_polygon = (Geo_Polygon *) PG_GETARG_POINTER(0);
	// Geo_Point *geo_point = (Geo_Point *)palloc0(VARSIZE(PG_GETARG_POINTER(1)));
  // SET_VARSIZE(geo_point, VARSIZE(PG_GETARG_POINTER(1)));



}
// fazer a funcao com uma lista de pontos tipo exemplo do tex no doc
Datum
geo_polygon_in(PG_FUNCTION_ARGS){
  char *str = PG_GETARG_CSTRING(0);
  char *inicial, *final;
  char *aux;
  int i, dimension = 0, len = 0, base_size, size;
  int pos_point = 0;
  Geo_Polygon *result;
  double x, y;

  inicial = str;
  final = str;

  dimension = dimension_count(str, '(');

  base_size = sizeof(result->points[0]) * dimension;
	size = offsetof(Geo_Polygon, points) + base_size;

  result = (Geo_Polygon *) palloc0(size);	/* zero any holes */

	SET_VARSIZE(result, size);
	result->n_points = dimension;


  while (*str != '\0') {
    while (*final != ')') {
      final++;
    }
    len = (&final - &inicial);
    aux = (char *) malloc(len);
    i = 0;
    while (str != final) {
      aux[i]= *str;
      i++;
      str++;
    }
    if(*(str + 1) == ',')
      str+=2;
    else
      str++;

    final = str;

    if(sscanf(aux, " ( %lf , %lf )", &x, &y)!= 2)
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
             errmsg("entrada errada para o tipo geo_polygon ")));

    result->points[pos_point].x = x;
    result->points[pos_point].y = y;
    pos_point++;
    free(aux);

  }
  PG_RETURN_POINTER(result);


}

Datum
geo_polygon_out(PG_FUNCTION_ARGS){

  // Geo_Polygon *geo_polygon = PG_GETARG_GEO_POLY(0);
	Geo_Polygon *geo_polygon = (Geo_Polygon *) PG_GETARG_POINTER(0);
  int i, nding, extra_float_digits = 0;
  StringInfoData buf;

  initStringInfo(&buf);

  nding = DBL_DIG + extra_float_digits;
  if (nding < 1) {
    nding = 1;
  }


  for (i = 0; i < geo_polygon->n_points; i++){
    appendStringInfoChar(&buf, '(');
    appendStringInfo(&buf, "%.*g", nding, geo_polygon->points[i].x);
    appendStringInfoString(&buf, ", ");
    appendStringInfo(&buf, "%.*g", nding, geo_polygon->points[i].y);
    appendStringInfoChar(&buf, ')');
  }

  PG_FREE_IF_COPY(geo_polygon, 0);
  PG_RETURN_CSTRING(buf.data);


}

//Converte o tipo poligono para a representacao binaria
Datum
geo_polygon_send(PG_FUNCTION_ARGS){
	Geo_Polygon *geo_polygon = (Geo_Polygon *) PG_GETARG_POINTER(0);
	StringInfoData buf;
	int32 i;

	pq_begintypsend(&buf);
	pq_sendint(&buf, geo_polygon->n_points, sizeof(int32));
	for (i = 0; i < geo_polygon->n_points; i++) {
		pq_sendfloat8(&buf, geo_polygon->points[i].x);
		pq_sendfloat8(&buf, geo_polygon->points[i].y);
	}

	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

// conver representacao externa em binaria para Geo_Polygon
//
// Datum
// geo_polygon_recv(PG_FUNCTION_ARGS){
// 	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
// 	Geo_Polygon *geo_polygon;
// 	int32 nponts, i;
// 	int size;
//
// 	nponts = pg_getmsgint(buf, sizeof(int32));
//
// 	size = offsetof(Geo_Polygon, points) + sizeof(geo_polygon->points[0] * nponts);
// 	geo_polygon = (Geo_Polygon *) palloc(size);
// 	SET_VARSIZE(geo_polygon, size);
// 	// StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
// 	// 	Polygon3D  *poly;
// 	// 	int32		npts;
// 	// 	int32		i;
// 	// 	int			size;
// 	//
// 	// 	npts = pq_getmsgint(buf, sizeof(int32));
// 	// 	if (npts <= 0 || npts >= (int32) ((INT_MAX - offsetof(Polygon3D, p)) / sizeof(Point3D)))
// 	// 		ereport(ERROR,
// 	// 				(errcode(ERRCODE_INVALID_BINARY_REPRESENTATION),
// 	// 		  errmsg("Invalid number of points in external \"polygon3D\" value")));
// 	//
// 	// 	size = offsetof(Polygon3D, p) +sizeof(poly->p[0]) * npts;
// 	// 	poly = (Polygon3D *) palloc0(size);	/* zero any holes */
// 	//
// 	// 	SET_VARSIZE(poly, size);
// 	// 	poly->npts = npts;
// 	//
// 	// 	for (i = 0; i < npts; i++)
// 	// 	{
// 	// 		poly->p[i].x = pq_getmsgfloat8(buf);
// 	// 		poly->p[i].y = pq_getmsgfloat8(buf);
// 	// 		poly->p[i].z = pq_getmsgfloat8(buf);
// 	// 	}
// 	//
// 	// 	poly3D_make_bbox(poly);
// 	//
// 	// 	PG_RETURN_POLYGON3D_P(poly);
//
//
// }
