#include "event_queue.h"

extern EventQueue GlobalEvQ;

int ev_q_add(EventType type, unsigned char scancode){
	
	if (type != EVENT_KEY_PRESS && type != EVENT_KEY_RELEASE){
		return 0;
	}
	
	GlobalEvQ.count++;
	GlobalEvQ.events[GlobalEvQ.count - 1].type = type;
	GlobalEvQ.events[GlobalEvQ.count - 1].data = scancode;
	
	if (GlobalEvQ.count == 1){
		GlobalEvQ.head = 0;
	}
	GlobalEvQ.tail = GlobalEvQ.count - 1;
	
	return 1;
}

int ev_q_get(Event *event){
	
	if (GlobalEvQ.count == 0){
		return 0;
	}
	
	event->data = GlobalEvQ.events[GlobalEvQ.head].data;
	event->type = GlobalEvQ.events[GlobalEvQ.head].type;
	
	ev_q_del();
	
	return 1;
}

void ev_q_del(){
	
	if (GlobalEvQ.count == 1){
		GlobalEvQ.count = 0;
		GlobalEvQ.tail = 0;
	}
	else {
		GlobalEvQ.events[GlobalEvQ.head].data = 0x00;
		GlobalEvQ.events[GlobalEvQ.head].type = EVENT_PLACEHOLDER;
		
		for (int i = 0; i <= GlobalEvQ.tail; i++){
			GlobalEvQ.events[i].data = GlobalEvQ.events[i+1].data;
			GlobalEvQ.events[i].type = GlobalEvQ.events[i+1].type;
		}
		
		GlobalEvQ.count--;
		GlobalEvQ.tail--;
	}
}