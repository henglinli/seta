/*
 *  test_main.c
 *  seta
 *
 *  Created by Fabio Pricoco on 14/10/2013.
 *  Copyright 2013 Erlang Solutions. All rights reserved.
 *
 */

#include "test_main.h"


/*
ready_queue_t * ready_queue_create();

void ready_queue_destroy(ready_queue_t *);

closure_t * ready_queue_extract_head_from_deepest_level(ready_queue_t *);

closure_t * ready_queue_extract_tail_from_shallowest_level(ready_queue_t *);

void ready_queue_post_closure_to_level(ready_queue_t *, closure_t *, int);

int ready_queue_space(ready_queue_t *);

void ready_queue_str(ready_queue_t *, char *);*/

void ready_queue_test() {
	
	ready_queue_t *rq = ready_queue_create();
	
	closure_t *closure = closure_create("pippo");
	ready_queue_post_closure_to_level(rq, closure, 2);

	closure_t *closure2 = closure_create("pluto");
	ready_queue_post_closure_to_level(rq, closure2, 1);

	closure_t *closure3 = closure_create("topolino");
	ready_queue_post_closure_to_level(rq, closure3, 2);
	
	dequeue_t *list_arguments1 = closure_create_list_arguments();
	closure_list_arguments_add(list_arguments1, "?");
	closure_list_arguments_add(list_arguments1, "?");
	closure_set_list_arguments(closure3, list_arguments1);
	
	closure_t *closure4 = closure_create("paperone");
	ready_queue_post_closure_to_level(rq, closure4, 3);	
	
	
	closure_t *cl_rem = ready_queue_extract_head_from_deepest_level(rq);
	
	msg_t msg = msg_new();
	ready_queue_str(&msg, rq);
	closure_str(&msg, cl_rem);
	printf("%s", msg);
	msg_destroy(msg);
	
	closure_t *cl_rem2 = ready_queue_extract_tail_from_shallowest_level(rq);
	
	msg_t msg2 = msg_new();
	ready_queue_str(&msg2, rq);
	closure_str(&msg2, cl_rem2);
	printf("%s", msg2);
	msg_destroy(msg2);
	
	
	ready_queue_destroy(rq);
}


int main() {
	printf("_________________test_________________\n\n");
	ready_queue_test();
}