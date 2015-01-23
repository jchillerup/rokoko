#include <assert.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/tcp.h>

/* ds = downstream *
 * us = upstream   */

const int ds_port_number = 14041;
const int us_port_number = 14040;

const char* ds_url = "tcp://*:14040";
const char* us_url = "ipc:///tmp/rokoko-socket.ipc";

int pubsub_sock;
int upstream_sock;

int init_pubsub() {
  int sock = nn_socket(AF_SP, NN_PUB);
  assert(sock >= 0);
  assert(nn_bind(sock, ds_url) >= 0);

  return sock;
}

int init_upstream() {
  // Set up the socket that the upstream connects to
  int sock = nn_socket(AF_SP, NN_PULL);
  assert(sock >= 0);
  assert(nn_bind(sock, us_url));

  return sock;
}

int main() {
  printf("ROKOKO message broker\n");

  pubsub_sock = init_pubsub();
  upstream_sock = init_upstream();
  
  /* the main loop that listens for messages from upstream */
  while (1) {
    char *buf = NULL;

    // Receive some bytes
    int recv_bytes = nn_recv (upstream_sock, &buf, NN_MSG, 0);
    assert (recv_bytes >= 0);
    
    // Pass them on downstream
    int sent_bytes = nn_send (pubsub_sock, buf, recv_bytes, 0);
    assert(recv_bytes == sent_bytes);

    /* printf("Sent: %s, %d bytes\n", buf, recv_bytes); */
    
    nn_freemsg (buf);
  }
  
  return nn_shutdown(pubsub_sock, 0);
}
