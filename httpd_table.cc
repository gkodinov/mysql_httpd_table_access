#include <microhttpd.h>
#include <mysql/components/component_implementation.h>
#include <mysql/components/services/table_access_service.h>
#include <stdio.h>
#include <stdlib.h>

REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_charset, charset_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_string_factory, string_factory_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_string_charset_converter,
                                string_converter_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(table_access_factory_v1, ta_factory_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(table_access_v1, ta_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(table_access_index_v1, ta_index_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(field_access_nullability_v1, fa_null_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(field_integer_access_v1, fa_integer_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(field_varchar_access_v1, fa_varchar_srv);

#define C_STR(x) x, sizeof(x) - 1

static size_t fill_table_data(int id, char *buf, size_t buf_size) {
  size_t retval = 0;
  TA_table tb = nullptr;
  Table_access ta = nullptr;
  TA_key pk = nullptr;
  my_h_string val = nullptr;
  bool is_null = false;
  size_t ticket = 0;
  static const TA_index_field_def pk_cols[] = {{C_STR("id"), false}};
  static const size_t pk_numcol = sizeof(pk_cols) / sizeof(TA_index_field_def);

  /* init table access */
  ta = ta_factory_srv->create(nullptr, 1);
  if (!ta) goto cleanup;

  /* add one table */
  ticket = ta_srv->add(ta, C_STR("httpd"), C_STR("replies"), TA_READ);

  /* begin transaction */
  if (ta_srv->begin(ta)) goto cleanup;

  /* set index access by primary key */
  tb = ta_srv->get(ta, ticket);
  if (!tb) goto cleanup;
  if (ta_index_srv->init(ta, tb, C_STR("PRIMARY"), pk_cols, pk_numcol, &pk))
    goto cleanup;

  /* set the key value to search for */
  if (fa_integer_srv->set(ta, tb, 0 /* column # */, id)) goto cleanup;

  /* search in the table */
  if (ta_index_srv->read_map(ta, tb, 1 /* key parts */, pk)) goto cleanup;

  /* check the data column for NULL */
  is_null = fa_null_srv->get(ta, tb, 1 /* column # */);

  if (!is_null) {
    /* fetch the string value */
    if (string_factory_srv->create(&val)) goto cleanup;
    if (fa_varchar_srv->get(ta, tb, 1 /* column # */, val)) goto cleanup;

    CHARSET_INFO_h utf8 = charset_srv->get_utf8mb4();
    if (string_converter_srv->convert_to_buffer(val, buf, buf_size,
                                                charset_srv->get_utf8mb4()))
      goto cleanup;
  } else {
    strncpy(buf, "<b>NULL<b>", buf_size);
    buf[buf_size - 1] = 0;
  }
  ta_srv->commit(ta);
  retval = strlen(buf);

cleanup:
  if (pk) ta_index_srv->end(ta, tb, pk);
  if (ta) ta_factory_srv->destroy(ta);
  if (val) string_factory_srv->destroy(val);
  return retval;
}

/* libhttpd stuff */
static enum MHD_Result ahc_echo(void *, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **ptr) {
  static int dummy = 0;
  struct MHD_Response *response;
  enum MHD_Result ret;

  if (0 != strcmp(method, "GET")) return MHD_NO; /* unexpected method */
  if (&dummy != *ptr) {
    /* The first time only the headers are valid,
       do not respond in the first round... */
    *ptr = &dummy;
    return MHD_YES;
  }
  if (0 != *upload_data_size) return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL;                               /* clear context pointer */

  /* we just do / */
  if (strcmp(url, "/")) return MHD_NO;

  /* fetch the table name */
  const char *id =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
  if (!id) {
    static char PAGE[] =
        "<html><head><title>libmicrohttpd demo</title>"
        "</head><body>Please supply the id as an argument</body></html>";
    response = MHD_create_response_from_buffer(strlen(PAGE), (void *)PAGE,
                                               MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }

  /* now read the table */
  char buffer[2048];
  size_t data_len = fill_table_data(atoi(id), buffer, sizeof(buffer));
  if (!data_len) return MHD_NO;
  response = MHD_create_response_from_buffer(data_len, &buffer[0],
                                             MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

static struct MHD_Daemon *d = nullptr;
static uint16_t port = 18080;

static mysql_service_status_t httpd_table_init() {
  d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL,
                       &ahc_echo, NULL, MHD_OPTION_END);
  return d == nullptr;
}
static mysql_service_status_t httpd_table_deinit() {
  if (d) MHD_stop_daemon(d);
  return 0;
}

BEGIN_COMPONENT_PROVIDES(httpd_table)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(httpd_table)
REQUIRES_SERVICE_AS(mysql_charset, charset_srv),
    REQUIRES_SERVICE_AS(mysql_string_factory, string_factory_srv),
    REQUIRES_SERVICE_AS(mysql_string_charset_converter, string_converter_srv),
    REQUIRES_SERVICE_AS(table_access_factory_v1, ta_factory_srv),
    REQUIRES_SERVICE_AS(table_access_v1, ta_srv),
    REQUIRES_SERVICE_AS(table_access_index_v1, ta_index_srv),
    REQUIRES_SERVICE_AS(field_access_nullability_v1, fa_null_srv),
    REQUIRES_SERVICE_AS(field_integer_access_v1, fa_integer_srv),
    REQUIRES_SERVICE_AS(field_varchar_access_v1, fa_varchar_srv),
    END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(httpd_table)
METADATA("mysql.author", "Oracle Corporation"),
    METADATA("mysql.license", "GPL"), METADATA("mysql.version", "1"),
    END_COMPONENT_METADATA();

DECLARE_COMPONENT(httpd_table, "mysql:httpd_table")
httpd_table_init, httpd_table_deinit END_DECLARE_COMPONENT();

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(httpd_table)
    END_DECLARE_LIBRARY_COMPONENTS