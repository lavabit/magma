
/**
 * @file /magma/objects/warehouse/warehouse.h
 *
 * @brief	Functions to provide access to warehoused reference data needed to make intelligent decisions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_WAREHOUSE_H
#define MAGMA_OBJECTS_WAREHOUSE_H

/// HIGH: Create an interface for loading SSL/TLS and DKIM certificates from the database.
/// HIGH: Create an interface for checking whether an address or IP is trusted.

typedef struct {
	int_t spf;
	int_t dkim;
	int_t wildcard;
	int_t mailboxes;
	int_t restricted;
	stringer_t *domain;
} __attribute__((__packed__)) domain_t;

/// datatier.c
inx_t *  warehouse_fetch_domains(void);
inx_t *  warehouse_fetch_patterns(void);

/// domains.c
domain_t *  domain_alloc(stringer_t *domain, int_t restricted, int_t mailboxes, int_t wildcard, int_t dkim, int_t spf);
int_t       domain_dkim(stringer_t *domain);
int_t       domain_mailboxes(stringer_t *domain);
int_t       domain_restricted(stringer_t *domain);
int_t       domain_spf(stringer_t *domain);
bool_t      domain_start(void);
void        domain_stop(void);
int_t       domain_wildcard(stringer_t *domain);

/// patterns.c
int_t    pattern_check(stringer_t *message);
bool_t   pattern_start(void);
void     pattern_stop(void);
void     pattern_update(void);

/// warehouse.c
bool_t warehouse_start(void);
void warehouse_stop(void);
void warehouse_update(void);

#endif
