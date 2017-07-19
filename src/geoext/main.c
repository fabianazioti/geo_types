#include <stdio.h>
#include <stdlib.h>

typedef struct coord2d
{
  double x;
  double y;
} coord2d;

/* Um geo_point é um tipo de tamanho fixo passado por referência. */
typedef struct geo_point
{
  coord2d coord;
  int srid;
  int dummy;
} geo_point;


int main()
{
  int s = sizeof(geo_point);

  printf("%d\n", s);

  return 0;
}

