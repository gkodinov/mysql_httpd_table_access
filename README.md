A sample MySQLD component implementing a http server
using libmicrohttpd.

ATTENTION! THIS IS NOT PRODUCTION CODE! USE AT YOUR OWN RISK!
IT DOESN'T DO ANY AUTHENTICATION!

To compile:
1. fetch the mysql-8.0 source code: git clone https://github.com/mysql/mysql-server mysql-server
2. put the component source in components/httpd_table: cd mysql-server/components; git clone https://github.com/gkodinov/mysql_httpd_table_access.git httpd_table
3. procure libmicrohttpd and compile it or download the binaries from https://www.gnu.org/software/libmicrohttpd/
3. compile the server and the component, e.g. cmake -DWITH_MICROHTTPD=<path to libmicrohttpd binaries> ..
4. start mysqld with the right --plugin-dir containing the component binary
5. create the httpd.replies SQL table by running init.sql against the server
6. INSTALL COMPONENT "file://component_httpd_table";
7. Access the HTTP server at port 18080, e.g. http://localhost:18080?id=1
