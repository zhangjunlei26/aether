#ifndef AETHER_MESSAGE_REGISTRY_H
#define AETHER_MESSAGE_REGISTRY_H

#include <stdint.h>

typedef struct MessageFieldDef {
    char* name;
    int type_kind;
    struct MessageFieldDef* next;
} MessageFieldDef;

typedef struct MessageDef {
    char* name;
    int message_id;
    MessageFieldDef* fields;
    struct MessageDef* next;
} MessageDef;

typedef struct MessageRegistry {
    MessageDef* messages;
    int next_id;
} MessageRegistry;

MessageRegistry* create_message_registry(void);
void free_message_registry(MessageRegistry* registry);

int register_message_type(MessageRegistry* registry, const char* name, MessageFieldDef* fields);
MessageDef* lookup_message(MessageRegistry* registry, const char* name);
MessageDef* lookup_message_by_id(MessageRegistry* registry, int id);

typedef struct MessageInstance {
    int message_id;
    void** field_values;
    int field_count;
} MessageInstance;

MessageInstance* create_message_instance(MessageDef* def);
void free_message_instance(MessageInstance* msg);

#endif
