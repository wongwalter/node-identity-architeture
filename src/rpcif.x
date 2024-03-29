/*
 * Copyright (c) 2001-2003 Regents of the University of California.
 * All rights reserved.
 *
 * See the file LICENSE included in this distribution for details.
 */

enum bamboo_stat {
  BAMBOO_OK = 0,
  BAMBOO_CAP = 1,
  BAMBOO_AGAIN = 2
};

typedef opaque bamboo_key[20]; 
typedef opaque bamboo_value<1024>; /* may be increased to 8192 eventually */
typedef opaque bamboo_placemark<100>;

struct bamboo_put_args {
  string application<255>;
  string client_library<255>;
  bamboo_key key;
  bamboo_value value;
  int ttl_sec;
};

struct bamboo_get_args {
  string application<255>;
  string client_library<255>;
  bamboo_key key;
  int maxvals;
  bamboo_placemark placemark;
};

struct bamboo_get_res {
    bamboo_value values<>;
    bamboo_placemark placemark;
};

program BAMBOO_DHT_GATEWAY_PROGRAM {
	version BAMBOO_DHT_GATEWAY_VERSION {
		void 
		BAMBOO_DHT_PROC_NULL (void) = 1;

	        bamboo_stat
		BAMBOO_DHT_PROC_PUT (bamboo_put_args) = 2;

                bamboo_get_res
		BAMBOO_DHT_PROC_GET (bamboo_get_args) = 3;
	} = 2;
} = 708655600;

