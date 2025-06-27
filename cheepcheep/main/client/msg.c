#include "msg.h"
#include "log.h"

#include <stdio.h>
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
#define MSG_ILOCK_SESS_END_STR      "interlock_session_end"
#define MSG_ILOCK_SESS_UPDATE_STR   "interlock_session_update"
#define MSG_ILOCK_OFF_STR           "interlock_off"
#define MSG_DEBIT_STR               "debit"
#define MSG_ACCESS_DENIED_STR       "log_access_denied"
#define MSG_ACCESS_LOCKED_OUT_STR   "log_access_locked_out"
#define MSG_ACCESS_GRANTED_STR      "log_access"

// Helpers
msg_type_t str_to_msgtype(char *msg_type_str);
char *msgtype_to_str(msg_type_t msg);

status_t msg_to_cJSON(msg_t *msg, cJSON *json)
{
    status_t status = -STATUS_INVALID;

    cJSON_AddStringToObject(json, "command", msgtype_to_str(msg->type));
    switch (msg->type)
    {
        case MSG_AUTHENTICATE:
            cJSON_AddStringToObject(json, "secret_key", msg->authenticate.secret_key);
            status = STATUS_OK;
            break;

        case MSG_IP_ADDR: {
            char ip_str[32];
            sprintf(ip_str, "%u.%u.%u.%u", 
                (uint8_t) ((msg->ip_address.ip_address >> 0) & 0xFF), 
                (uint8_t) ((msg->ip_address.ip_address >> 8) & 0xFF), 
                (uint8_t) ((msg->ip_address.ip_address >> 16) & 0xFF), 
                (uint8_t) ((msg->ip_address.ip_address >> 24) & 0xFF)
            );
            cJSON_AddStringToObject(json, "ip_address", ip_str);
            status = STATUS_OK;
            break;
        }

        case MSG_ILOCK_SESS_START:
        case MSG_ILOCK_SESS_UPDATE:
        case MSG_ILOCK_SESS_END:
        case MSG_ILOCK_OFF:
        case MSG_ILOCK_SESS_REJECTED:
        case MSG_DEBIT:
            status = -STATUS_UNIMPL;
            break;

        case MSG_ACCESS_DENIED:
            cJSON_AddNumberToObject(json, "card_id", msg->access_denied.card_id);
            status = STATUS_OK;
            break;

        case MSG_ACCESS_LOCKED_OUT:
            cJSON_AddNumberToObject(json, "card_id", msg->access_lockout.card_id);
            status = STATUS_OK;
            break;

        case MSG_ACCESS_GRANTED:
            cJSON_AddNumberToObject(json, "card_id", msg->access_granted.card_id);
            status = STATUS_OK;
            break;

        // These have no payloads
        case MSG_PING:
            status = STATUS_OK;
            break;
        
        // These messages aren't sent by the client. The should be parsed 
        // for completeness of implementation, but are the lowest priority
        case MSG_AUTHORISED:
        case MSG_UPDATE_LOCKOUT:
        case MSG_SYNC:
        case MSG_REBOOT:
        case MSG_BUMP:
        case MSG_UNLOCK:
        case MSG_LOCK:
        case MSG_PONG:
            status = -STATUS_UNIMPL;
            break;
        
        // The rest of these are wrong
        case MSG_INVALID:
        default:
            ERROR("Unexpected message with type %u", msg->type);
    }
    
    return status;
}

