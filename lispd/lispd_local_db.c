/*
 * lispd_local_db.h
 *
 * This file is part of LISP Mobile Node Implementation.
 * Send registration messages for each database mapping to
 * configured map-servers.
 *
 * Copyright (C) 2011 Cisco Systems, Inc, 2011. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    Albert Lopez              <alopez@ac.upc.edu>
 *    Alberto Rodriguez Natal   <arnatal@ac.upc.edu>
 */



#include <netinet/in.h>
#include "lispd_external.h"
#include "lispd_lib.h"
#include "lispd_map_cache_db.h"


/*
 *  Patricia tree based databases
 */
patricia_tree_t *EIDv4_database           = NULL;
patricia_tree_t *EIDv6_database           = NULL;


/*
 *  Add a EID entry to the database.
 */
int add_mapping_to_db(lispd_mapping_elt *mapping);

patricia_node_t *lookup_eid_node(lisp_addr_t eid);

patricia_node_t *lookup_eid_exact_node(
        lisp_addr_t eid,
        int         eid_prefix_length);


/*
 * Initialize databases
 */

int db_init(void)
{
    EIDv4_database  = New_Patricia(sizeof(struct in_addr)  * 8);
    EIDv6_database  = New_Patricia(sizeof(struct in6_addr) * 8);

    if (!EIDv4_database || !EIDv6_database) {
        lispd_log_msg(LISP_LOG_CRIT, "db_init: Unable to allocate memory for database");
        return (BAD);
    };
    return (GOOD);
}

patricia_tree_t* get_local_db(int afi)
{
    if (afi == AF_INET)
        return EIDv4_database;
    else
        return EIDv6_database;
}


/*
 *  Add a mapping entry to the database.
 */
int add_mapping_to_db(lispd_mapping_elt *mapping)
{
    prefix_t            *prefix             = NULL;
    patricia_node_t     *node               = NULL;
    lisp_addr_t         eid_prefix;
    int                 eid_prefix_length;

    eid_prefix = mapping->eid_prefix;
    eid_prefix_length = mapping->eid_prefix_length;

    if ((node = malloc(sizeof(patricia_node_t))) == NULL) {
        lispd_log_msg(LISP_LOG_WARNING, "add_mapping_to_db: Unable to allocate memory for patrica_node_t: %s", strerror(errno));
        return(ERR_MALLOC);
    }

    switch(eid_prefix.afi) {
    case AF_INET:
        if ((prefix = New_Prefix(AF_INET, &(eid_prefix.address.ip), eid_prefix_length)) == NULL) {
            lispd_log_msg(LISP_LOG_WARNING, "add_mapping_to_db: Unable to allocate memory for prefix_t (AF_INET): %s", strerror(errno));
            free(node);
            return(ERR_MALLOC);
        }
        node = patricia_lookup(EIDv4_database, prefix);
        break;
    case AF_INET6:
        if ((prefix = New_Prefix(AF_INET6, &(eid_prefix.address.ipv6), eid_prefix_length)) == NULL) {
            lispd_log_msg(LISP_LOG_WARNING, "add_mapping_to_db: Unable to allocate memory for prefix_t (AF_INET): %s", strerror(errno));
            free(node);
            return(ERR_MALLOC);
        }
        node = patricia_lookup(EIDv6_database, prefix);
        break;
    default:
        free(node);
        lispd_log_msg(LISP_LOG_DEBUG_2, "add_mapping_to_db: Unknown afi (%d) when allocating prefix_t", eid_prefix.afi);
        return(ERR_AFI);
    }
    Deref_Prefix(prefix);

    if (node->data == NULL){            /* its a new node */
        node->data = (lispd_mapping_elt *) mapping;
        lispd_log_msg(LISP_LOG_DEBUG_2, "EID prefix %s/%d inserted in the database",
                get_char_from_lisp_addr_t(mapping->eid_prefix),
                mapping->eid_prefix_length);
        return (GOOD);
    }else{
        lispd_log_msg(LISP_LOG_DEBUG_2, "add_mapping_to_db: EID prefix entry (%s/%d) already installed in the data base",
                get_char_from_lisp_addr_t(eid_prefix),eid_prefix_length);
        return (BAD);
    }
}



