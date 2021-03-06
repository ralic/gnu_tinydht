/***************************************************************************
 *  Copyright (C) 2007 by Saritha Kalyanam                                 *
 *  kalyanamsaritha@gmail.com                                              *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU Affero General Public License as         *
 *  published by the Free Software Foundation, either version 3 of the     *
 *  License, or (at your option) any later version.                        *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU Affero General Public License for more details.                    *
 *                                                                         *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "kbucket.h"

int 
kbucket_new(struct kbucket *k)
{
    ASSERT(k);

    bzero(k, sizeof(struct kbucket));
    LIST_INIT(&k->node_list);
    LIST_INIT(&k->ext_node_list);

    return SUCCESS;
}

int
kbucket_insert_node(struct kbucket *k, struct node *n, int max_nodes)
{
    ASSERT(k && n);

    if (kbucket_contains_node(k, n)) {
        return SUCCESS;
    }
   
    if (k->n_nodes < max_nodes) {
        LIST_INSERT_HEAD(&k->node_list, n, kb_next);
        k->n_nodes++;
        return SUCCESS;
    }

    LIST_INSERT_HEAD(&k->ext_node_list, n, kb_next);
    k->n_ext_nodes++;

    return SUCCESS;
}

struct node *
kbucket_delete_node(struct kbucket *k, struct node *n)
{
    struct node *tn = NULL, *tnn = NULL;
    struct node *xtn = NULL;

    ASSERT(k && n);

    /* search the kbucket's node list */
    LIST_FOREACH_SAFE(tn, &k->node_list, kb_next, tnn) {

        if (key_cmp(&tn->id, &n->id) == 0) {
            
            LIST_REMOVE(tn, kb_next);
            k->n_nodes--;

            if (k->n_ext_nodes == 0) {
                return tn;
            }

            /* move a node from the extended routing table to 
             * the main routing table */
            xtn = LIST_FIRST(&k->ext_node_list);
            LIST_REMOVE(xtn, kb_next);
            k->n_ext_nodes--;
            LIST_INSERT_HEAD(&k->node_list, xtn, kb_next);
            k->n_nodes++;
            return tn;
        }
    }

    /* also search the extended node list */
    LIST_FOREACH_SAFE(tn, &k->ext_node_list, kb_next, tnn) {
        if (key_cmp(&tn->id, &n->id) == 0) {
            LIST_REMOVE(tn, kb_next);
            k->n_ext_nodes--;
            return tn;
        }
    }

    return NULL;
}

int
kbucket_contains_node(struct kbucket *k, struct node *n)
{
    struct node *tn = NULL, *tnn = NULL;

    /* search the kbucket's node list */
    LIST_FOREACH_SAFE(tn, &k->node_list, kb_next, tnn) {
        if (key_cmp(&tn->id, &n->id) == 0) {
            return TRUE;
        }
    }

    /* also search the extended node list */
    LIST_FOREACH_SAFE(tn, &k->ext_node_list, kb_next, tnn) {
        if (key_cmp(&tn->id, &n->id) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

struct node *
kbucket_get_node(struct kbucket *k, struct key *key)
{
    struct node *tn = NULL, *tnn = NULL;

    ASSERT(k && key);

    /* search the kbucket's node list */
    LIST_FOREACH_SAFE(tn, &k->node_list, kb_next, tnn) {
        if (key_cmp(&tn->id, key) == 0) {
            return tn;
        }
    }

    /* also search the extended node list */
    LIST_FOREACH_SAFE(tn, &k->ext_node_list, kb_next, tnn) {
        if (key_cmp(&tn->id, key) == 0) {
            return tn;
        }
    }

    return NULL;
}

int
kbucket_index(struct key *self, struct key *k)
{
    struct key  xor;
    int         index;
    int         max_index;
    int         ret;

    ASSERT(self && k);
    
    ret = key_distance(self, k, &xor);

    max_index = k->len*8*sizeof(k->data[0]);
    ASSERT(max_index == 160);

    for (index = (max_index - 1); index >= 0; index--) {
        if (key_nth_bit(&xor, index) != 0) {
            break;
        }
    }

    return (max_index - 1) - index;
}
