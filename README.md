# Spatial extension for PostgreSQL

pg_geoext is a spatial extension prototype for teaching spatial database class

### Building & installation
---
src/geoext
make
make install

### Usage
---


``` c
	CREATE EXTENSION geoext;
	CREATE TABLE t2
        (
  		c geo_linestring
	);

	INSERT INTO t2 VALUES(linestring_from_text('LINESTRING(0 0, 10 10)')),
                     (linestring_from_text('LINESTRING(11 11, 12 12, 13 13)'));

	SELECT * FROM t2;

	SELECT to_str(c) FROM t2;

	SELECT linestring_to_array(linestring_from_text('LINESTRING(11 11, 12 12, 13 13, 14 14, 15 15, 11 11)'));
```
### Types:

* POINT(31.3168034143746 29.5994637720287)
* LINESTRING(11 11, 12 12, 13 13)
* POLYGON((0 0, 10 0, 10 10, 0 10, 00))

### Functions:
```c

geo_point_from_text();
geo_point_to_str();
geo_point_distance()

geo_polygon_area();
geo_polygon_perimeter();
geo_polygon_contains_point();

geo_box_contain(box_from_text('BOX (7 7, 1 2)'), box_from_text('BOX (6 6, 2 2)'));

```

#### Observation:

The input and output  of geo_point is hex