patricia_node_t *lookup_eid_node(lisp_addr_t eid)
{
    patricia_node_t *node = NULL;
    prefix_t prefix;

    switch(eid.afi) {
    case AF_INET:
        prefix.family = AF_INET;
        prefix.bitlen = 32;
        prefix.ref_count = 0;
        prefix.add.sin.s_addr = eid.address.ip.s_addr;
        node = patricia_search_best(EIDv4_database, &prefix);
        break;
    case AF_INET6:
        prefix.family = AF_INET6;
        prefix.bitlen = 128;
        prefix.ref_count = 0;
        memcpy (&(prefix.add.sin6), &(eid.address.ipv6), sizeof(struct in6_addr));
        node = patricia_search_best(EIDv6_database, &prefix);
        break;
    default:
        break;
    }

    if ( node==NULL ){
        lispd_log_msg(LISP_LOG_DEBUG_3, "The entry %s is not a local EID", get_char_from_lisp_addr_t(eid));
    }
    return(node);
}

patricia_node_t *lookup_eid_exact_node(
        lisp_addr_t eid,
        int         eid_prefix_length)
{
    patricia_node_t *node = NULL;
    prefix_t        prefix;

    switch(eid.afi) {
    case AF_INET:
        prefix.family = AF_INET;
        prefix.bitlen = eid_prefix_length;
        prefix.ref_count = 0;
        memcpy (&(prefix.add.sin), &(eid.address.ip), sizeof(struct in_addr));
        node = patricia_search_exact(EIDv4_database, &prefix);
        break;
    case AF_INET6:
        prefix.family = AF_INET6;
        prefix.bitlen = eid_prefix_length;
        prefix.ref_count = 0;
        memcpy (&(prefix.add.sin6), &(eid.address.ipv6), sizeof(struct in6_addr));
        node = patricia_search_exact(EIDv6_database, &prefix);
        break;
    default:
        break;
    }

    if (node == NULL){
        lispd_log_msg(LISP_LOG_DEBUG_3, "The entry %s is not a local EID", get_char_from_lisp_addr_t(eid));
    }
    return(node);
}


/*
 * lookup_eid_in_db
 *
 * Look up a given eid in the database, returning the
 * lispd_mapping_elt of this EID if it exists or NULL.
 */
lispd_mapping_elt *lookup_eid_in_db(lisp_addr_t eid)
{
    lispd_mapping_elt       *mapping = NULL;
    patricia_node_t         *result     = NULL;

    result = lookup_eid_node(eid);
    if (result == NULL){
        return(NULL);
    }
    mapping = (lispd_mapping_elt *)(result->data);

    return(mapping);
}

/*
 * lookup_eid_in_db
 *
 *  Look up a given eid in the database, returning the
 * lispd_mapping_elt containing the exact EID if it exists or NULL.
 */
lispd_mapping_elt *lookup_eid_exact_in_db(lisp_addr_t eid_prefix, int eid_prefix_length)
{
    lispd_mapping_elt       *mapping = NULL;
    patricia_node_t         *result     = NULL;

    result = lookup_eid_exact_node(eid_prefix,eid_prefix_length);
    if (result == NULL){
        return(NULL);
    }
    mapping = (lispd_mapping_elt *)(result->data);

    return(mapping);
}



/*
 * del_mapping_entry_from_db()
 *
 * Delete an EID mapping from the data base
 */
void del_mapping_entry_from_db(
        lisp_addr_t eid,
        int prefixlen)
{
    lispd_mapping_elt    *entry     = NULL;
    patricia_node_t      *result    = NULL;

    result = lookup_eid_exact_node(eid, prefixlen);
    if (result == NULL){
        lispd_log_msg(LISP_LOG_WARNING,"del_mapping_entry_from_db: Unable to locate eid entry %s/%d for deletion",
                get_char_from_lisp_addr_t(eid),prefixlen);
        return;
    } else {
        lispd_log_msg(LISP_LOG_DEBUG_2,"Deleting EID entry %s/%d", get_char_from_lisp_addr_t(eid),prefixlen);
    }

    /*
     * Remove the entry from the trie
     */
    entry = (lispd_mapping_elt *)(result->data);
    if (eid.afi==AF_INET)
        patricia_remove(EIDv4_database, result);
    else
        patricia_remove(EIDv6_database, result);

    free_mapping_elt(entry);
}

