#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#define MAX_EVENTS 64

typedef enum {
	EVENT_PLACEHOLDER,
	EVENT_KEY_PRESS,
	EVENT_KEY_RELEASE
} EventType;

typedef struct {
	EventType type;
	unsigned int data;
} Event;

typedef struct {
	Event events[MAX_EVENTS];
	unsigned int head;
	unsigned int tail;
	unsigned int count;
} EventQueue;

int ev_q_add(EventType type, unsigned char scancode);
int ev_q_get(Event *event);
void ev_q_del();

#endif