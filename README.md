# Spatial extension for PostgreSQL

geoext is an spatial extension for PostgreSQL to support geometries

### Status
---
### Requeriments
---
### Building & installation
---
### Usage
---
For more information read the doc

``` c
	CREATE EXTENSION geoext;
	SELECT geo_point_to_str('POINT (22.2 33.2);SRID = 4356'::geo_point)
	SELECT 'POLYGON ((1 2),(12 6 ),(5 9),(1 2));SRID = 4556'::geo_polygon
```
### Types:

* point(x,y)
* polygon((x1 y1), (x2 y3) .... (x1 y1))

### Functions:
```c
geo_point_to_str();
geo_point_from_text();
geo_point_distance();

geo_polygon_area();
geo_polygon_perimeter();
```

#### Observation:

The input and output  of geo_point is hex