/**
 * Returns a list with all the mappings in the database according to the specified afi
 * @param afi Indicate the mapping that should be added to the list (AF_INET, AF_INET6 or AF_UNSPEC for both)
 * @return List of mappings or NULL if no mapping matching the afi
 */
lisp_addr_t *get_main_eid(int afi)
{
    lisp_addr_t                 *eid        = NULL;
    lispd_mapping_elt           *entry      = NULL;
    patricia_tree_t             *database   = NULL;
    patricia_node_t             *node       = NULL;

    switch (afi){
    case AF_INET:
        database = EIDv4_database;
        break;
    case AF_INET6:
        database = EIDv6_database;
        break;
    }

    PATRICIA_WALK(database->head, node) {
        entry = ((lispd_mapping_elt *)(node->data));
        if (entry != NULL){
            eid = &(entry->eid_prefix);
            break;
        }
    }PATRICIA_WALK_END;

    return (eid);
}

/*
 * Returns a list with all the mappings in the database according to the specified afi
 * @param afi Indicate the mapping that should be added to the list (AF_INET, AF_INET6 or AF_UNSPEC for both)
 * @return List of mappings or NULL if no mapping matching the afi
 */
lispd_mapping_list *get_all_mappings(int afi)
{
    lispd_mapping_list          *list           = NULL;
    lispd_mapping_elt           *mapping        = NULL;
    patricia_node_t             *node           = NULL;
    patricia_tree_t             *database[2]    = {NULL,NULL};
    int                         ctr             = 0;

    switch (afi){
    case AF_INET:
        database[0] = EIDv4_database;
        break;
    case AF_INET6:
        database[1] = EIDv6_database;
        break;
    case AF_UNSPEC:
        database[0] = EIDv4_database;
        database[1] = EIDv6_database;
        break;
    default:
        return (NULL);
    }

    for (ctr = 0 ; ctr < 2 ; ctr++){
        if (database[ctr] == NULL){
            continue;
        }
        PATRICIA_WALK(database[ctr]->head, node) {
            mapping = ((lispd_mapping_elt *)(node->data));
            if (mapping != NULL){
                add_mapping_to_list(mapping,&list);
            }
        }PATRICIA_WALK_END;
    }
    return (list);
}

/*
 * Return the number of entries of the database
 */
int num_entries_in_db(patricia_tree_t *database){
    patricia_node_t             *node       = NULL;
    int                         ctr         = 0;

    PATRICIA_WALK(database->head, node) {
        ctr ++;
    }PATRICIA_WALK_END;

    return (ctr);
}

/*
 * dump the mapping list of the database
 */
void dump_local_db(int log_level)
{
    patricia_tree_t     *dbs [2] = {EIDv4_database, EIDv6_database};
    int                 ctr      = 0;
    patricia_node_t     *node    = NULL;
    lispd_mapping_elt   *entry   = NULL;

    if (is_loggable(log_level) == FALSE){
        return;
    }

    lispd_log_msg(log_level,"****************** LISP Local Mappings ****************\n");

    for (ctr = 0 ; ctr < 2 ; ctr++){
        PATRICIA_WALK(dbs[ctr]->head, node) {
            entry = ((lispd_mapping_elt *)(node->data));
            dump_mapping_entry(entry, log_level);
        } PATRICIA_WALK_END;
    }
    lispd_log_msg(log_level,"*******************************************************\n");
}

void drop_local_mappings()
{
	lispd_log_msg(LISP_LOG_DEBUG_3,"free_local_db: Releasing memory of local mappings\n");
	if (EIDv4_database != NULL)
		Destroy_Patricia(EIDv4_database,free_mapping_elt);
	if (EIDv6_database != NULL)
		Destroy_Patricia(EIDv6_database,free_mapping_elt);
	EIDv4_database = NULL;
	EIDv6_database = NULL;
}
