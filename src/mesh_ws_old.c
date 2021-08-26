#include "libwebsockets.h"
#include "string.h"
#include "signal.h"
#include "../include/data_solve.h"
#define LWS_PLUGIN_STATIC
#define RING_DEPTH 4096


/* one of these created for each message */

struct msg {
	void *payload; /* is malloc'd */
	size_t len;
	char binary;
	char first;
	char final;
};

struct per_session_data__minimal_server_echo {
	struct lws_ring *ring;
	uint32_t msglen;
	uint32_t tail;
	uint8_t completed:1;
	uint8_t flow_controlled:1;
	uint8_t write_consume_pending:1;
};

struct vhd_minimal_server_echo {
	struct lws_context *context;
	struct lws_vhost *vhost;

	int *interrupted;
	int *options;
};

static void
__minimal_destroy_message(void *_msg)
{
	struct msg *msg = _msg;

	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}
#include <assert.h>
pid_t pid = -1;
static int callback_minimal_server_echo(struct lws *wsi, enum lws_callback_reasons reason,
										void *user, void *in, size_t len)
{
	struct per_session_data__minimal_server_echo *pss =
			(struct per_session_data__minimal_server_echo *)user;
	struct vhd_minimal_server_echo *vhd = (struct vhd_minimal_server_echo *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi),
				lws_get_protocol(wsi));
	const struct msg *pmsg;
	struct msg amsg;
	int m, n, flags;

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		printf("lws init\n");
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
										  lws_get_protocol(wsi),
										  sizeof(struct vhd_minimal_server_echo));
		if (!vhd)
			return -1;

		vhd->context = lws_get_context(wsi);
		vhd->vhost = lws_get_vhost(wsi);

		/* get the pointers we were passed in pvo */

		vhd->interrupted = (int *)lws_pvo_search(
			(const struct lws_protocol_vhost_options *)in,
			"interrupted")->value;
		vhd->options = (int *)lws_pvo_search(
			(const struct lws_protocol_vhost_options *)in,
			"options")->value;
		break;

	case LWS_CALLBACK_ESTABLISHED:
		// if(pid != 0){
			// pid = fork();
			// if(pid == 0){
				/* generate a block of output before travis times us out */
				lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");
				pss->ring = lws_ring_create(sizeof(struct msg), RING_DEPTH,
								__minimal_destroy_message);
				if (!pss->ring)
					return 1;
				pss->tail = 0;
				
			// }
		// }
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		// if(pid == 0){
			lwsl_user("LWS_CALLBACK_SERVER_WRITEABLE\n");

			if (pss->write_consume_pending) {
				/* perform the deferred fifo consume */
				lws_ring_consume_single_tail(pss->ring, &pss->tail, 1);
				pss->write_consume_pending = 0;
			}

			pmsg = lws_ring_get_element(pss->ring, &pss->tail);
			if (!pmsg) {
				lwsl_user(" (nothing in ring)\n");
				break;
			}

			flags = lws_write_ws_flags(
					pmsg->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
					pmsg->first, pmsg->final);

			/* notice we allowed for LWS_PRE in the payload already */
			m = lws_write(wsi, ((unsigned char *)pmsg->payload) +
					LWS_PRE, pmsg->len, (enum lws_write_protocol)flags);
			if (m < (int)pmsg->len) {
				lwsl_err("ERROR %d writing to ws socket\n", m);
				return -1;
			}

			lwsl_user(" wrote %d: flags: 0x%x first: %d final %d\n",
					m, flags, pmsg->first, pmsg->final);
			/*
			* Workaround deferred deflate in pmd extension by only
			* consuming the fifo entry when we are certain it has been
			* fully deflated at the next WRITABLE callback.  You only need
			* this if you're using pmd.
			*/
			pss->write_consume_pending = 1;
			lws_callback_on_writable(wsi);

			if (pss->flow_controlled &&
				(int)lws_ring_get_count_free_elements(pss->ring) > RING_DEPTH - 5) {
				lws_rx_flow_control(wsi, 1);
				pss->flow_controlled = 0;
			}

			if ((*vhd->options & 1) && pmsg && pmsg->final)
				pss->completed = 1;
		// }
		break;
		
	case LWS_CALLBACK_RECEIVE:
		//printf("%s\r\n",in);
		// if(pid == 0){
			lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, "
					"last %d, bin %d, msglen %d (+ %d = %d))\n",
					(int)len, (int)lws_remaining_packet_payload(wsi),
					lws_is_first_fragment(wsi),
					lws_is_final_fragment(wsi),
					lws_frame_is_binary(wsi), pss->msglen, (int)len,
					(int)pss->msglen + (int)len);

			if (len) {
				char in_data[5000] = {0}, out_data[5000] = {0};
				memcpy(in_data, in, len);
				ws_data_Parse_Json(in_data, out_data);
				printf("%s\n%s\n", in_data, out_data);

				amsg.first = (char)lws_is_first_fragment(wsi);
				amsg.final = (char)lws_is_final_fragment(wsi);
				amsg.binary = (char)lws_frame_is_binary(wsi);
				n = (int)lws_ring_get_count_free_elements(pss->ring);
				if (!n) {
					lwsl_user("dropping!\n");
					break;
				}
				int send_len = strlen(out_data);
				if (amsg.final)
					pss->msglen = 0;
				else
					pss->msglen += (uint32_t)send_len;

				amsg.len = send_len;
				/* notice we over-allocate by LWS_PRE */
				amsg.payload = malloc(LWS_PRE + send_len);
				if (!amsg.payload) {
					lwsl_user("OOM: dropping\n");
					break;
				}

				memcpy((char *)amsg.payload + LWS_PRE, out_data, send_len);
				if (!lws_ring_insert(pss->ring, &amsg, 1)) {
					__minimal_destroy_message(&amsg);
					lwsl_user("dropping!\n");
					break;
				}
				lws_callback_on_writable(wsi);			
			}
			if (n < 3 && !pss->flow_controlled) {
				pss->flow_controlled = 1;
				lws_rx_flow_control(wsi, 0);
			}
		// }
		break;
	case LWS_CALLBACK_CLOSED:
		if(pid == 0){
			lwsl_user("LWS_CALLBACK_CLOSED\n");
			lws_ring_destroy(pss->ring);

			if (*vhd->options & 1) {
				if (!*vhd->interrupted)
					*vhd->interrupted = 1 + pss->completed;
				lws_cancel_service(lws_get_context(wsi));
			}
			exit(0);
		}
		break;

	default:
		break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL_SERVER_ECHO \
	{ \
		"lws-minimal-server-echo", \
		callback_minimal_server_echo, \
		sizeof(struct per_session_data__minimal_server_echo), \
		1024, \
		0, NULL, 0 \
	}


