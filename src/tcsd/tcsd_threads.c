
/*
 * Licensed Materials - Property of IBM
 *
 * trousers - An open source TCG Software Stack
 *
 * (C) Copyright International Business Machines Corp. 2004-2006
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "trousers/tss.h"
#include "trousers_types.h"
#include "tcs_int_literals.h"
#include "tcs_tsp.h"
#include "tcs_utils.h"
#include "tcsd_wrap.h"
#include "tcsd.h"
#include "tcslog.h"
#include "rpc_tcstp_tcs.h"

struct tcsd_thread_mgr *tm = NULL;

TSS_RESULT
tcsd_threads_final()
{
	int rc;
	UINT32 i;

	MUTEX_LOCK(tm->lock);

	tm->shutdown = 1;

	MUTEX_UNLOCK(tm->lock);

	/* wait for all currently running threads to exit */
	for (i = 0; i < tm->max_threads; i++) {
		if (tm->thread_data[i].thread_id != THREAD_NULL) {
			if ((rc = THREAD_JOIN(*(tm->thread_data[i].thread_id), NULL))) {
				LogError("Thread join failed: error: %d", rc);
			}
		}
	}

	free(tm->thread_data);
	free(tm);

	return TSS_SUCCESS;
}

TSS_RESULT
tcsd_threads_init(void)
{
	/* allocate the thread mgmt structure */
	tm = calloc(1, sizeof(struct tcsd_thread_mgr));
	if (tm == NULL) {
		LogError("malloc of %zd bytes failed.", sizeof(struct tcsd_thread_mgr));
		return TCSERR(TSS_E_OUTOFMEMORY);
	}
	/* initialize mutex */
	MUTEX_INIT(tm->lock);

	/* set the max threads variable from config */
	tm->max_threads = tcsd_options.num_threads;

	/* allocate each thread's data structure */
	tm->thread_data = calloc(tcsd_options.num_threads, sizeof(struct tcsd_thread_data));
	if (tm->thread_data == NULL) {
		LogError("malloc of %zu bytes failed.",
			 tcsd_options.num_threads * sizeof(struct tcsd_thread_data));
		free(tm);
		return TCSERR(TSS_E_OUTOFMEMORY);
	}

	return TSS_SUCCESS;
}


TSS_RESULT
tcsd_thread_create(int socket, char *hostname)
{
	UINT32 thread_num = -1;
	int rc = TCS_SUCCESS;
#ifndef TCSD_SINGLE_THREAD_DEBUG
	THREAD_ATTR_DECLARE(tcsd_thread_attr);

	/* init the thread attribute */
	if ((rc = THREAD_ATTR_INIT(tcsd_thread_attr))) {
		LogError("Initializing thread attribute failed: error=%d: %s", rc, strerror(rc));
		rc = TCSERR(TSS_E_INTERNAL_ERROR);
		goto out;
	}
	/* make all threads joinable */
	if ((rc = THREAD_ATTR_SETJOINABLE(tcsd_thread_attr))) {
		LogError("Making thread attribute joinable failed: error=%d: %s", rc, strerror(rc));
		rc = TCSERR(TSS_E_INTERNAL_ERROR);
		goto out;
	}

	MUTEX_LOCK(tm->lock);
#endif
	if (tm->num_active_threads == tm->max_threads) {
		if (hostname != NULL) {
			LogError("max number of connections reached (%d), new connection"
				 " from %s refused.", tm->max_threads, hostname);
		} else {
			LogError("max number of connections reached (%d), new connection"
				 " refused.", tm->max_threads);
		}
		rc = TCSERR(TSS_E_CONNECTION_FAILED);
#ifndef TCSD_SINGLE_THREAD_DEBUG
		goto out_unlock;
#else
		goto out;
#endif
	}

	/* search for an open slot to store the thread data in */
	for (thread_num = 0; thread_num < tm->max_threads; thread_num++) {
		if (tm->thread_data[thread_num].thread_id == THREAD_NULL)
			break;
	}

	DBG_ASSERT(thread_num != tm->max_threads);

	tm->thread_data[thread_num].sock = socket;
	tm->thread_data[thread_num].context = NULL_TCS_HANDLE;
	if (hostname != NULL)
		tm->thread_data[thread_num].hostname = hostname;

#ifdef TCSD_SINGLE_THREAD_DEBUG
	(void)tcsd_thread_run((void *)(&(tm->thread_data[thread_num])));
#else
	tm->thread_data[thread_num].thread_id = calloc(1, sizeof(THREAD_TYPE));
	if (tm->thread_data[thread_num].thread_id == NULL) {
		rc = TCSERR(TSS_E_OUTOFMEMORY);
		LogError("malloc of %zd bytes failed.", sizeof(THREAD_TYPE));
		goto out_unlock;
	}

	if ((rc = THREAD_CREATE(tm->thread_data[thread_num].thread_id,
				 &tcsd_thread_attr,
				 tcsd_thread_run,
				 (void *)(&(tm->thread_data[thread_num]))))) {
		LogError("Thread create failed: %d", rc);
		rc = TCSERR(TSS_E_INTERNAL_ERROR);
		goto out_unlock;
	}

	tm->num_active_threads++;

out_unlock:
	MUTEX_UNLOCK(tm->lock);
#endif
out:
	/* cleanup in case of error */
	if (rc != TCS_SUCCESS) {
		if (hostname != NULL) {
			tm->thread_data[thread_num].hostname = NULL;
			free(hostname);
		}
		close(socket);
	}
	return rc;
}

