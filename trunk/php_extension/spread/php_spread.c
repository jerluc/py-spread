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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
# include <winsock.h>
# define signal(a, b) NULL
#elif defined(NETWARE)
# include <sys/socket.h>
# define signal(a, b) NULL
#else
# if HAVE_SIGNAL_H
#  include <signal.h>
# endif
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <netdb.h>
# include <netinet/in.h>
# if HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif

#include "php.h"
#include "php_globals.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_spread.h"
#include "sp.h"
#include <sys/types.h>
#include <unistd.h>


ZEND_DECLARE_MODULE_GLOBALS(spread)

#define CHECK_LINK(link) { if (link==-1) { php_error_docref(NULL TSRMLS_CC, E_WARNING, "A link to the server could not be established"); RETURN_FALSE; } }
#define SAFE_STRING(s) ((s)?(s):"")
/* True global resources - no need for thread safety here */
static int le_spread_link, le_spread_plink;

/* {{{ spread_functions[]
 *
 * Every user visible function must have an entry in spread_functions[].
 */
zend_function_entry spread_functions[] = {
	PHP_FE(spread_connect, NULL)
	PHP_FE(spread_pconnect, NULL)
	PHP_FE(spread_multicast, NULL)
	PHP_FE(spread_error, NULL)
	PHP_FE(spread_errno, NULL)	
	PHP_FE(spread_close, NULL)
	PHP_FE(spread_join, NULL)
	PHP_FE(spread_receive, NULL)
	PHP_FE(spread_leave, NULL)
	{NULL, NULL, NULL}	/* Must be the last line in spread_functions[] */
};
/* }}} */

/* {{{ spread_module_entry
 */
zend_module_entry spread_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"spread",
	spread_functions,
	ZEND_MODULE_STARTUP_N(spread),
	PHP_MSHUTDOWN(spread),
	PHP_RINIT(spread),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(spread),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(spread),
#if ZEND_MODULE_API_NO >= 20010901
	"1.1.0", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SPREAD
