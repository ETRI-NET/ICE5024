/* For sockaddr_in */
#include <netinet/in.h>
/* For socket functions */
#include <sys/socket.h>/* For fcntl */
#include <fcntl.h> 
#include <event2/event.h>
#include <arpa/inet.h> 
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h> 
#define MAX_LINE 16384 

void do_read(evutil_socket_t fd, short events, void *arg); 

struct fd_state {
    char buffer[MAX_LINE];
    struct in_addr mcast_ip;
    u_int16_t mcast_port;
    struct event *read_event;
}; 


struct fd_state * alloc_fd_state(struct event_base *base, evutil_socket_t fd)
{
    struct fd_state *state = malloc(sizeof(struct fd_state));
    if (!state)
        return NULL;
    state->read_event = event_new(base, fd, EV_READ|EV_PERSIST, do_read, state);
    if (!state->read_event) {
        free(state);
        return NULL;
    }
    return state;
} 

void free_fd_state(struct fd_state *state)
{
    event_free(state->read_event);
    free(state);
} 

int mcast_channel_fd_new(struct event_base *base, struct in_addr mcast, struct in_addr local, u_int16_t mcast_port)
{
    evutil_socket_t nsock;
    struct sockaddr_in sin;
    int loop = 1;
    struct ip_mreq mreq;
    struct fd_state *state;
    mreq.imr_multiaddr = mcast;
    mreq.imr_interface = local;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = mcast_port;
    nsock = socket(AF_INET, SOCK_DGRAM, 0);
    evutil_make_socket_nonblocking(nsock);
 #ifndef WIN32
    {
        int one = 1;
        setsockopt(nsock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(nsock, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return -1;
    }

    if(setsockopt(nsock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
    {
        perror("setsocket():IP MULTICAST_LOOP");
        return -1;
    }

    if(setsockopt(nsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0){
        printf("%s setsockopt():IP ADD MEMBURSHIP\n",strerror(errno));
        return -1;
    }

    state = alloc_fd_state(base, nsock);
    assert(state); /*XXX err*/
    state->mcast_ip = mreq.imr_multiaddr;
    state->mcast_port = mcast_port;

    event_add(state->read_event, NULL);

    return 0;
}

void do_read(evutil_socket_t fd, short events, void *arg)
{
    struct fd_state *state = arg;
    char buf[1024];
    int i;
    struct sockaddr_in src_addr;
    socklen_t len;
    ssize_t result;
    result = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&src_addr, &len);
    fprintf(stderr, "recv %zd from %s:%d", result, inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
    fprintf(stderr, " with mcast_channel %s:%d\n", inet_ntoa(state->mcast_ip), ntohs(state->mcast_port));

    if (result == 0) {
        free_fd_state(state);
    } else if (result < 0) {
        if (errno == EAGAIN) // XXXX use evutil macro
        {
            return;
        }
        perror("recv");
        free_fd_state(state);
    }
}

int main(int c, char **v)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    struct in_addr mcast;
    struct in_addr local;
    struct event_base *base;

    local.s_addr = inet_addr("127.0.0.1");
    base = event_base_new();
    if (!base)
        return; /*XXXerr*/

    mcast.s_addr = inet_addr("224.0.0.251");

    if (mcast_channel_fd_new(base, mcast, local, htons(5350)) < 0)
        return;

    mcast.s_addr = inet_addr("224.0.0.252");
    if (mcast_channel_fd_new(base, mcast, local, htons(5252)) < 0)
        return;
    event_base_dispatch(base);

    return 0;
} 

