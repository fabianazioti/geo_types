CREATE OR REPLACE FUNCTION public.geo_point_in(cstring)
    RETURNS geo_point
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_point_out(geo_point)
    RETURNS cstring
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION public.geo_point_recv(internal)
    RETURNS geo_point
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_point_send(geo_point)
    RETURNS bytea
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_polygon_in(cstring)
    RETURNS geo_polygon
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_polygon_out(geo_polygon)
    RETURNS cstring
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_polygon_send(geo_point)
    RETURNS bytea
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE geo_point(
  internallength = 20,
  input = geo_point_in,
  output = geo_point_out,
  receive = geo_point_recv,
  send = geo_point_send,
  alignment = double
);
--
CREATE TYPE geo_polygon(
	INTERNALLENGTH = variable,
	INPUT = geo_polygon_in,
	OUTPUT = geo_polygon_out,
	STORAGE = extended
);

CREATE FUNCTION adiciona_pontos(geo_point, geo_point)
    RETURNS geo_point
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR + (
    leftarg = geo_point,
    rightarg = geo_point,
    procedure = adiciona_pontos,
    commutator = +
  );

CREATE FUNCTION distancia_euclidiana(geo_point, geo_point)
    RETURNS float8
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION polygon_area(geo_polygon)
    RETURNS float8
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION polygon_has_point(geo_polygon, geo_point)
    RETURNS bool
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION polygon_perimeter(geo_polygon)
    RETURNS float8
    AS '$libdir/geo_types'
    LANGUAGE C IMMUTABLE STRICT;
