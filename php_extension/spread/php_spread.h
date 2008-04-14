/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Yanbin Zhu <yanbin@staff.sina.com.cn>                        |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.10.8.1.4.1 2006/01/01 13:46:48 sniper Exp $ */

#ifndef PHP_SPREAD_H
#define PHP_SPREAD_H

#define SP_MAX_GROUP 100

extern zend_module_entry spread_module_entry;
#define phpext_spread_ptr &spread_module_entry

#ifdef PHP_WIN32
#define PHP_SPREAD_API __declspec(dllexport)
#else
#define PHP_SPREAD_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(spread);
PHP_MSHUTDOWN_FUNCTION(spread);
PHP_RINIT_FUNCTION(spread);
PHP_RSHUTDOWN_FUNCTION(spread);
PHP_MINFO_FUNCTION(spread);

/* declarations of functions to be exported */
PHP_FUNCTION(spread_connect);
PHP_FUNCTION(spread_pconnect);
PHP_FUNCTION(spread_multicast);
PHP_FUNCTION(spread_close);
PHP_FUNCTION(spread_error);
PHP_FUNCTION(spread_errno);
PHP_FUNCTION(spread_join);
PHP_FUNCTION(spread_receive);
PHP_FUNCTION(spread_leave);

/* Declare any global variables */

ZEND_BEGIN_MODULE_GLOBALS(spread)
	int default_conn;
	char *default_spread_name, *default_spread_private_name;
	long allow_persistent;
	int connect_errno;
	long connect_timeout;
	int default_service_type, default_mess_type;
ZEND_END_MODULE_GLOBALS(spread)

/* In every utility function you add that needs to use variables 
   in php_spread_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SPREAD_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SpG(v) TSRMG(spread_globals_id, zend_spread_globals *, v)
#else
#define SpG(v) (spread_globals.v)
#endif

#endif	/* PHP_SPREAD_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
