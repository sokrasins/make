#ifndef MSGS_H_
#define MSGS_H_

#include "status.h"
#include "cJSON.h"

#include <stdint.h>
#include <stdbool.h>

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
    MSG_ILOCK_SESS_END,
    MSG_ILOCK_OFF,
    MSG_ILOCK_SESS_REJECTED,
    MSG_DEBIT,
    MSG_ACCESS_DENIED,
    MSG_ACCESS_LOCKED_OUT,
    MSG_ACCESS_GRANTED,
    MSG_INVALID,
} msg_type_t;

typedef struct {
    char *secret_key;
} authenticate_payload_t;

typedef struct {
    bool authorised;
} authorised_payload_t;

typedef struct {
    uint32_t ip_address;
} ip_addr_payload_t;

typedef struct {
    bool locked_out;
} update_lockout_payload_t;

typedef struct {
    uint8_t hash[16];
    cJSON *tags; // Special case, pass the JSON array for further parsing to avoid having to copy
} sync_payload_t;

typedef struct {
    uint32_t card_id;
} ilock_sess_start_reqpayload_t;

typedef struct {
    char session_id[32];
} ilock_sess_start_rsppayload_t;

typedef struct {
    char session_id[32];
    float session_kwh; // check
} ilock_sess_update_payload_t;

typedef struct {
    char session_id[32];
    float session_kwh; // check
    uint32_t card_id;
} ilock_sess_end_payload_t;

typedef struct {
    uint32_t card_id;
} access_denied_payload_t;

typedef struct {
    uint32_t card_id;
} access_locked_out_payload_t;

typedef struct {
    uint32_t card_id;
} access_granted_payload_t;

typedef struct {
    uint32_t card_id;
    float amount;
} debit_reqpayload_t;

typedef struct {
    bool success;
    float balance;
} debit_rsppayload_t;

typedef struct {
    msg_type_t type;
    union {
        authenticate_payload_t authenticate;
        authorised_payload_t authorised;
        ip_addr_payload_t ip_address;
        update_lockout_payload_t update_lockout;
        sync_payload_t sync;
        ilock_sess_start_reqpayload_t ilock_start_req;
        ilock_sess_start_rsppayload_t ilock_start_rsp;
        ilock_sess_update_payload_t ilock_update;
        ilock_sess_end_payload_t ilock_end;
        access_denied_payload_t access_denied;
        access_locked_out_payload_t access_lockout;
        access_granted_payload_t access_granted;
        debit_reqpayload_t debit_req;
        debit_rsppayload_t debit_rsp;
    };
} msg_t;

status_t msg_to_cJSON(msg_t *msg, cJSON *json);
status_t msg_from_cJSON(cJSON *json, msg_t *msg);

#endif /*MSGS_H_*/