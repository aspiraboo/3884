find_path(PostgreSQL_INCLUDE_DIRS libpq-fe.h   
   /usr/include/pgsql/
   /usr/local/include/pgsql/
   /usr/include/postgresql/
   /usr/local/opt/
   /usr/local/opt/libpq/include/
)

find_library(PostgreSQL_LIBRARIES NAMES pq libpq)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PostgreSQL DEFAULT_MSG
                                  PostgreSQL_INCLUDE_DIRS PostgreSQL_LIBRARIES )

mark_as_advanced(PostgreSQL_INCLUDE_DIRS PostgreSQL_LIBRARIES)