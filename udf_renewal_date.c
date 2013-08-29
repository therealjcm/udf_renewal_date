#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <mysql/mysql.h>


#define IPCED_DATE_STR_LEN 10

typedef struct
{
	int month;
	int day;
	int year;
	char date_str[IPCED_DATE_STR_LEN+1];
} ipced_date_t;

int
is_valid_ipced_date(ipced_date_t *v)
{
	if (v->day < 0 || v->day > 31 || v->month < 0 || v->month > 12) {
		return 0;
	}
	return 1;
}

int
is_numstring(char *the_string, int start, int len)
{
	for (int i = start; i < start+len; ++i) {
		if (!isdigit(the_string[i])) {
			return 0;
		}
	}
	return 1;
}

/* parse a string into a canonical string format yyyy-mm-dd
 * valid input formats are:
 *   yyyy-mm-dd
 *   mm-dd-yyyy, will attempt to re-parse as dd-mm-yyyy if mm > 12
 *   mmddyyyy
 */
ipced_date_t *
ipced_parse_date(char *the_date)
{
	ipced_date_t *rv = malloc(sizeof(ipced_date_t));
	int arg_len = strlen(the_date);
	if (arg_len == 8) {
		if (sscanf(the_date, "%2d%2d%4d", &(rv->month), &(rv->day), &(rv->year)) == 3) {
			// mmddyyyy format
			if (!is_valid_ipced_date(rv)) goto error_exit;
			sprintf(rv->date_str, "%d-%02d-%02d", rv->year, rv->month, rv->day);
			return rv;
		}
	} else if (arg_len == 10) {
		// yyyy-mm-dd or mm-dd-yyyy format
		if (sscanf(the_date, "%4d-%2d-%2d", &(rv->year), &(rv->month), &(rv->day)) == 3) {
			// yyyy-mm-dd format
			if (!is_valid_ipced_date(rv)) goto error_exit;
			sprintf(rv->date_str, "%d-%02d-%02d", rv->year, rv->month, rv->day);
			return rv;
		} else if (sscanf(the_date, "%2d-%2d-%4d", &(rv->month), &(rv->day), &(rv->year)) == 3) {
			// mm-dd-yyyy or dd-mm-yyyy format
			if (rv->month > 12) {
				// not valid as mm-dd-yyyy, attempt to parse as dd-mm-yyyy
				int tmp_month = rv->month;
				rv->month = rv->day;
				rv->day = tmp_month;
				if (!is_valid_ipced_date(rv)) goto error_exit;
			} else {
				if (!is_valid_ipced_date(rv)) goto error_exit;
			}
			sprintf(rv->date_str, "%d-%02d-%02d", rv->year, rv->month, rv->day);
			return rv;
		}
	}
	error_exit:
	free(rv);
	return NULL;
}

/* ctor */
my_bool
udf_renewal_date_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count != 1) {
		strcpy(message, "udf_echostr can only accept a single string arg");
		return 1;
	}
	if (args->arg_type[0] != STRING_RESULT) {
		strcpy(message, "udf_echostr arg must be string");
		return 1;
	}
	if (!args->args[0]) {
		strcpy(message, "udf_echostr arg must not be null");
		return 1;
	}

	ipced_date_t *ipced_date = ipced_parse_date(args->args[0]);
	if (!ipced_date) {
		strcpy(message, "udf_renewal_date format error");
		return 1;
	}
	initid->ptr = (char *)ipced_date;
	initid->max_length = IPCED_DATE_STR_LEN;

	return 0;

}

/* get string */
char *
udf_renewal_date(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *result_len, char *null_value, char *error)
{
	*result_len = IPCED_DATE_STR_LEN;
	ipced_date_t *rv = (ipced_date_t *)initid->ptr;
	return rv->date_str;
}

/* dtor */
void
udf_renewal_date_deinit(UDF_INIT *initid)
{
	free(initid->ptr);
}


