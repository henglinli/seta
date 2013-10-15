/*
 *  scheduler.c
 *  stealing-work scheduler
 *
 *  Created by Fabio Pricoco on 12/09/2013.
 *  Copyright 2013 Erlang Solutions Ltd. All rights reserved.
 *
 */

#include "scheduler.h"


void stats_and_print(processor_t *local_proc) {
	processor_lock_ready_queue(local_proc);
	processor_lock_stalled(local_proc);
	processor_evaluate_stats(local_proc);
	processor_print(local_proc);
	processor_unlock_stalled(local_proc);
	processor_unlock_ready_queue(local_proc);
}

void stack_depth_computation(processor_t *p, closure_t *cl, context_t *context) {
	int c1 = closure_space(cl);
	int c2 = context->allocated_ancients;
	int S1_candidate = c1 + c2;
	if (S1_candidate > p->stack_depth) {
		p->stack_depth = S1_candidate;
	}
	cl->allocated_ancients = context->allocated_ancients;
	context->allocated_ancients += c1;
}

void scheduler_execute_closure(processor_t *local_proc, closure_t *cl) {
	user_fun = cl->fun;
	context_t context;
	context.debug_list_arguments = NULL;
	context.is_last_thread = false;
	context.n_local_proc = local_proc->id;
	context.level = cl->level;
	context.allocated_ancients = cl->allocated_ancients;
	void *ptr = cl->args;
	if (cl->is_last_thread) {
		stop_computation = true;
	}
	closure_destroy(cl);
	user_fun(ptr, context);
}

void *scheduling_loop(void *ptr) {
	int n_local_proc = *(int *)ptr;
	processor_t *local_proc = processors[n_local_proc];
	logger_t local_logger = local_proc->logger;
	while (1) {
		processor_lock_ready_queue(local_proc);
		closure_t *cl = ready_queue_extract_head_from_deepest_level(local_proc->rq);
		processor_unlock_ready_queue(local_proc);
		if (cl != NULL) {
			//-- print to file
			msg_t msg = msg_new();
			istrcat(&msg, "-> closure extracted: \"");
			closure_str(&msg, cl);
			istrcatf(&msg, "\" from the level %d\n\n", cl->level);
			logger_write(local_logger, msg);
			msg_destroy(msg);
			//--
			
			scheduler_execute_closure(local_proc, cl);
			if (stop_computation) {
				break;
			}
		}
		else {
			if (stop_computation) {
				break;
			}

			int n_rand_proc = 0;
			do {
				n_rand_proc = rand() % NPROC;
			} while (n_rand_proc == n_local_proc);
			
			logger_write_format(local_logger, "-> try stealing from the processor %d...",
								n_rand_proc);
			
			processor_t *remote_proc = processors[n_rand_proc];
			processor_lock_ready_queue(remote_proc);
			closure_t *rcl = ready_queue_extract_tail_from_shallowest_level(remote_proc->rq);
			processor_unlock_ready_queue(remote_proc);
			
			if (rcl == NULL) {
				logger_write(local_logger, "failed.\n\n");
			}
			else {
				logger_write(local_logger, "ok.\n\n");
				
				//-- print to file
				msg_t msg = msg_new();
				istrcat(&msg, "-> closure remotely extracted: \"");
				closure_str(&msg, rcl);
				istrcatf(&msg, "\" from the level %d\n\n", rcl->level);
				logger_write(local_logger, msg);
				msg_destroy(msg);
				//--

				scheduler_execute_closure(local_proc, rcl);
			}
		}		
	}
	
	logger_write(local_logger, "\n\n//-------------------------------------//");
	logger_write_format(local_logger, "\nmemory_usage: %dB\n", local_proc->total_space);
	logger_write_format(local_logger, "\nstack_depth_partial: %dB\n", local_proc->stack_depth);
		
	return NULL;
}

void scheduler_print_results(logger_t logger) {
	int stack_depth = 0, total_space = 0;
	for (int i=0; i<NPROC; i++) {
		logger_write_format(logger, "processor %d memory usage: %dB\n",
							i, processors[i]->total_space);
		if (processors[i]->stack_depth > stack_depth) {
			stack_depth = processors[i]->stack_depth;
		}
		total_space += processors[i]->total_space;
	}
	logger_write_format(logger, "\nstack depth(S1): %dB\n\n", stack_depth);
	logger_write_format(logger, "all processors memory usage (SP): %dB\n\n", total_space);
	logger_write_format(logger, "S1 * P: %dB\n\n", stack_depth * NPROC);
	logger_write_format(logger, "%d(SP) < %d(S1 * P)\n\n", total_space, stack_depth * NPROC);
}


// ---------

cont create_cont(void *arg) {
	cont c;
	c.arg = arg;
	return c;
}

void connect_cont(handle_spawn_next *hsn, cont *c) {
	c->cl = hsn->cl;
	hsn->cl->join_counter += 1;
}

void assign_cont(cont *c1, cont c2) {
	c1->cl = c2.cl;
	c1->arg = c2.arg;
	c1->debug_n_arg = c2.debug_n_arg;
}

cont copy_cont(cont c) {
	cont r;
	r.cl = c.cl;
	r.arg = c.arg;
	r.debug_n_arg = c.debug_n_arg;
	return r;
}

handle_spawn_next prepare_spawn_next(void *fun, void *args, context_t *context) {
	handle_spawn_next hsn;
	closure_t *cl = closure_create(context->spawned);
	closure_set_fun(cl, fun);
	closure_set_args(cl, args);
	cl->proc = context->n_local_proc;
	cl->level = context->level;
	cl->args_size = context->args_size;
	stack_depth_computation(processors[0], cl, context);
	cl->is_last_thread = context->is_last_thread;
	if (context->debug_list_arguments != NULL) {
		closure_set_list_arguments(cl, context->debug_list_arguments);
		//cl->debug_list_arguments = context->debug_list_arguments;
		context->debug_list_arguments = NULL;
	}
	context->is_last_thread = false;
	hsn.cl = cl;
	return hsn;
}

