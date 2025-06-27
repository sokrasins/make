#include "msg.h"
#include <stddef.h>
#include <string.h>

#define MSG_AUTHENTICATE_STR        "authenticate"
#define MSG_AUTHORISED_STR          "authorised"
#define MSG_IP_ADDR_STR             "ip_address"
#define MSG_PONG_STR                "pong"
#define MSG_PING_STR                "ping"
#define MSG_REBOOT_STR              "reboot"
#define MSG_UPDATE_LOCKOUT_STR      "update_device_locked_out"
#define MSG_BUMP_STR                "bump"
#define MSG_SYNC_STR                "sync"
#define MSG_UNLOCK_STR              "unlock"
#define MSG_LOCK_STR                "lock"
#define MSG_ILOCK_SESS_START_STR    "interlock_session_start"
#define MSG_ILOCK_SESS_REJECTED_STR "interlock_session_rejected"
#define MSG_ILOCK_SESS_UPDATE_STR   "interlock_session_update"
#define MSG_DEBIT_STR               "debit"

msg_type_t str_to_msgtype(char *msg_type_str)
{
    if (strcmp(MSG_AUTHENTICATE_STR, msg_type_str) == 0)        { return MSG_AUTHENTICATE; }
    if (strcmp(MSG_AUTHORISED_STR, msg_type_str) == 0)          { return MSG_AUTHORISED; }
    if (strcmp(MSG_IP_ADDR_STR, msg_type_str) == 0)             { return MSG_IP_ADDR; }
    if (strcmp(MSG_PONG_STR, msg_type_str) == 0)                { return MSG_PONG; }
    if (strcmp(MSG_PING_STR, msg_type_str) == 0)                { return MSG_PING; }
    if (strcmp(MSG_REBOOT_STR, msg_type_str) == 0)              { return MSG_REBOOT; }
    if (strcmp(MSG_UPDATE_LOCKOUT_STR, msg_type_str) == 0)      { return MSG_UPDATE_LOCKOUT; }
    if (strcmp(MSG_BUMP_STR, msg_type_str) == 0)                { return MSG_BUMP; }
    if (strcmp(MSG_SYNC_STR, msg_type_str) == 0)                { return MSG_SYNC; }
    if (strcmp(MSG_UNLOCK_STR, msg_type_str) == 0)              { return MSG_UNLOCK; }
    if (strcmp(MSG_LOCK_STR, msg_type_str) == 0)                { return MSG_LOCK; }
    if (strcmp(MSG_ILOCK_SESS_START_STR, msg_type_str) == 0)    { return MSG_ILOCK_SESS_START; }
    if (strcmp(MSG_ILOCK_SESS_REJECTED_STR, msg_type_str) == 0) { return MSG_ILOCK_SESS_REJECTED; }
    if (strcmp(MSG_ILOCK_SESS_UPDATE_STR, msg_type_str) == 0)   { return MSG_ILOCK_SESS_UPDATE; }
    if (strcmp(MSG_DEBIT_STR, msg_type_str) == 0)               { return MSG_DEBIT; }
    return MSG_INVALID;
}

char *msgtype_to_str(msg_type_t msg)
{
    if (MSG_AUTHENTICATE == msg)        { return MSG_AUTHENTICATE_STR; }
    if (MSG_AUTHORISED == msg)          { return MSG_AUTHORISED_STR; }
    if (MSG_IP_ADDR == msg)             { return MSG_IP_ADDR_STR; }
    if (MSG_PONG == msg)                { return MSG_PONG_STR; }
    if (MSG_PING == msg)                { return MSG_PING_STR; }
    if (MSG_REBOOT == msg)              { return MSG_REBOOT_STR; }
    if (MSG_UPDATE_LOCKOUT == msg)      { return MSG_UPDATE_LOCKOUT_STR; }
    if (MSG_BUMP == msg)                { return MSG_BUMP_STR; }
    if (MSG_SYNC == msg)                { return MSG_SYNC_STR; }
    if (MSG_UNLOCK == msg)              { return MSG_UNLOCK_STR; }
    if (MSG_LOCK == msg)                { return MSG_LOCK_STR; }
    if (MSG_ILOCK_SESS_START == msg)    { return MSG_ILOCK_SESS_START_STR; }
    if (MSG_ILOCK_SESS_REJECTED == msg) { return MSG_ILOCK_SESS_REJECTED_STR; }
    if (MSG_ILOCK_SESS_UPDATE == msg)   { return MSG_ILOCK_SESS_UPDATE_STR; }
    if (MSG_DEBIT == msg)               { return MSG_DEBIT_STR; }
    return NULL;
}