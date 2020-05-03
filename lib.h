#ifndef LIB
#define LIB

typedef struct {
    int len;
    char payload[1400];
} msg;

typedef struct {
  unsigned char soh;
  unsigned char len;
  unsigned char seq;
  unsigned char type;
  unsigned char data[250];
  unsigned short check;
  unsigned char mark;
} kermit_pkt;

typedef struct {
  unsigned short maxl;
  unsigned short time;
  unsigned char npad;
  unsigned char padc;
  unsigned char eol;
  unsigned char qctl;
  unsigned char qbin;
  unsigned char chkt;
  unsigned char rept;
  unsigned char capa;
  unsigned char r;
} send_init_data;

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);


#endif
