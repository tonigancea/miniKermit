#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"
#include <string.h>

unsigned char increment(unsigned char old){
  //Ah ah ah! You'll never understand why this one works.
  return (old < 63) ? old + 1 : 0;
}

kermit_pkt createPkt(unsigned char seq, unsigned short MAXL, unsigned char EOL){
  kermit_pkt p;
  p.soh = 1;
  p.len = 255;
  p.seq = seq;
  p.type = 'E';
  memset(p.data,0,MAXL);
  p.check = 0;
  p.mark = EOL;
  return p;
}

//Abandon all hope ye who enter beyond this point

int doWhatYouMust(msg *t, unsigned char *seq, unsigned short TIME, kermit_pkt *s){
  int retrials = 0;
  while (1) {
    msg *y = receive_message_timeout(TIME*1000);
    if (y == NULL) {
      retrials += 1;
      if (retrials == 4){
        printf("[-->]      Failed. Too many retrials.\n");
        return -1;
      }
      printf("[-->]      Timeout, this is retrial %d to recieve. [%#x]\n", retrials, increment((*seq)));
      // retrimit pachetul cu acelasi seq
      send_message(t);
    } else {
        retrials = 0;
        *s = *((kermit_pkt*)y->payload);
        if (s->seq != increment(*seq)) {
          printf("[-->]      Received [%#x] - Expected [%#x] - ignorring\n", s->seq, increment(*seq));
        } else {

          if (s->type == 'N') {               // pachetul este nak
            (*seq) = increment(s->seq);       // seq-ul meu este seq-ul lui nak, incrementat
            kermit_pkt p;
            p = *((kermit_pkt*)t->payload);
            p.seq = *seq;
            p.check = crc16_ccitt(&p, 254);

            t->len = (int)sizeof(p) - 1;
            memcpy(t->payload,&p,t->len);     // actualizez seq-ul in pachetul de trimis         
            printf("[-->][%#x] Reseding last msg\n", *seq);
            send_message(t);
          }
          if (s->type == 'Y') {               // pachetul este ack
            return 0;
          }
        }
      }
    }
}
