so:
	gcc -std=c99 -shared -o udf_renewal_date.so udf_renewal_date.c

install: so
	/bin/cp udf_renewal_date.so /usr/lib/mysql/plugin

clean:
	/bin/rm udf_renewal_date.so
