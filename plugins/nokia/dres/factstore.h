#ifndef __OHM_RESOLVER_FACTSTORE_H__
#define __OHM_RESOLVER_FACTSTORE_H__

static int  factstore_init(void);
static void factstore_exit(void);

static int  retval_to_facts(char ***objects, OhmFact **facts, int max);

static void set_fact(int cid, char *buf);



#endif /* __OHM_RESOLVER_FACTSTORE_H__ */



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