void spawn_next(handle_spawn_next hsn) {
	processor_t *local_proc = processors[hsn.cl->proc];
	if (hsn.cl->join_counter == 0) {
		
		//-- print to file
		msg_t msg = msg_new();
		istrcat(&msg, "-> spawn next \"");
		closure_str(&msg, hsn.cl);
		istrcatf(&msg, "\" to level %d\n\n", hsn.cl->level);
		logger_write(local_proc->logger, msg);
		msg_destroy(msg);
		//--
		
		processor_lock_ready_queue(local_proc);
		ready_queue_post_closure_to_level(local_proc->rq, hsn.cl, hsn.cl->level);
		processor_unlock_ready_queue(local_proc);
	}
	else {
		//-- print to file
		msg_t msg = msg_new();
		istrcat(&msg, "-> stall \"");
		closure_str(&msg, hsn.cl);
		istrcat(&msg, "\"\n\n");
		logger_write(local_proc->logger, msg);
		msg_destroy(msg);
		//--
		
		processor_lock_stalled(local_proc);
		add_to_stalled(local_proc, hsn.cl);
		processor_unlock_stalled(local_proc);
	}
	
	stats_and_print(local_proc);
}

void spawn(void *fun, void *args, context_t *context) {
	closure_t *cl = closure_create(context->spawned);
	closure_set_fun(cl, fun);
	closure_set_args(cl, args);
	cl->level = context->level + 1;
	cl->args_size = context->args_size;
	
	if (context->debug_list_arguments != NULL) {
		//cl->debug_list_arguments = context->debug_list_arguments;
		closure_set_list_arguments(cl, context->debug_list_arguments);
		context->debug_list_arguments = NULL;
	}
	
	processor_t *local_proc = processors[context->n_local_proc];
	stack_depth_computation(local_proc, cl, context);
	
	//-- print to file
	msg_t msg = msg_new();
	istrcat(&msg, "-> spawn \"");
	closure_str(&msg, cl);
	istrcatf(&msg, "\" to level %d\n\n", cl->level);
	logger_write(local_proc->logger, msg);
	msg_destroy(msg);
	//--
	
	processor_lock_ready_queue(local_proc);
	ready_queue_post_closure_to_level(local_proc->rq, cl, cl->level);
	processor_unlock_ready_queue(local_proc);
	
	
	stats_and_print(local_proc);
}

void send_argument(cont c, void *src, int size, context_t *context) {
	void *dst = c.arg;
	memcpy(dst, src, size);
	
	closure_t *cl = c.cl;
	cl->join_counter -= 1;
	
	if (cl->join_counter == 0) {
		processor_t *local_proc = processors[context->n_local_proc];
		processor_t *target_proc = processors[cl->proc];
		
		if (local_proc == target_proc) {
			//-- print to file
			msg_t msg = msg_new();
			istrcat(&msg, "-> enable local closure \"");
			closure_str(&msg, cl);
			istrcatf(&msg, "\" to level %d\n\n", cl->level);
			logger_write(local_proc->logger, msg);
			msg_destroy(msg);		
			//--
		}
		else {
			//-- print to file
			msg_t msg = msg_new();
			istrcat(&msg, "-> enable remote closure \"");
			closure_str(&msg, cl);
			istrcatf(&msg, "\" on processor %d to level %d\n\n",
					 target_proc->id, cl->level);
			logger_write(local_proc->logger, msg);
			msg_destroy(msg);
			//--
		}		
		
		processor_lock_stalled(target_proc);
		remove_from_stalled(target_proc, cl);
		processor_unlock_stalled(target_proc);
		
		processor_lock_ready_queue(local_proc);
		ready_queue_post_closure_to_level(local_proc->rq, cl, cl->level);
		processor_unlock_ready_queue(local_proc);
		
		stats_and_print(local_proc);
	}
}

void debug_send_argument(cont c, char *arg) {
	closure_t *cl = c.cl;
	if (cl->list_arguments == NULL) {
		return;
	}
	char *str = (char *)dequeue_get_element(*(cl->list_arguments), c.debug_n_arg);
	strcpy(str, arg);
}

debug_list_arguments_t *debug_list_arguments_new() {
	return closure_create_list_arguments();
}

void debug_list_arguments_add(debug_list_arguments_t *debug_list_arguments, char *str) {
	closure_list_arguments_add(debug_list_arguments, str);
}



void scheduler_start(void *fun)
{	
	time_t t;
	srand((unsigned)time(&t));
	
	char filename[] = "scheduler.txt";
	logger = logger_create(filename);
	
	for (int i=0; i<NPROC; i++) {
		processors[i] = processor_create(i);
	}
	
	closure_t *cl = closure_create("start");
	closure_set_fun(cl, fun);
	
	processor_lock_ready_queue(processors[0]);
	ready_queue_post_closure_to_level(processors[0]->rq, cl, 0);
	processor_lock_ready_queue(processors[0]);
	
	stats_and_print(processors[0]);
	
	for (int i=0; i<NPROC; i++) {
		processor_start(processors[i]);
	}
	for (int i=0; i<NPROC; i++) {
		processor_wait_4_me(processors[i]);
	}
	
	for (int i=0; i<NPROC; i++) {
		processor_destroy(processors[i]);
	}
	
	scheduler_print_results(logger);
	
	logger_destroy(logger);
}
