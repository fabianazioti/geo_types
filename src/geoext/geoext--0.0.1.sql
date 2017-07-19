CREATE OR REPLACE FUNCTION public.geo_point_in(cstring)
    RETURNS geo_point
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_point_out(geo_point)
    RETURNS cstring
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_point_distance(geo_point, geo_point)
    RETURNS float8
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_point_from_text(cstring)
    RETURNS geo_point
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_point_to_str(geo_point)
    RETURNS cstring
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_polygon_in(cstring)
    RETURNS geo_polygon
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION public.geo_polygon_out(geo_polygon)
    RETURNS cstring
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;


CREATE TYPE geo_point(
    internallength = 24,
    input = geo_point_in,
    output = geo_point_out,
    alignment = double
);

CREATE TYPE geo_polygon(
    internallength = variable,
    input = geo_polygon_in,
    output = geo_polygon_out,
    storage = extended,
    alignment = double
);

CREATE FUNCTION geo_polygon_area(geo_polygon)
    RETURNS float8
    AS '$libdir/geoext'
    LANGUAGE C IMMUTABLE STRICT;
