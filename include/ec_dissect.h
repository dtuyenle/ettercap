
/* $Id: ec_dissect.h,v 1.14 2003/10/28 22:15:02 alor Exp $ */

#ifndef EC_DISSECT_H
#define EC_DISSECT_H

#include <ec_packet.h>
#include <ec_session.h>
#include <ec_decode.h>

/* session identifier */

struct dissect_ident {
   u_int32 magic;
      #define DISSECT_MAGIC  0x0500e77e
   struct ip_addr L3_src;
   struct ip_addr L3_dst;
   u_int16 L4_src;
   u_int16 L4_dst;
   u_int8 L4_proto; /* Odd byte has to be the last for correct session hash matching */
};

#define DISSECT_IDENT_LEN sizeof(struct dissect_ident)

/* exported functions */

extern void dissect_add(char *name, u_int8 level, u_int32 port, FUNC_DECODER_PTR(decoder));
extern int dissect_modify(int mode, char *name, u_int32 port);
#define MODE_ADD  0
#define MODE_REP  1

extern int dissect_match(void *id_sess, void *id_curr);
extern void dissect_create_session(struct session **s, struct packet_object *po); 
extern void dissect_wipe_session(struct packet_object *po);
extern size_t dissect_create_ident(void **i, struct packet_object *po); 

extern int dissect_on_port(char *name, u_int16 port);
   
/*
 * creates the session on the first packet sent from
 * the server (SYN+ACK)
 */

#define CREATE_SESSION_ON_SYN_ACK(name, session) do{        \
      if ((PACKET->L4.flags & TH_SYN) && (PACKET->L4.flags & TH_ACK) && dissect_on_port(name, ntohs(PACKET->L4.src)) == ESUCCESS) { \
         DEBUG_MSG("%s --> create_session_on_syn_ack", name);   \
         /* create the session */                           \
         dissect_create_session(&session, PACKET);          \
         session_put(session);                              \
         return NULL;                                       \
      }                                                     \
   }while(0)

/*
 * helper macros to get the banner of a service if it is the first thing 
 * the server send to the client.
 * it must be used this way:
 *
 * IF_FIRST_PACKET_FROM_SERVER(21, s, i) {
 * 
 *    ... do something with PACKET->DISSECTOR.banner
 *    
 * } ENDIF_FIRST_PACKET_FROM_SERVER(21, s, i)
 *
 */

#define IF_FIRST_PACKET_FROM_SERVER(name, session, ident)                  \
   if (dissect_on_port(name, ntohs(PACKET->L4.src)) == ESUCCESS && PACKET->L4.flags & TH_PSH) {  \
      dissect_create_ident(&ident, PACKET);                                \
      /* the session exist */                                              \
      if (session_get(&session, ident, sizeof(struct dissect_ident)) != -ENOTFOUND) { \
         /* prevent the deletion of session created for the user and pass */ \
         if (session->data == NULL)                                        


#define ENDIF_FIRST_PACKET_FROM_SERVER(session, ident)         \
         if (session->data == NULL)                            \
            session_del(ident, sizeof(struct dissect_ident));  \
      }                                                        \
      SAFE_FREE(ident);                                        \
      return NULL;                                             \
   }  


/* return true if the packet is coming from the server */
#define FROM_SERVER(name, pack) (dissect_on_port(name, ntohs(pack->L4.src)) == ESUCCESS)

/* return true if the packet is coming from the client */
#define FROM_CLIENT(name, pack) (dissect_on_port(name, ntohs(pack->L4.dst)) == ESUCCESS)


#define DISSECT_MSG(x, ...) do {    \
   if (!GBL_OPTIONS->superquiet)    \
      USER_MSG(x, ## __VA_ARGS__ ); \
} while(0)

#endif

/* EOF */

// vim:ts=3:expandtab

