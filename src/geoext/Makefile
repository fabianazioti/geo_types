EXTENSION = geoext        # the extensions name
DATA = geoext--0.0.1.sql  # script files to install
MODULES = geoext          # our c module file to build

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