status_t msg_from_cJSON(cJSON *json, msg_t *msg)
{
    // the "authorised" message has a different format, try to parse it first
    if (cJSON_HasObjectItem(json, "authorised"))
    { 
        cJSON *value = cJSON_GetObjectItem(json, "authorised");
        if (value == NULL) { return -STATUS_INVALID; }
        msg->type = MSG_AUTHORISED; 
        msg->authorised.authorised = (bool) value->valueint;
        return STATUS_OK;
    }

    // Everything else following the same general format.
    // Try to get the message tyoe first.
    cJSON *msg_type_json = cJSON_GetObjectItem(json, "command");
    if (!msg_type_json) { return -STATUS_INVALID; }
    msg->type = str_to_msgtype(msg_type_json->valuestring);
    if (msg->type == MSG_INVALID) { return -STATUS_INVALID; }

    status_t status = -STATUS_INVALID;
    switch (msg->type)
    {
        case MSG_UPDATE_LOCKOUT: {
            cJSON *payload_val = cJSON_GetObjectItem(json, "locked_out");
            if (payload_val) 
            { 
                msg->update_lockout.locked_out = (bool) payload_val->valueint;
                status = STATUS_OK;
            }
            break;
        }

        case MSG_SYNC: {
            // TODO: how to not copy kB's of data
            cJSON *payload_val = cJSON_GetObjectItem(json, "hash");
            if (payload_val)
            {
                // TODO: convert string to hex bytes
            }
            payload_val = cJSON_GetObjectItem(json, "tags");
            msg->sync.tags = payload_val; // To be parsed by the handler
            status = STATUS_OK;
            break;
        }

        case MSG_ILOCK_SESS_START:
        case MSG_ILOCK_SESS_UPDATE:
        case MSG_ILOCK_SESS_END:
        case MSG_ILOCK_OFF:
        case MSG_ILOCK_SESS_REJECTED:
        case MSG_DEBIT:
            status = -STATUS_UNIMPL;
            break;

        // These have no payloads
        case MSG_PING:
        case MSG_PONG:
        case MSG_REBOOT:
        case MSG_BUMP:
        case MSG_UNLOCK:
        case MSG_LOCK:
            status = STATUS_OK;
            break;

        // These messages aren't sent by the server. The should be parsed 
        // for completeness of implementation, but are the lowest priority
        case MSG_AUTHENTICATE:
        case MSG_IP_ADDR:
        case MSG_ACCESS_DENIED:
        case MSG_ACCESS_LOCKED_OUT:
        case MSG_ACCESS_GRANTED:
            break;
        
        // The rest of these are wrong
        case MSG_AUTHORISED: // Handled above, shouldn't reach here
        case MSG_INVALID:
        default:
            ERROR("Unexpected message with type %u", msg->type);
    }

    return status;
}

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
    if (strcmp(MSG_ILOCK_SESS_END_STR, msg_type_str) == 0)      { return MSG_ILOCK_SESS_END; }
    if (strcmp(MSG_ILOCK_SESS_UPDATE_STR, msg_type_str) == 0)   { return MSG_ILOCK_SESS_UPDATE; }
    if (strcmp(MSG_ILOCK_OFF_STR, msg_type_str) == 0)           { return MSG_ILOCK_OFF; }
    if (strcmp(MSG_DEBIT_STR, msg_type_str) == 0)               { return MSG_DEBIT; }
    if (strcmp(MSG_ACCESS_DENIED_STR, msg_type_str) == 0)       { return MSG_ACCESS_DENIED; }
    if (strcmp(MSG_ACCESS_LOCKED_OUT_STR, msg_type_str) == 0)   { return MSG_ACCESS_LOCKED_OUT; }
    if (strcmp(MSG_ACCESS_GRANTED_STR, msg_type_str) == 0)      { return MSG_ACCESS_GRANTED; }
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
    if (MSG_ILOCK_SESS_END == msg)      { return MSG_ILOCK_SESS_END_STR; }
    if (MSG_ILOCK_SESS_UPDATE == msg)   { return MSG_ILOCK_SESS_UPDATE_STR; }
    if (MSG_ILOCK_OFF == msg)           { return MSG_ILOCK_OFF_STR; }
    if (MSG_DEBIT == msg)               { return MSG_DEBIT_STR; }
    if (MSG_ACCESS_DENIED == msg)       { return MSG_ACCESS_DENIED_STR; }
    if (MSG_ACCESS_LOCKED_OUT == msg)   { return MSG_ACCESS_LOCKED_OUT_STR; }
    if (MSG_ACCESS_GRANTED == msg)      { return MSG_ACCESS_GRANTED_STR; }
    return NULL;
}