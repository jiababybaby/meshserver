#include "libwebsockets.h"
#include "string.h"
#include "signal.h"
#include "../include/data_solve.h"
#define LWS_PLUGIN_STATIC


/* one of these created for each message */

struct msg {
	void *payload; /* is malloc'd */
	size_t len;
	char binary;
	char first;
	char final;
};

struct per_session_data__meshserver {
	struct lws_ring *ring;
	uint32_t msglen;
	uint32_t tail;
	uint8_t completed:1;
	uint8_t flow_controlled:1;
	uint8_t write_consume_pending:1;
};

struct vhd_meshserver {
	struct lws_context *context;
	struct lws_vhost *vhost;

	int *interrupted;
	int *options;
};
#include <assert.h>
static int callback_meshserver(struct lws *wsi, enum lws_callback_reasons reason,
										void *user, void *in, size_t len)
{
	struct per_session_data__meshserver *pss =
			(struct per_session_data__meshserver *)user;
	struct vhd_meshserver *vhd = (struct vhd_meshserver *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi),
				lws_get_protocol(wsi));
	const struct msg *pmsg;
	struct msg amsg;
	int m, n, flags;
	char in_data[5000] = {0}, out_data[5000] = {0};

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		printf("lws init\n");
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
										  lws_get_protocol(wsi),
										  sizeof(struct vhd_meshserver));
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
				/* generate a block of output before travis times us out */
				lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
			lwsl_user("LWS_CALLBACK_SERVER_WRITEABLE\n");
		break;
		
	case LWS_CALLBACK_RECEIVE:
			lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, "
					"last %d, bin %d, msglen %d (+ %d = %d))\n",
					(int)len, (int)lws_remaining_packet_payload(wsi),
					lws_is_first_fragment(wsi),
					lws_is_final_fragment(wsi),
					lws_frame_is_binary(wsi), pss->msglen, (int)len,
					(int)pss->msglen + (int)len);

			if (len>0 && len < 1024) {
				memset(in_data, 0, sizeof(in_data));
				memset(out_data, 0, sizeof(out_data));
				memcpy(in_data, in, len);
				ws_data_Parse_Json(in_data, out_data);
				// printf("%s\n%s\n", in_data, out_data);

				if(!strncmp(in_data, "ping", 4)){
					memset(out_data,0,strlen(out_data));
					strcpy(out_data, in_data);
				}
				amsg.first = (char)lws_is_first_fragment(wsi);
				amsg.final = (char)lws_is_final_fragment(wsi);
				amsg.binary = (char)lws_frame_is_binary(wsi);
				int send_len = strlen(out_data);
				if (amsg.final)
					pss->msglen = 0;
				else
					pss->msglen += (uint32_t)send_len;

				amsg.len = send_len;
				// amsg.payload = malloc(send_len);
				// if(amsg.payload == NULL){
				// 	printf("malloc error.\n");
				// 	break;
				// }
				// flags = lws_write_ws_flags(
				// 	amsg.binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
				// 	amsg.first, amsg.final);
				flags = lws_write_ws_flags(LWS_WRITE_TEXT, 1, 1);
				// memcpy((char *)amsg.payload, out_data, send_len);
				/* notice we allowed for LWS_PRE in the payload already */
				m = lws_write(wsi, ((unsigned char *)out_data), amsg.len, (enum lws_write_protocol)flags);
				// printf("send %d %d\n",m, amsg.len);
				if (m < (int)amsg.len)
				{
					lwsl_err("ERROR %d writing to ws socket\n", m);
				}
				// free(amsg.payload);
				// amsg.payload = NULL;
				// amsg.len = 0;
			}
		break;
	case LWS_CALLBACK_CLOSED:
			lwsl_user("LWS_CALLBACK_CLOSED\n");
			// if (*vhd->options & 1) {
			// 	if (!*vhd->interrupted)
			// 		*vhd->interrupted = 1 + pss->completed;
			// 	lws_cancel_service(lws_get_context(wsi));
			// }
		break;

	default:
		break;
	}

	return 0;
}

#define LWS_MESHSERVER \
	{ \
		"meshserver", \
		callback_meshserver, \
		sizeof(struct per_session_data__meshserver), \
		1024, \
		0, NULL, 0 \
	}


static struct lws_protocols protocols[] = {
	LWS_MESHSERVER,
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
	"lws-meshserver",	/* protocol name we belong to on this vhost */
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
	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = port;
	info.protocols = protocols;
	info.pvo = &pvo;
	info.pt_serv_buf_size = 128 * 1024;
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

	while (1){
		//printf("service %d\n",n);
		n = lws_service(context, 0);
	}

	lws_context_destroy(context);

	lwsl_user("Completed %s\n", interrupted == 2 ? "OK" : "failed");

	return interrupted != 2;

}