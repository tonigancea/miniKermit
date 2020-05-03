#include "functions.h"
#define MAX 250
#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char** argv) {

    msg t;
    kermit_pkt p, s;
    unsigned char seq = 0;

    //send-init info
    unsigned short MAXL = MAX;
    unsigned short TIME = 5;
    unsigned char NPAD = 0;
    unsigned char PADC = 0;
    unsigned char EOL = 13;

    //all the magic is here
    init(HOST, PORT);

    //clear 'em
    memset(t.payload,0,sizeof(t.payload));
    memset(&p, 0, MAX + 2);

    //construiesc send_init
    p = createPkt(seq,MAXL,EOL);
    p.type = 'S';

      //populez send-init
      send_init_data send_data;
      memset(&send_data,0,13);
      send_data.maxl = MAXL;
      send_data.time = TIME;
      send_data.npad = NPAD;
      send_data.padc = PADC;
      send_data.eol = EOL;
      memcpy(p.data, &send_data, 13);

    p.check = crc16_ccitt(&p, 254);

    //copiez send-init in msg
    t.len = (int)sizeof(p) - 1;
    memcpy(t.payload, &p, t.len);

    //trimit pachetul
    send_message(&t);
    printf("[-->][%#x] Sending Send-Init \n", t.payload[2]);

    //astept confirmarea receiverului pentru send-init
    if(doWhatYouMust(&t,&seq,TIME,&s) != 0){
      printf("[->>][%#x] Error. Exiting ...\n", seq);
      return -1;
    }

      int fd;
      //for each file
      for(int file = 1; file <= argc - 1; file++){
        fd = open(argv[file], O_RDONLY);

        seq = increment(s.seq);
        p.type = 'F';
        p.seq = seq;
        memset (p.data, 0, MAXL);
        memcpy (p.data, argv[file], strlen(argv[file]));      //sending the filename
        p.check = crc16_ccitt(&p, 254);

        memset (t.payload, 0, 1400);
        t.len = (int)sizeof(p) - 1;
        memcpy (t.payload, &p, t.len);

        send_message(&t);
        printf("[-->][%#x] Sending filename %s\n", t.payload[2], argv[file]);

        //astept confirmarea receiverului pentru numele fisierului
        if(doWhatYouMust(&t,&seq,TIME,&s) != 0){
          printf("[-->][%#x] Error. Exiting ...\n",seq);
          return -1;
        }

        int count;
        char buffer[MAXL];
        while((count = read(fd, buffer, MAXL)) > 0){

          seq = increment(s.seq);
          p.seq = seq;
          p.type = 'D';
          memset (p.data, 0, MAXL);
          p.len = count + 5;                          // seq[1] + type[1] + data[count] + check[2] + mark[1]
          memcpy (p.data, buffer, count);             // chunk is send to pkt
          p.check = crc16_ccitt(&p, 254);

          t.len = (int)sizeof(p) - 1;
          memcpy (t.payload, &p, t.len);

          printf ("[-->][%#x] Chunk of file with size of %d bytes.\n", t.payload[2], count);
          send_message (&t);

          //astept confirmarea receiverului pentru chunk-ul trimis
          if(doWhatYouMust(&t,&seq,TIME,&s) != 0){
            printf("[-->][%#x] Error. Exiting ...\n",seq);
            return -1;
            }
          }

        seq = increment(s.seq);
        p.type = 'Z';
        p.seq = seq;
        memset(p.data, 0, MAXL);
        p.check = crc16_ccitt(&p, 254);

        t.len = (int)sizeof(p) - 1;
        memcpy(t.payload, &p, t.len);

        printf("[-->][%#x] Sending the Z pkt. \n", seq);
        send_message (&t);

        if(doWhatYouMust(&t,&seq,TIME,&s) != 0){
          printf("[-->][%#x] Error. Exiting ...\n",seq);
          return -1;
          }
        close(fd);
        }

        seq = increment(s.seq);
        p.type = 'B';
        p.seq = seq;
        memset(p.data, 0, MAXL);
        p.check = crc16_ccitt(&p, 254);

        t.len = (int)sizeof(p) - 1;
        memcpy(t.payload, &p, t.len);

        printf("[-->][%#x] Sending the B pkt. \n", seq);
        send_message (&t);

        if(doWhatYouMust(&t,&seq,TIME,&s) != 0){
          printf("[->>][%#x] Error. Exiting ...\n",seq);
          return -1;
          }

    return 0;
}
