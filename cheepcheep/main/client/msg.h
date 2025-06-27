#ifndef MSGS_H_
#define MSGS_H_

typedef enum {
    MSG_AUTHENTICATE,
    MSG_AUTHORISED,
    MSG_IP_ADDR,
    MSG_PING,
    MSG_PONG,
    MSG_REBOOT,
    MSG_UPDATE_LOCKOUT,
    MSG_BUMP,
    MSG_SYNC,
    MSG_UNLOCK,
    MSG_LOCK,
    MSG_ILOCK_SESS_START,
    MSG_ILOCK_SESS_UPDATE,
    MSG_ILOCK_SESS_REJECTED,
    MSG_DEBIT,
    MSG_INVALID,
} msg_type_t;

msg_type_t str_to_msgtype(char *msg_type_str);
char *msgtype_to_str(msg_type_t msg);

#endif /*MSGS_H_*/