/* Since we don't want any of the worker threads to catch any signals, we must mask off any
 * potential signals here after creating the threads.  If any of the created threads catch a signal,
 * they'd eventually call join on themselves, causing a deadlock.
 */
void
thread_signal_init()
{
	sigset_t thread_sigmask;
	int rc;

	if ((rc = sigfillset(&thread_sigmask))) {
		LogError("sigfillset failed: error=%d: %s", rc, strerror(rc));
		LogError("worker thread %ld is exiting prematurely", THREAD_ID);
		THREAD_EXIT(NULL);
	}

	if ((rc = THREAD_SET_SIGNAL_MASK(SIG_BLOCK, &thread_sigmask, NULL))) {
		LogError("Setting thread sigmask failed: error=%d: %s", rc, strerror(rc));
		LogError("worker thread %ld is exiting prematurely", THREAD_ID);
		THREAD_EXIT(NULL);
	}
}

void *
tcsd_thread_run(void *v)
{
	struct tcsd_thread_data *data = (struct tcsd_thread_data *)v;
	BYTE *buffer;
	int recd_so_far, empty_space, total_recv_size, recv_chunk_size, send_size;
	TSS_RESULT result;
	UINT64 offset;
#ifndef TCSD_SINGLE_THREAD_DEBUG
	int rc;

	thread_signal_init();
#endif

	data->comm.buf_size = TCSD_INIT_TXBUF_SIZE;
	data->comm.buf = calloc(1, data->comm.buf_size);
	while (data->comm.buf) {
		/* get the packet header to get the size of the incoming packet */
		if (recv_from_socket(data->sock, data->comm.buf,
				     sizeof(struct tcsd_packet_hdr)) < 0)
			break;

		recd_so_far = sizeof(struct tcsd_packet_hdr);

		/* check the packet size */
		total_recv_size = Decode_UINT32(data->comm.buf);
		if (total_recv_size < (int)sizeof(struct tcsd_packet_hdr)) {
			LogError("Packet to receive from socket %d is too small (%d bytes)",
				 data->sock, total_recv_size);
			break;
		}

		LogDebug("total_recv_size %d, buf_size %u, recd_so_far %d", total_recv_size,
			 data->comm.buf_size, recd_so_far);

		empty_space = data->comm.buf_size - recd_so_far;

		/* instead of blindly allocating recv_size bytes off the bat, stage the realloc
		 * and wait for the data to come in over the socket. This protects against
		 * trivially asking tcsd to alloc 2GB */
		while (total_recv_size > (int) data->comm.buf_size) {
			BYTE *new_buffer;
			int new_bufsize;

			if ((int)data->comm.buf_size + TCSD_INCR_TXBUF_SIZE < total_recv_size) {
				new_bufsize = data->comm.buf_size + TCSD_INCR_TXBUF_SIZE;
				recv_chunk_size = empty_space + TCSD_INCR_TXBUF_SIZE;
			} else {
				new_bufsize = total_recv_size;
				recv_chunk_size = total_recv_size - recd_so_far;
			}

			LogDebug("Increasing communication buffer to %d bytes.", new_bufsize);
			new_buffer = realloc(data->comm.buf, new_bufsize);
			if (new_buffer == NULL) {
				LogError("realloc of %d bytes failed.", new_bufsize);
				data->comm.buf = NULL;
				goto no_mem_error;
			}

			data->comm.buf_size = new_bufsize;
			data->comm.buf = new_buffer;
			buffer = data->comm.buf + recd_so_far;

			LogDebug("recv_chunk_size %d recd_so_far %d", recv_chunk_size, recd_so_far);
			if (recv_from_socket(data->sock, buffer, recv_chunk_size) < 0) {
				result = TCSERR(TSS_E_INTERNAL_ERROR);
				goto error;
			}

			recd_so_far += recv_chunk_size;
			empty_space = 0;
		}

		if (recd_so_far < total_recv_size) {
			buffer = data->comm.buf + recd_so_far;
			recv_chunk_size = total_recv_size - recd_so_far;

			LogDebug("recv_chunk_size %d recd_so_far %d", recv_chunk_size, recd_so_far);

			if (recv_from_socket(data->sock, buffer, recv_chunk_size) < 0) {
				result = TCSERR(TSS_E_INTERNAL_ERROR);
				goto error;
			}
		}
		LogDebug("Rx'd packet");

		/* create a platform version of the tcsd header */
		offset = 0;
		UnloadBlob_UINT32(&offset, &data->comm.hdr.packet_size, data->comm.buf);
		UnloadBlob_UINT32(&offset, &data->comm.hdr.u.result, data->comm.buf);
		UnloadBlob_UINT32(&offset, &data->comm.hdr.num_parms, data->comm.buf);
		UnloadBlob_UINT32(&offset, &data->comm.hdr.type_size, data->comm.buf);
		UnloadBlob_UINT32(&offset, &data->comm.hdr.type_offset, data->comm.buf);
		UnloadBlob_UINT32(&offset, &data->comm.hdr.parm_size, data->comm.buf);
		UnloadBlob_UINT32(&offset, &data->comm.hdr.parm_offset, data->comm.buf);

		result = getTCSDPacket(data);
error:
		if (result) {
			/* something internal to the TCSD went wrong in preparing the packet
			 * to return to the TSP.  Use our already allocated buffer to return a
			 * TSS_E_INTERNAL_ERROR return code to the TSP. In the non-error path,
			 * these LoadBlob's are done in getTCSDPacket().
			 */
			/* set everything to zero, fill in what is non-zero */
			memset(data->comm.buf, 0, data->comm.buf_size);
			offset = 0;
			/* load packet size */
			LoadBlob_UINT32(&offset, sizeof(struct tcsd_packet_hdr), data->comm.buf);
			/* load result */
			LoadBlob_UINT32(&offset, result, data->comm.buf);
		}
		send_size = Decode_UINT32(data->comm.buf);
		LogDebug("Sending 0x%X bytes back", send_size);
		send_size = send_to_socket(data->sock, data->comm.buf, send_size);
		if (send_size < 0)
			break;

		/* check for shutdown */
		if (tm->shutdown) {
			LogDebug("Thread %ld exiting via shutdown signal!", THREAD_ID);
			break;
		}
	}
no_mem_error:
	LogDebug("Thread exiting.");

	/* Closing connection to TSP */
	close(data->sock);
	data->sock = -1;
	free(data->comm.buf);
	data->comm.buf = NULL;
	data->comm.buf_size = -1;
	/* If the connection was not shut down cleanly, free TCS resources here */
	if (data->context != NULL_TCS_HANDLE) {
		TCS_CloseContext_Internal(data->context);
		data->context = NULL_TCS_HANDLE;
	}
	if(data->hostname != NULL) {
		free(data->hostname);
		data->hostname = NULL;
	}

#ifndef TCSD_SINGLE_THREAD_DEBUG
	pthread_mutex_lock(&(tm->lock));
	tm->num_active_threads--;
	/* if we're not in shutdown mode, then nobody is waiting to join this thread, so
	 * detach it so that its resources are free at pthread_exit() time. */
	if (!tm->shutdown) {
		if ((rc = pthread_detach(*(data->thread_id)))) {
			LogError("pthread_detach failed (errno %d)."
				 " Resources may not be properly released.", rc);
		}
	}
	free(data->thread_id);
	data->thread_id = THREAD_NULL;
	pthread_mutex_unlock(&(tm->lock));
	pthread_exit(NULL);
#endif
	return NULL;
}
