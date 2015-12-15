#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>

#define DEFAULT_IF "eth0"
#define BUF_SIZ 64

int main(int argc, char *argv[]) {
  int sockfd;
  struct ifreq if_idx;
  struct ifreq if_mac;
  char sendbuf[BUF_SIZ];
  struct sockaddr_ll socket_address;
  char ifName[IFNAMSIZ];
  unsigned int mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //using as uint8_t

  /* Get interface name */
  if (argc > 1){
    strcpy(ifName, argv[1]);
    if (argc > 2) { /* 2 arguments, second argument is mac */
      sscanf(argv[2], "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    }
  }
  else
    strcpy(ifName, DEFAULT_IF);

  /* Print info */
  printf("Broadcasting from STDIN to MAC address ");
  printf("%2x:%2x:%2x:%2x:%2x:%2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  /* Open RAW socket to send on */
  if ((sockfd = socket(AF_PACKET, SOCK_DGRAM, htons(0x88b5))) == -1) {
    perror("Error opening socket");
    return -1;
  }

  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
    perror("Error getting interface index");
    return -1;
  }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) {
    perror("Error getting interface MAC address");
    return -1;
  }

  socket_address.sll_family=AF_PACKET;
  /* Index of the network device */
  socket_address.sll_ifindex = if_idx.ifr_ifindex;
  /* Address length*/
  socket_address.sll_halen = ETHER_ADDR_LEN;

  socket_address.sll_protocol = htons(0x88b5);
  /* Destination MAC */
  socket_address.sll_addr[0] = mac[0];
  socket_address.sll_addr[1] = mac[1];
  socket_address.sll_addr[2] = mac[2];
  socket_address.sll_addr[3] = mac[3];
  socket_address.sll_addr[4] = mac[4];
  socket_address.sll_addr[5] = mac[5];


  while (1) {
   
    /* Read packet data */
    if(fread((void *)sendbuf, 1, BUF_SIZ, stdin) != BUF_SIZ) {
      if (feof(stdin)) {
	printf("End of input reached\n");
	return 0;
      } else {
	perror("Error reading from stdin");
	return -1;
      }
    }
      
    /* Send packet */
    if (sendto(sockfd, sendbuf, BUF_SIZ, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
      perror("Error sending packet");
      return -1;
    }

  } //end while
  
  return 0;
}
