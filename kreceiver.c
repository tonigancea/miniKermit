#include "functions.h"
#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char** argv) {
    msg r, t;
    kermit_pkt p, s;
    unsigned char seq = -1;
    int fd;

    //info ce le primim de la sender
    unsigned short MAXL = 0;                         //initial
    unsigned short TIME = 5000;                      //initial
    unsigned char NPAD;
    unsigned char PADC;
    unsigned char EOL = 0;                           //initial

    init(HOST, PORT);

    memset(r.payload, 0, sizeof(r.payload));
    memset(t.payload, 0, sizeof(t.payload));

    int retrials = 0;
    while(1){
      msg *y = receive_message_timeout(TIME);
      if (y == NULL) {
        retrials += 1;
        if (retrials == 3){
          printf("[<--]     Send-init is not coming.\n");
          return -1;
        }
        printf("[<--]     Waiting for Send-init. Trial: %d\n",retrials);
      } else {
        retrials = 0;

        // despachetare pachet din mesaj
        s = *((kermit_pkt*)y->payload);
        if (s.seq != increment(seq)){
          printf("[<--]     Received [%#x] - Expected [%#x] - ignorring\n", s.seq, increment(seq));
        } else if (s.check == crc16_ccitt(&s, 254)) {

            //despachetare date din pachet
            send_init_data data_from_sender = *((send_init_data*) s.data);
            MAXL = data_from_sender.maxl;
            TIME = data_from_sender.time * 1000;
            NPAD = data_from_sender.npad;
            PADC = data_from_sender.padc;
            EOL = data_from_sender.eol;
            printf("[<--]      Trading complete. Values: %d %d %#x %#x %#x\n", MAXL, TIME, NPAD, PADC, EOL);

            seq = increment(s.seq);
            p = createPkt(seq, MAXL, EOL);

            p.type = 'Y';                                 // sending ack
            if (s.type == 'S'){
              memcpy(p.data, &data_from_sender, 13);
            } else {
              memset(p.data, 0, sizeof(p.data));
            }
            p.check = crc16_ccitt(&p, 254);

            t.len = (int)sizeof(p) - 1;
            memcpy(t.payload, &p, t.len);
            printf("[<--][%#x] Sending ACK for [%#x]\n", seq, s.seq);
            send_message (&t);
            break;

          } else {
            seq = increment(s.seq);
            p = createPkt(seq,MAXL,EOL);

            p.type = 'N';
            p.check = crc16_ccitt(&p, 254);
            t.len = (int)sizeof(p) - 1;
            memcpy(t.payload, &p, t.len);
            printf("[<--][%#x] Sending NAK for [%#x]\n", seq, s.seq);
            send_message(&t);
          }
      }
    }

    //waiting for the files
    while (1) {
      msg *y = receive_message_timeout(TIME);
      if (y == NULL) {
        // seq = increment (s.seq);
        // t.payload[2] = seq;
        send_message(&t);
        printf ("[<--][%#x] Timeout Resending the last pkt. Resending ... \n",seq);
      } else {
        // despachetare
        s = *((kermit_pkt*)y->payload);

        if(s.seq != increment(seq)) {
          printf("[-->]      Received [%#x] - Expected [%#x] - ignorring\n", s.seq, increment(seq));
        } else {

          if (s.check == crc16_ccitt(&s, 254)) {
            if (s.type == 'F'){
              char tmp[MAXL];
              memcpy(tmp, s.data, MAXL);
              char *filename = (char*)malloc((strlen(tmp) + 6) * sizeof(char));
              memcpy(filename, "recv_", 5);
              memcpy(filename + 5, tmp, strlen(tmp));
              printf("[<--][%#x] Filename recieved: %s. Creating %s \n", s.seq, tmp, filename);

              fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

            }

            if (s.type == 'D'){
              //scriem in fisierul deschis anterior
              write(fd, s.data, s.len - 5);
            }

            if (s.type == 'Z') {
              //inchidem fisierul
              close(fd);
            }

            if (s.type == 'B') {
              //inchidem transmisia
              p.type = 'Y';
              p.len = 5;
              seq = increment (s.seq);
              p.seq = seq;
              memset(p.data, 0, sizeof(p.data));
              p.check = crc16_ccitt(&p, 254);

              t.len = (int)sizeof(p) - 1;;
              memcpy(t.payload, &p, t.len);

              break;
            }

            seq = increment (s.seq);
            p.seq = seq;
            p.type = 'Y';                                 // sending ack
            p.check = crc16_ccitt(&p, 254);

            t.len = (int)sizeof(p) - 1;;
            memcpy (t.payload, &p, t.len);
            printf ("[<--][%#x] Sending ACK for [%#x]\n", seq, s.seq);
            send_message (&t);

          } else {
            seq = increment (s.seq);
            p.seq = seq;
            p.type = 'N';
            p.check = crc16_ccitt(&p, 254);

            t.len = (int)sizeof(p) - 1;;
            memcpy (t.payload, &p, t.len);
            printf ("[<--][%#x] Sending NAK for [%#x]\n", seq, s.seq);
            send_message (&t);
          }
        }
      }
    }

    printf("[<--][%#x] Sending the final ACK! Yey!\n", seq);
    send_message(&t);
    printf("\nAnd a little bonus. While writing this code, I was reading 'Man's Search For Ultimate Meaning' and this quote inspired me:\n");
    printf("'The last of the human freedoms: to choose one's attitude in any given set of circumstances, to choose one's own way. And there were always choices to make.' -  Viktor Frankl\n");
    printf("I hope you feel inspired, too.\n\n");
	return 0;
}
