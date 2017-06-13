EXTENSION = geo_types       # the extensions name
DATA = geo_types--0.0.1.sql  # script files to install
MODULES = geo_types          # our c module file to build

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