static struct lws_protocols protocols[] = {
	LWS_PLUGIN_PROTOCOL_MINIMAL_SERVER_ECHO,
	LWS_PROTOCOL_LIST_TERM
};

static int interrupted, port = 7681, options;

/* pass pointers to shared vars to the protocol */

static const struct lws_protocol_vhost_options pvo_options = {
	NULL,
	NULL,
	"options",		/* pvo name */
	(void *)&options	/* pvo value */
};

static const struct lws_protocol_vhost_options pvo_interrupted = {
	&pvo_options,
	NULL,
	"interrupted",		/* pvo name */
	(void *)&interrupted	/* pvo value */
};

static const struct lws_protocol_vhost_options pvo = {
	NULL,				/* "next" pvo linked-list */
	&pvo_interrupted,		/* "child" pvo linked-list */
	"lws-minimal-server-echo",	/* protocol name we belong to on this vhost */
	""				/* ignored */
};


void sigint_handler(int sig)
{
	interrupted = 1;
}
int mesh_ws_init(int port){
struct lws_context_creation_info info;
	struct lws_context *context;
	const char *p;
	int n = 100, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
			/* for LLL_ verbosity above NOTICE to be built into lws,
			 * lws must have been configured and built with
			 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
			/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
			/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
			/* | LLL_DEBUG */;

	signal(SIGINT, sigint_handler);
	lwsl_user("LWS minimal ws client echo + permessage-deflate + multifragment bulk message\n");

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = port;
	info.protocols = protocols;
	info.pvo = &pvo;
	info.pt_serv_buf_size = 32 * 1024;
	info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;
	lwsl_user("Server using TLS\n");
	info.ssl_cert_filepath = "./1.pem";
	info.ssl_private_key_filepath = "./1.key";
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return 1;
	}

	while (n >= 0 && !interrupted){
		//printf("service %d\n",n);
		n = lws_service(context, 0);
	}

	lws_context_destroy(context);

	lwsl_user("Completed %s\n", interrupted == 2 ? "OK" : "failed");

	return interrupted != 2;

}