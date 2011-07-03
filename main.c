#include <stdio.h>

#include <sys/types.h>	/* socket, getaddrinfo */
#include <sys/socket.h>	/* socket, getaddrinfo */

#include <netdb.h>	/* getaddrinfo */
#include <netinet/in.h>	/* udp */

#include <stdarg.h>	/* va_* */
#include <unistd.h>	/* getopt */

static const char optstr[] = ":p:f:l:";

static int warn(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int x = vfprintf(stderr, fmt, ap);
	va_end(ap);
	return x;
}

static void usage(char *pname)
{
	warn(
		"usage: %s	[options]\n"
		"	-h		show help\n"
		"	-f <cfgfile>	cfg file path\n"
		"	-l <local>	where to listen\n"
		"	-p <peer>	an initial peer to connect to.\n"
		"\n"
		"			ipv4 - ip:port,   ip, host, host:port\n"
		"			ipv6 - [ip]:port, ip, host, host:port\n"
	    , pname);

}


struct peer_addr {
	char *host;
	uint16_t port;
};


int peer_str_to_addr(char *in_str, struct peer_addr *pa)
{
	/* TODO: parse in_str, determine */
	return -1;
}

struct peer_args {
	
};

static void *peer_thread(void *pa_v)
{
	struct peer_args *pa = pa_v;



	return NULL;
}

int main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, optstr)) != -1) {
		switch(c) {
		case 'p':
			warn("need to add peer-str to list: %s\n", optarg);
			break;
		case 'l':
			warn("add this to listen list: %s\n", optarg);
			break;
		case 'f':
			warn("config file path: %s\n", optarg);
			break;
		case ':':
			warn("option '-%c' requires an argument\n", optopt);
			break;
		case '?':
			warn("option '-%c' is unrecognized\n", optopt);
			break;
		default:
			warn("opt: %c\n", c);
			return -1;
		}
	}

	return 0;
}
