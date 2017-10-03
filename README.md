# Spatial extension for PostgreSQL

pg_geoext is a simplified spatial extensionn for teaching spatial database class


### Building & installation
---
* src/geoext
* make
* make install

### Types:

* POINT(31.3168034143746 29.5994637720287)
* POLYGON((0 0, 10 0, 10 10, 0 10, 00))

### Functions:
```c

geo_point_from_text();
geo_point_to_str();
geo_point_distance()

geo_polygon_area();

```