ZEND_GET_MODULE(spread)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("spread.allow_persistent",	"1",	PHP_INI_SYSTEM,		OnUpdateInt,		allow_persistent,	zend_spread_globals,		spread_globals)
    STD_PHP_INI_ENTRY("spread.connect_timeout",		"60",	PHP_INI_ALL,		OnUpdateInt,		connect_timeout, 	zend_spread_globals,		spread_globals)
    STD_PHP_INI_ENTRY("spread.default_spread_name", "4803@localhost", PHP_INI_ALL, OnUpdateString, default_spread_name, zend_spread_globals, spread_globals)
    STD_PHP_INI_ENTRY("spread.default_spread_private_name", "spread", PHP_INI_ALL, OnUpdateString, default_spread_private_name, zend_spread_globals, spread_globals)
    STD_PHP_INI_ENTRY("spread.default_service_type", "2", PHP_INI_ALL, OnUpdateInt, default_service_type, zend_spread_globals, spread_globals)
    STD_PHP_INI_ENTRY("spread.default_mess_type", "1", PHP_INI_ALL, OnUpdateInt, default_mess_type, zend_spread_globals, spread_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_spread_init_globals
 */

static void php_spread_init_globals(zend_spread_globals *spread_globals)
{
	spread_globals->connect_timeout = 0;
	spread_globals->default_spread_name = NULL;	
	spread_globals->default_spread_private_name = NULL;
	spread_globals->default_service_type = 0;
	spread_globals->default_mess_type = 0;	
	spread_globals->connect_errno = 0;
}

/* }}} */

/* {{{ _close_spread_link
*/
static void _close_spread_link(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	mailbox *mbox = (mailbox *)rsrc->ptr;
	void (*handler) (int);
	handler = signal(SIGPIPE, SIG_IGN);
    SP_disconnect(*mbox);
	efree(mbox);
	signal(SIGPIPE, handler);
}

/* }}} */

/* {{{ _close_spread_plink
*/
static void _close_spread_plink(zend_rsrc_list_entry *rsrc TSRMLS_DC) 
{
	mailbox *mbox = (mailbox *)rsrc->ptr;	
	void (*handler) (int); 
	handler = signal(SIGPIPE, SIG_IGN);
    SP_disconnect(*mbox);
	free(mbox);
	signal(SIGPIPE, handler);
}
/* }}} */

/* {{{ php_spread_set_default_link
 */
static void php_spread_set_default_link(int id TSRMLS_DC)
{
	if (SpG(default_conn) != -1) {
		zend_list_delete(SpG(default_conn));
	}
	SpG(default_conn) = id;
	zend_list_addref(id);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
ZEND_MODULE_STARTUP_D(spread)
{
	ZEND_INIT_MODULE_GLOBALS(spread, php_spread_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	le_spread_link = zend_register_list_destructors_ex(_close_spread_link, NULL, "spread link", module_number);
	le_spread_plink = zend_register_list_destructors_ex(NULL, _close_spread_plink, "spread link persistent", module_number);
	Z_TYPE(spread_module_entry) = type;
	/* module version */
	REGISTER_STRING_CONSTANT("SPREAD_VERSION",  "1.1.0", CONST_CS | CONST_PERSISTENT);

	/* spread message config  */
	REGISTER_LONG_CONSTANT("SP_LOW_PRIORITY", LOW_PRIORITY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_MEDIUM_PRIORITY", MEDIUM_PRIORITY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_HIGH_PRIORITY", HIGH_PRIORITY, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("SP_UNRELIABLE_MESS", UNRELIABLE_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_RELIABLE_MESS", RELIABLE_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_FIFO_MESS", FIFO_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_CAUSAL_MESS", CAUSAL_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_AGREED_MESS", AGREED_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_SAFE_MESS", SAFE_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SP_REGULAR_MESS", REGULAR_MESS, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("REG_MEMB_MESS", REG_MEMB_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TRANSITION_MESS", TRANSITION_MESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CAUSED_BY_JOIN", CAUSED_BY_JOIN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CAUSED_BY_LEAVE", CAUSED_BY_LEAVE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CAUSED_BY_DISCONNECT", CAUSED_BY_DISCONNECT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CAUSED_BY_NETWORK", CAUSED_BY_NETWORK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MEMBERSHIP_MESS", MEMBERSHIP_MESS, CONST_CS | CONST_PERSISTENT);
	/* spread conf */
	REGISTER_LONG_CONSTANT("SP_MAX_GROUP_NAME", MAX_GROUP_NAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SP_MAX_PRIVATE_NAME", MAX_PRIVATE_NAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SP_MAX_GROUP", SP_MAX_GROUP, CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(spread)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(spread)
{
	SpG(default_conn) = -1;
	SpG(connect_errno) = 0;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(spread)
{
	SpG(connect_errno) = 0;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(spread)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "spread support", "enabled");
	php_info_print_table_end();
	
	DISPLAY_INI_ENTRIES();
	
}
/* }}} */

/* {{{ php_spread_do_connect (INTERNAL_FUNCTION_PARAMETERS, int persistent)
*/
static void php_spread_do_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	char *spread_name = NULL;
	char *private_name = NULL;
	char sp_private_name[32], sp_spread_name[128];
	mailbox *mbox = NULL;
	char private_group[MAX_GROUP_NAME];	
	sp_time conn_timeout;
	zval **z_spread_name = NULL, **z_private_name = NULL, **z_time_out=NULL;
	char *hashed_details;
    int hashed_details_length;
	list_entry *le;
	
	switch(ZEND_NUM_ARGS()) {
		case 0: 
				break;
				
		case 1: 
				if (zend_get_parameters_ex(1, &z_spread_name) == FAILURE) 
				{
					return;
				}				
				break;
				
		case 2:  
				if (zend_get_parameters_ex(2, &z_spread_name, &z_private_name) == FAILURE) 
				{ 
					return;
				} 
				break;
				
		case 3: 
				if (zend_get_parameters_ex(3, &z_spread_name, &z_private_name, &z_time_out) == FAILURE)
				{						
					return;						
				}
				break;
				
		default:
			WRONG_PARAM_COUNT;
	}	
	 
	if (z_spread_name)
	{
		SEPARATE_ZVAL(z_spread_name);
    	convert_to_string_ex(z_spread_name);
    	spread_name = (*z_spread_name)->value.str.val; 
	}
	else
	{
		snprintf(sp_spread_name,128, "%s", SpG(default_spread_name));
		spread_name = sp_spread_name; 
	}
	 
    if (z_private_name) 
	{
       	convert_to_string_ex(z_private_name);
       	snprintf(sp_private_name,MAX_PRIVATE_NAME,"%s",(*z_private_name)->value.str.val);
		private_name =  sp_private_name; 
    }
	else
	{
		snprintf(sp_private_name,MAX_PRIVATE_NAME,"%s",SpG(default_spread_private_name));
		private_name = sp_private_name; 
	}
	
	 
	if (z_time_out)
	{
		SEPARATE_ZVAL(z_time_out);
		convert_to_long_ex(z_time_out);
		conn_timeout.sec = (*z_time_out)->value.lval;	
	}
	else
	{
		conn_timeout.sec = SpG(connect_timeout);
	}
	
	conn_timeout.usec = 0;  
	
	hashed_details_length = sizeof("spread__") + strlen(SAFE_STRING(spread_name)) + strlen(SAFE_STRING(private_name));
    hashed_details = (char *) emalloc(hashed_details_length);
    snprintf(hashed_details, hashed_details_length, "spread_%s_%s", spread_name, private_name);
	if (!SpG(allow_persistent))
	{
		persistent = 0;
	}
	
	if (persistent) 
	{
		if (zend_hash_find(&EG(persistent_list), hashed_details, 
			hashed_details_length, (void **) &le) == FAILURE)
   		{
        	list_entry new_le;
        	int retval;
			mbox = (mailbox *) malloc(sizeof(mailbox));
			memset(private_group, 0, sizeof(private_group));
		
    		if ((retval = SP_connect_timeout(spread_name, private_name, 0, 
							0, mbox, private_group, conn_timeout)) != ACCEPT_SESSION)
    		{	
				zend_error(E_WARNING, "Failed to connect to spread daemon %s, error returned was: %d", spread_name, retval);
				SpG(connect_errno) = retval;
				free(mbox);
				efree(hashed_details);
				RETURN_FALSE;
    		}

			// set errno
			SpG(connect_errno) = 0;
			
			Z_TYPE(new_le) = le_spread_plink;
			new_le.ptr = mbox;
			
        	if (zend_hash_update(&EG(persistent_list), hashed_details, 
				hashed_details_length, (void *) &new_le, sizeof(list_entry),
            	NULL) == FAILURE)
        	{
				efree(hashed_details);
				RETURN_FALSE;
        	}
		}
		else 
		{
			// we have a pre-existing connection			
			if (le->type != le_spread_plink) 
			{            		
				efree(hashed_details);
				RETURN_FALSE;
        	}
			mbox = (mailbox *) le->ptr;	
    	}
	
		ZEND_REGISTER_RESOURCE(return_value, mbox,  le_spread_plink);
	}
	else
	// non persistent
	{
		if (zend_hash_find(&EG(regular_list), hashed_details, 
			hashed_details_length, (void **) &le) == SUCCESS)
		{
			int type;
			int link;
			void *ptr;
			
			link = (int) le->ptr;		
			// check if the link is still there
			ptr = zend_list_find(link,&type);   
			if (ptr && (type==le_spread_link|| type==le_spread_plink))
			{
				zend_list_addref(link);
				Z_LVAL_P(return_value) = link;
				php_spread_set_default_link(link TSRMLS_CC);
				Z_TYPE_P(return_value) = IS_RESOURCE;
				efree(hashed_details);
				return;
			} 
			else 
			{
				zend_hash_del(&EG(regular_list), hashed_details, hashed_details_length);
			}
		}
	
		list_entry new_le;
		
        int retval;
		mbox = (mailbox *) emalloc (sizeof(mailbox));
		memset(private_group, 0, sizeof(private_group));
		
    	if ((retval = SP_connect_timeout(spread_name, private_name, 0, 
						0, mbox, private_group, conn_timeout)) != ACCEPT_SESSION)
    	{	
			zend_error(E_WARNING, "Failed to connect to spread daemon %s, error returned was: %d", spread_name, retval);
			SpG(connect_errno) = retval;
			efree(mbox);
			efree(hashed_details);
			RETURN_FALSE;
    	}
		// set errno
		SpG(connect_errno) = 0;
		
		// add it to the list 
		ZEND_REGISTER_RESOURCE(return_value, mbox, le_spread_link);
	}
	efree(hashed_details);
	php_spread_set_default_link(Z_LVAL_P(return_value) TSRMLS_CC);
	return;
}
/* }}} */

/* {{{ php_spread_get_default_link
 */
static int php_spread_get_default_link(INTERNAL_FUNCTION_PARAMETERS)
{
	if (SpG(default_conn)==-1) { /* no link opened yet, implicitly open one */
		ht = 0;
		php_spread_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
	}
	return SpG(default_conn);
}
/* }}} */

/* {{{ proto resource spread_connect([string spreadname [, string privatename [, int timeout]]])
   Open a connect to a Spread Server */
PHP_FUNCTION(spread_connect)
{
	php_spread_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */
/* {{{ proto resource spread_pconnect([string spreadname [, string privatename [, int timeout]]])
   Open a connect to a Spread Server */
PHP_FUNCTION(spread_pconnect)
{
	php_spread_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
/* {{{ proto bool spread_join([string group [, resource link_identifier]])
   join a group(s) to a Spread Server */
PHP_FUNCTION(spread_join)
{
    zval **group=NULL, **mbox_zval=NULL;
    mailbox *mbox=NULL;
    mailbox default_mbox = -1;
    int sperrno;
	
    switch(ZEND_NUM_ARGS())
	{
		case 1:
				if (zend_get_parameters_ex(1, &group) == FAILURE)
				{
                	RETURN_FALSE;
            	}
				default_mbox = php_spread_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
				CHECK_LINK(default_mbox);
            	break;
        case 2:
            	if (zend_get_parameters_ex(2, &group, &mbox_zval) == FAILURE)
				{
                	RETURN_FALSE;
            	}
            	break;
        default:
            	WRONG_PARAM_COUNT;
            	break;
    }
    
    ZEND_FETCH_RESOURCE2(mbox, mailbox *, mbox_zval, default_mbox, "Spread-Link", le_spread_link, le_spread_plink);
    
    SEPARATE_ZVAL(group)
    if ((*group)->type == IS_STRING)
	{
		char *tmpgrp;
        convert_to_string_ex(group);
        tmpgrp = estrndup((*group)->value.str.val,(*group)->value.str.len);
        if ((sperrno = SP_join(*mbox, tmpgrp)) < 0)
		{
			zend_error(E_WARNING, "SP_join error(%d)", sperrno);
			SpG(connect_errno) = sperrno;
			efree(tmpgrp);
            RETURN_FALSE;
        }
		
		efree(tmpgrp);
		// set errno
		SpG(connect_errno) = 0;
		
		if (0 == sperrno)
		{
			RETURN_TRUE;
	 	}

	 	return_value->type = IS_LONG;
        return_value->value.lval = sperrno;
        return;
    }
    else if((*group)->type == IS_ARRAY)
	{
        HashPosition pos;
        zval *tmparr=NULL, **tmp=NULL;
        int n = 0;
        int error = 0;
        tmparr = *group;
        zend_hash_internal_pointer_reset_ex(tmparr->value.ht, &pos);
        while(zend_hash_get_current_data_ex(tmparr->value.ht, (void **) &tmp, 
				&pos) == SUCCESS 
			&& n < 100)
		{
            convert_to_string_ex(tmp);
            if ((sperrno = SP_join(*mbox,  (*tmp)->value.str.val)) < 0)
			{
                zend_error(E_WARNING, "SP_join error(%d)", sperrno);
                error = sperrno;
				SpG(connect_errno) = sperrno;
            }
            n++;
            zend_hash_move_forward_ex(tmparr->value.ht, &pos);
        }
		// set errno
		SpG(connect_errno) = 0;
        if (0 != error)
		{
            return_value->type = IS_LONG;
            return_value->value.lval = sperrno;
            return;
        } 
		else
		{
            RETURN_TRUE;
        }
    }
    else 
	{
        zend_error(E_WARNING, "SP_join reports inconsistent datatypes");
		RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool spread_leave([string group [, resource link_identifier]])
   leave a group(s) to a Spread Server */
PHP_FUNCTION(spread_leave)
{
	zval **group=NULL, **mbox_zval=NULL;
    mailbox *mbox=NULL;
    mailbox default_mbox = -1;
    int sperrno;
	
    switch(ZEND_NUM_ARGS())
	{
		case 1:
				if (zend_get_parameters_ex(1, &group) == FAILURE)
				{
                	RETURN_FALSE;
            	}
				default_mbox = php_spread_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
				CHECK_LINK(default_mbox);
            	break;
        case 2:
            	if (zend_get_parameters_ex(2, &group, &mbox_zval) == FAILURE)
				{
                	RETURN_FALSE;
            	}
            	break;
        default:
            	WRONG_PARAM_COUNT;
            	break;
    }
    
    ZEND_FETCH_RESOURCE2(mbox, mailbox *, mbox_zval, default_mbox, "Spread-Link", le_spread_link, le_spread_plink);
    
    SEPARATE_ZVAL(group)
    if ((*group)->type == IS_STRING)
	{
		char *tmpgrp;
        convert_to_string_ex(group);
        tmpgrp = estrndup((*group)->value.str.val,(*group)->value.str.len);
        if ((sperrno = SP_leave(*mbox, tmpgrp)) < 0)
		{
			zend_error(E_WARNING, "SP_leave error(%d)", sperrno);
			SpG(connect_errno) = sperrno;
			efree(tmpgrp);
            RETURN_FALSE;
        }

		efree(tmpgrp);
		// set errno
		SpG(connect_errno) = 0;
		
		if (0 == sperrno)
		{
			RETURN_TRUE;
	 	}

	 	return_value->type = IS_LONG;
        return_value->value.lval = sperrno;
        return;
    }
    else if((*group)->type == IS_ARRAY)
	{
        HashPosition pos;
        zval *tmparr, **tmp;
        int n = 0;
        int error = 0;
        tmparr = *group;
        zend_hash_internal_pointer_reset_ex(tmparr->value.ht, &pos);
        while(zend_hash_get_current_data_ex(tmparr->value.ht, (void **) &tmp, 
				&pos) == SUCCESS 
			&& n < 100)
		{
            convert_to_string_ex(tmp);
            if ((sperrno = SP_leave(*mbox,  (*tmp)->value.str.val)) < 0)
			{
                zend_error(E_WARNING, "SP_leave error(%d)", sperrno);
                error = sperrno;
				SpG(connect_errno) = sperrno;
            }
            n++;
            zend_hash_move_forward_ex(tmparr->value.ht, &pos);
        }

		// set errno
		SpG(connect_errno) = 0;
        if (0 != error)
		{
            return_value->type = IS_LONG;
            return_value->value.lval = sperrno;
            return;
        } 
		else
		{
            RETURN_TRUE;
        }
    }
    else 
	{
        zend_error(E_WARNING, "SP_leave reports inconsistent datatypes");
		RETURN_FALSE;
    }
}
/* }}} */
/* {{{ proto bool spread_receive([resource link_identifier [, long timeout]])
   receive a message from a Spread Server */
PHP_FUNCTION(spread_receive)
{
    zval **mbox_zval=NULL, **timeout_zval=NULL, **groups_zval=NULL;
	zval **sender_zval=NULL, **mess_type_zval=NULL;
    mailbox *mbox = NULL;
    double timeout;
    struct timeval towait;
    
    mailbox default_mbox = -1;
    int i, endmis, ret, ngrps, msize;
    int16 mtype;
    service stype;
    static int oldmsize = 0;
    static int newmsize = (1<<15);
    static char* mess=NULL;
    char sender[MAX_GROUP_NAME];
	static char groups[SP_MAX_GROUP][MAX_GROUP_NAME];
    fd_set readfs;

    switch(ZEND_NUM_ARGS())
	{
		case 0:	
				towait.tv_sec = 0;
				towait.tv_usec = 0;
				default_mbox = php_spread_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
				CHECK_LINK(default_mbox);
				break;
        case 1:
            	if (zend_get_parameters_ex(1, &mbox_zval) == FAILURE)
				{
                	RETURN_FALSE;
				}
            	towait.tv_sec = 0;
            	towait.tv_usec = 0;
            	break;
        case 2:
				if (zend_get_parameters_ex(2, &mbox_zval, &timeout_zval) == FAILURE)
				{
                	RETURN_FALSE;
            	}
            	convert_to_double_ex(timeout_zval);
            	timeout = (double)Z_DVAL_PP(timeout_zval);
            	towait.tv_sec = (unsigned long)timeout;
            	towait.tv_usec = (unsigned long)(1000000.0*(timeout-(double)towait.tv_sec));				
            	break;
		case 3:
				if (zend_get_parameters_ex(3, &mbox_zval, &timeout_zval, &groups_zval) == FAILURE)
				{
                	RETURN_FALSE;
            	}
            	convert_to_double_ex(timeout_zval);
            	timeout = (double)Z_DVAL_PP(timeout_zval);
            	towait.tv_sec = (unsigned long)timeout;
            	towait.tv_usec = (unsigned long)(1000000.0*(timeout-(double)towait.tv_sec));				
            	break;
		case 4:
				if (zend_get_parameters_ex(4, &mbox_zval, &timeout_zval, &groups_zval, &sender_zval) == FAILURE)
				{
                	RETURN_FALSE;
            	}
            	convert_to_double_ex(timeout_zval);
            	timeout = (double)Z_DVAL_PP(timeout_zval);
            	towait.tv_sec = (unsigned long)timeout;
            	towait.tv_usec = (unsigned long)(1000000.0*(timeout-(double)towait.tv_sec));				
            	break;
		case 5:
				if (zend_get_parameters_ex(5, &mbox_zval, &timeout_zval, &groups_zval, &sender_zval, &mess_type_zval) == FAILURE)
				{
                	RETURN_FALSE;
            	}
            	convert_to_double_ex(timeout_zval);
            	timeout = (double)Z_DVAL_PP(timeout_zval);
            	towait.tv_sec = (unsigned long)timeout;
            	towait.tv_usec = (unsigned long)(1000000.0*(timeout-(double)towait.tv_sec));				
            	break;
        default:
            	WRONG_PARAM_COUNT;
            	break;
    }

	if (groups_zval && PZVAL_IS_REF(*groups_zval))
	{
		zval_dtor(*groups_zval);		
		ZVAL_STRING(*groups_zval, "", 1);		
	}
	
	if (sender_zval && PZVAL_IS_REF(*sender_zval))
	{
		zval_dtor(*sender_zval);
		ZVAL_STRING(*sender_zval, "", 1);
	}

	if (mess_type_zval && PZVAL_IS_REF(*mess_type_zval))
	{
		zval_dtor(*mess_type_zval);
		ZVAL_LONG(*mess_type_zval, 0);
	}
   
    ZEND_FETCH_RESOURCE2(mbox, mailbox *, mbox_zval, default_mbox, "Spread-Link", le_spread_link, le_spread_plink);
    FD_ZERO(&readfs);
    FD_SET(*mbox, &readfs);
    if ((ret = select(*mbox+1, &readfs, NULL, &readfs, &towait))!=1)
	{
        RETURN_FALSE;
    }
	
    try_again: {
        if (oldmsize != newmsize)
		{
            if (mess)
			{
                mess = (char *) erealloc(mess, newmsize);
            } 
			else
			{ 
                mess = (char *) emalloc(newmsize);
            }
            oldmsize = newmsize;
        }

		memset(mess, 0, oldmsize);

        if ((ret=SP_receive(*mbox, &stype, sender, SP_MAX_GROUP, &ngrps, groups,
            &mtype, &endmis, newmsize, mess))<0)
        {
            if (ret==BUFFER_TOO_SHORT)
			{
                newmsize=-endmis;
                newmsize++;
                msize = oldmsize;
                goto try_again;
            }
        }
        msize = strlen(mess);
    }

	if (groups_zval && PZVAL_IS_REF(*groups_zval))
	{
		zval_dtor(*groups_zval);
		array_init(*groups_zval);
		for (i = 0; i < ngrps; i++)
		{
        	add_index_stringl(*groups_zval, i, groups[i], strlen(groups[i]), 1);
    	}
		
	}
	
	if (sender_zval && PZVAL_IS_REF(*sender_zval))
	{
		zval_dtor(*sender_zval);	
		ZVAL_STRINGL(*sender_zval, sender, strlen(sender), 1);
	}

	if (mess_type_zval && PZVAL_IS_REF(*mess_type_zval))
	{
		zval_dtor(*mess_type_zval);
		ZVAL_LONG(*mess_type_zval, mtype);
	}
	
	RETURN_STRINGL(mess, msize, 1);
}
/* }}} */
/* {{{ proto array spread_multicast([mixed group, string message [, resource link_identifier [, int mess_type, int service_type]]])
   Multicast a message to spread group(s), returns bytes sent */
PHP_FUNCTION(spread_multicast)
{	
    zval **group = NULL;
    zval **message = NULL;
    zval **mbox_zval = NULL;
    zval **mess_type_zval = NULL;
    zval **service_type_zval = NULL;
    mailbox  *mbox=NULL, default_mbox = -1;
    int service_type, mess_type;
    int sperrno;
					
	service_type = SpG(default_service_type);
	mess_type = SpG(default_mess_type);
	
    switch(ZEND_NUM_ARGS()) {
        case 2:
            if (zend_get_parameters_ex(2, &group, &message) == FAILURE)
			{
                RETURN_FALSE;
            }
			default_mbox = php_spread_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
			CHECK_LINK(default_mbox);
            break;
			
        case 3:
            if(zend_get_parameters_ex(3, &group, &message, &mbox_zval) == FAILURE) {
                RETURN_FALSE;
            }
            
            break;
			
        case 5:
            if(zend_get_parameters_ex(5,&group,                                        
                                      &message,
                                      &mbox_zval,
                                      &mess_type_zval,
                                      &service_type_zval) == FAILURE) 
            {
                RETURN_FALSE;
            }
			convert_to_long_ex(service_type_zval);
			convert_to_long_ex(mess_type_zval);
            service_type = Z_LVAL_PP(service_type_zval);
            mess_type = Z_LVAL_PP(mess_type_zval);
            break;
			
        default:
            WRONG_PARAM_COUNT;
            break;
    }
	
    SEPARATE_ZVAL(group)
    SEPARATE_ZVAL(message)
    
    ZEND_FETCH_RESOURCE2(mbox, mailbox *, mbox_zval, default_mbox, "Spread-Link", le_spread_link, le_spread_plink);
    convert_to_string_ex(message);
	if ((*group)->type == IS_STRING) 
	{
		char *tmpgrp;
    	convert_to_string_ex(group);
		tmpgrp = estrndup((*group)->value.str.val,(*group)->value.str.len);
		if ((sperrno = (SP_multicast(*mbox, service_type, 
							tmpgrp, mess_type, (*message)->value.str.len,
        					(*message)->value.str.val)) )<0)
    	{
    		efree(tmpgrp);
			zend_error(E_WARNING, "SP_mulicast error(%d)", sperrno);
			SpG(connect_errno) = sperrno;
			RETURN_FALSE;
    	}

		efree(tmpgrp);
		// set errno
		SpG(connect_errno) = 0;
		return_value->type = IS_LONG;
		return_value->value.lval = sperrno;
		return;
	}
	
	if ((*group)->type == IS_ARRAY) 
	{
		char groupnames[SP_MAX_GROUP][MAX_GROUP_NAME];
		HashPosition pos;
		zval *tmparr, **tmp;
		int n = 0;
		tmparr = *group;

		zend_hash_internal_pointer_reset_ex(tmparr->value.ht, &pos);
		while (zend_hash_get_current_data_ex(tmparr->value.ht, (void **) &tmp, &pos) == SUCCESS && n < SP_MAX_GROUP) 
		{
			convert_to_string_ex(tmp);
			strncpy(groupnames[n], (*tmp)->value.str.val, MAX_GROUP_NAME);
			n++;
			zend_hash_move_forward_ex(tmparr->value.ht, &pos);
		}

		if((sperrno = SP_multigroup_multicast(*mbox, service_type,
			n, (const char (*)[MAX_GROUP_NAME]) groupnames, mess_type, 
			(*message)->value.str.len, (*message)->value.str.val)) <0)
		{
			zend_error(E_WARNING, "SP_multicast error(%d)", sperrno);
			SpG(connect_errno) = sperrno;
            return;
        }
		
		return_value->type = IS_LONG;
		return_value->value.lval = sperrno;
        return;
    }
}
/* }}} */
/* {{{ proto string spread_error()
   Returns the text of the error message from previous Spread operation */
PHP_FUNCTION(spread_error)
{
	char err[256];
  	int error;
	
	memset(err, 0, sizeof(err));
	error = SpG(connect_errno);
	
	switch( error )
	{
		case 0:
			 break;
		case ILLEGAL_SPREAD:
			snprintf(err, sizeof(err), "SP_error: (%d) Illegal spread was provided\n", error);
			break;
		case COULD_NOT_CONNECT:
			snprintf(err,sizeof(err), "SP_error: (%d) Could not connect. Is Spread running?\n", error );
			break;
		case REJECT_QUOTA:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection rejected, to many users\n", error );
			break;
		case REJECT_NO_NAME:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection rejected, no name was supplied\n", error );
			break;
		case REJECT_ILLEGAL_NAME:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection rejected, illegal name\n", error );
			break;
		case REJECT_NOT_UNIQUE:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection rejected, name not unique\n", error );
			break;
		case REJECT_VERSION:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection rejected, library does not fit daemon\n", error );
			break;
		case CONNECTION_CLOSED:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection closed by spread\n", error );
			break;
		case REJECT_AUTH:
			snprintf(err,sizeof(err), "SP_error: (%d) Connection rejected, authentication failed\n", error );
			break;
		case ILLEGAL_SESSION:
			snprintf(err,sizeof(err), "SP_error: (%d) Illegal session was supplied\n", error );
			break;
		case ILLEGAL_SERVICE:
			snprintf(err,sizeof(err), "SP_error: (%d) Illegal service request\n", error );
			break;
		case ILLEGAL_MESSAGE:
			snprintf(err,sizeof(err), "SP_error: (%d) Illegal message\n", error );
			break;
		case ILLEGAL_GROUP:
			snprintf(err,sizeof(err), "SP_error: (%d) Illegal group\n", error );
			break;
		case BUFFER_TOO_SHORT:
			snprintf(err,sizeof(err), "SP_error: (%d) The supplied buffer was too short\n", error );
			break;
		case GROUPS_TOO_SHORT:
			snprintf(err,sizeof(err), "SP_error: (%d) The supplied groups list was too short\n", error );
			break;
		case MESSAGE_TOO_LONG:
			snprintf(err,sizeof(err), "SP_error: (%d) The message body + group names was too large to fit in a message\n", error );
			break;
		case NET_ERROR_ON_SESSION:
			snprintf(err,sizeof(err), "SP_error: (%d) The network socket experienced an error. This Spread mailbox will no longer work until the connection is disconnected and then reconnected\n", error );
			break;
		default:
			snprintf(err,sizeof(err), "SP_error: (%d) unrecognized error\n", error );
	}

	RETURN_STRING(err, 1);
}

/* }}} */
/* {{{ proto int spread_errno()
   Returns the number of the error message from previous Spread operation*/
PHP_FUNCTION(spread_errno)
{
	RETURN_LONG(SpG(connect_errno));	
}

/* }}} */
/* {{{ */
PHP_FUNCTION(spread_close)
{
	zval **spread_conn = NULL;
	mailbox *mbox;
	mailbox id;
	
	switch(ZEND_NUM_ARGS())
	{
		case 0:
			id = SpG(default_conn);
			break;
		case 1:
			if(zend_get_parameters_ex(1, &spread_conn) == FAILURE)
			{
				RETURN_FALSE;
			}
			id = -1;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	ZEND_FETCH_RESOURCE2(mbox, mailbox *, spread_conn, id, "Spread-Link", le_spread_link, le_spread_plink);
	
	if (id == -1)
	{
		zend_list_delete(Z_RESVAL_PP(spread_conn));
	}
	
	if(id != -1 || (spread_conn && Z_RESVAL_PP(spread_conn)==SpG(default_conn)))
	{
		zend_list_delete(SpG(default_conn));
		SpG(default_conn) = -1;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
