#include "mdcode.h"

extern int input_mode;
extern int input_exec_ready;

extern char input_buf[BUFFER_SIZE];
extern char input_buf_exec[BUFFER_SIZE];
extern unsigned int buf_id;

extern int debug_mode;

extern void check_comm(const char* comm);

void interpret_program(unsigned char *program){
	
	setmemory(input_buf, 0, sizeof(input_buf));
	setmemory(input_buf_exec, 0, sizeof(input_buf_exec));
	
	Executor state;
	state.program = program;
	state.program_c = 0;
	setmemory(state.work_stack, 0, sizeof(state.work_stack));
	state.stack_pointer = 0;
	state.flags = 0;
	
	while(1) {
		unsigned char opcode_o_arr[2];
		unsigned char opcode_t_arr[2];
		
		unsigned int int_opcode_h;
		unsigned int int_opcode = 0;
		
		opcode_o_arr[0] = state.program[state.program_c++];
		opcode_o_arr[1] = '\0';
		opcode_t_arr[0] = state.program[state.program_c];
		opcode_t_arr[1] = '\0';
		
		if (isnumber(opcode_o_arr[0])){
			if (isnumber(opcode_t_arr[0]) && opcode_o_arr[0] != '0'){
				strtodig(opcode_t_arr, &int_opcode);
				strtodig(opcode_o_arr, &int_opcode_h);
				
				int_opcode_h = int_opcode_h * 10;
				
				state.program_c++;
			}
			else {
				strtodig(opcode_o_arr, &int_opcode_h);
			}
		}
		else {
			continue;
		}
		if (debug_mode){
			
			char address[10];
			digtostr((state.program_c - 1), address);
			
			push_text("[DEBUG STACK] - ['");
			push_text(state.work_stack);
			push_text("'; command address (for goto) - '");
			push_text(address);
			push_text("', OP: '");
			push_text(opcode_o_arr);
			push_text(opcode_t_arr);
			push_text("']\n");
		}
		switch ((int_opcode_h + int_opcode)){
			case OP_PUSH_CHAR: {
				char value = state.program[state.program_c++];
				state.work_stack[state.stack_pointer++] = value;
				break;
			}
			case OP_PUSH_TEXT: {
				const char *text = (const char *)(state.program + state.program_c);
				while (state.program[state.program_c] != '\0' && state.program[state.program_c] != '\n'){
					state.work_stack[state.stack_pointer++] = state.program[state.program_c++];
				}
				
				state.work_stack[state.stack_pointer++] = '\n';
				
				state.program_c++;
				break;
			}
			case OP_INPUT: {
				
				setmemory(input_buf, 0, sizeof(input_buf));
				setmemory(input_buf_exec, 0, sizeof(input_buf_exec));
				
				input_mode = 1;
				input_exec_ready = 0;
				buf_id = 0;
				input_buf[0] = '\0';
				
				idt_ini();
				
				push_text("> ");
				
				pic_remap();
				
				while (!input_exec_ready){
					asm volatile("hlt");
				}
				input_exec_ready = 0;
				input_mode = 0;
				
				for (char *p = input_buf_exec; *p != '\0'; p++){
					state.work_stack[state.stack_pointer++] = *p;
				}
				state.work_stack[state.stack_pointer++] = '\0';
				push_char('\n');
				
				break;
			}
			case OP_EXIT: {
				input_mode = 0;
				return;
			}
			case OP_SYS_EXEC: {
				const char *text = (const char *)(state.program + state.program_c);
				check_comm(text);
			}
			case OP_PRINT: {
				int i = 0;
				while (i < state.stack_pointer){
					push_char(state.work_stack[i]);
					i++;
				}
				state.stack_pointer = 0;
				break;
			}
			case OP_CLEAR_BUF: {
				
				for (int i = 0; i < sizeof(state.work_stack); i++){
					state.work_stack[i] = 0;
				}
				state.stack_pointer = 0;
				break;
			}
			case OP_CALC: {
				if (state.stack_pointer < 3) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				
				char num1_str[32] = {0};
				char num2_str[32] = {0};
				char op = 0;

				int num1_len = 0;
				int num2_len = 0;
    
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num2_str[num2_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num2_len / 2; i++) {
					char temp = num2_str[i];
					num2_str[i] = num2_str[num2_len - 1 - i];
					num2_str[num2_len - 1 - i] = temp;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				if (sp >= 0) {
					op = state.work_stack[sp];
					sp--;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}

				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}

				if (op == 0) {
					push_text("\nRun time error: No operator found!\n");
					input_mode = 0;
					return;
				}
				
				int num1 = 0, num2 = 0;
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				if (state.work_stack[sp] == '-'){
					num1 = -num1;
					sp--;
				}
				
				for (int i = 0; i < num2_len; i++) {
					num2 = num2 * 10 + (num2_str[i] - '0');
				}
				
				int answer;
				
				switch (op) {
					case '+': 
						answer = num1 + num2;
						break;
					case '-': 
						answer = num1 - num2;
						break;
					case '*': 
						answer = num1 * num2;
						break;
					case '/': 
						if (num2 == 0) {
							push_text("\nRun time error! Division by zero!\nExit the program...\n");
							input_mode = 0;
							return;
						}
						answer = num1 / num2;
						break;
					default:
						push_text("\nRun time error! Unknown operator (");
						push_char(op);
						push_text(")\nExit the program...\n");
						input_mode = 0;
						return;
				}
				
				state.stack_pointer = sp + 1;
				
				push_format("num1=%i : %i at %u", num1, answer, state.stack_pointer);
				
				if (answer < 0){
					state.work_stack[state.stack_pointer++] = '-';
					answer = -answer;
				}
				
				char strans[32];
				int temp = answer;
				int digits = 0;
				
				do {
					digits++;
					temp /= 10;
				} while (temp > 0);
				
				temp = answer;
				for (int i = digits - 1; i >= 0; i--) {
					strans[i] = '0' + (temp % 10);
					temp /= 10;
				}
				strans[digits] = '\0';
				
				for (int i = 0; i < digits; i++) {
					state.work_stack[state.stack_pointer++] = strans[i];
				}
				
				char address[10];
				digtostr((state.program_c - 1), address);
				
				push_text("[DEBUG STACK] - ['");
				push_text(state.work_stack);
				push_text("'; command address (for goto) - '");
				push_text(address);
				push_text("', OP: '");
				push_text(opcode_o_arr);
				push_text(opcode_t_arr);
				push_text("']\n");
    
				break;

			}
			case OP_GOTO:{
				state.program_c++;
				
				char addr_str[8] = {0};
				int addr_len = 0;
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					addr_str[addr_len++] = state.work_stack[sp];
					sp--;
				}
				for (int i = 0; i < addr_len / 2; i++){
					char temp = addr_str[i];
					addr_str[i] = addr_str[addr_len -1 -i];
					addr_str[addr_len -1 -i] = temp;
				}
				
				strtodig(addr_str, &state.program_c);
				if (state.program_c > MAX_FILE_SIZE){
					state.program_c = 0;
				}
				state.stack_pointer = sp + 1;
				break;
			}
			case OP_ABS_GOTO: {
				state.program_c = 0;
				break;
			}
			case OP_EQ: {
				if (state.stack_pointer < 3) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				char num1_str[32] = {0};
				char num2_str[32] = {0};
				char op = 0;

				int num1_len = 0;
				int num2_len = 0;
    
				int sp = state.stack_pointer - 1;
				
				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num2_str[num2_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num2_len / 2; i++) {
					char temp = num2_str[i];
					num2_str[i] = num2_str[num2_len - 1 - i];
					num2_str[num2_len - 1 - i] = temp;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				if (sp >= 0) {
					op = state.work_stack[sp];
					sp--;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				int num1 = 0, num2 = 0;
				
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				for (int i = 0; i < num2_len; i++) {
					num2 = num2 * 10 + (num2_str[i] - '0');
				}
				
				int result;
				switch (op) {
					case '=': 
						result = (num1 == num2);
						break;
					case '>': 
						result = (num1 > num2);
						break;
					case '<': 
						result = (num1 < num2);
						break;
					case '!': 
						result = (num1 != num2);
						break;
					default:
						push_text("\nRun time error! Unknown comparison operator:(");
						push_char(op);
						push_text(")\n");
						input_mode = 0;
						return;
				}
				state.stack_pointer = sp + 1;
    
                //1 - eq; 0 - not eq
				state.work_stack[state.stack_pointer++] = result ? '1' : '0';
    
				break;
			}
			case OP_JMP_IF_TRUE: {
				if (state.stack_pointer < 1) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				
				state.program_c++;
				char condition = state.work_stack[--state.stack_pointer];
    
				if (condition == '1') {
					state.program_c++;
					unsigned int address = 0;
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						address = address * 10 + (state.program[state.program_c] - '0');
						state.program_c++;
					}
					
					if (state.program_c > MAX_FILE_SIZE){
						state.program_c = 0;
					}
        
					state.program_c = address;
				} else {
					state.program_c++; 
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						state.program_c++;
					}
				}
				break;
			}
			case OP_JMP_IF_FALSE: {
				if (state.stack_pointer < 1) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				state.program_c++;
				char condition = state.work_stack[--state.stack_pointer];
    
				if (condition == '0') {
					state.program_c++;
					unsigned int address = 0;
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						address = address * 10 + (state.program[state.program_c] - '0');
						state.program_c++;
					}
					if (state.program_c > MAX_FILE_SIZE){
						state.program_c = 0;
					}
        
					state.program_c = address;
				} else {
					state.program_c++; 
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						state.program_c++;
					}
				}
				break;
			}
			case OP_ABS: {
				char num1_str[32] = {0};
				char op;
				
				int num1_len = 0;
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				if (sp >= 0) {
					op = state.work_stack[sp];
					sp--;
				}
				
				int num1 = 0;
				
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				if (op == '-'){
					num1 = -num1;
				}
				
				if (num1 < 0){
					num1 = -num1;
				}
				else{
					num1 = num1;
				}
				
				state.stack_pointer = sp + 1;
				
				char strans[32];
				int temp = num1;
				int digits = 0;
				
				do {
					digits++;
					temp /= 10;
				} while (temp > 0);
				
				temp = num1;
				for (int i = digits - 1; i >= 0; i--) {
					strans[i] = '0' + (temp % 10);
					temp /= 10;
				}
				strans[digits] = '\0';
				
				for (int i = 0; i < digits; i++) {
					state.work_stack[state.stack_pointer++] = strans[i];
				}
    
				break;
				
			}
			case OP_DELAY: {
				char num1_str[32] = {0};
				
				int num1_len = 0;
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				int num1 = 0;
				
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				slp(num1);
				break;
				
				
			}
			case OP_RAW_TIME: {
				unsigned long unixt = get_unix_t();
				
				char time_str[16];
				digtostr(unixt, time_str);
				
				for (int i = 0; time_str[i] != '\0'; i++){
					state.work_stack[state.stack_pointer++] = time_str[i];
				}
				break;
				
			}
			case OP_SET_REG: {
				state.program_c++;
				int reg_num = state.program[state.program_c] - '0';
				if (reg_num >= 0 && reg_num <= 7){
					strnumbercopy(state.registers[reg_num], state.work_stack, 16);
					if (state.stack_pointer >= 16){
						state.stack_pointer -= 16;
					}
					else {
						state.stack_pointer = 0;
					}
				}
				else {
					push_text("\nRun time error! Unknown register number!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				break;
			}
			case OP_LOAD_REG: {
				state.program_c++;
				int reg_num = state.program[state.program_c] - '0';
				if (reg_num >= 0 && reg_num <= 7){
					for (int i = 0; state.registers[reg_num][i] != '\0'; i++){
						state.work_stack[state.stack_pointer++] = state.registers[reg_num][i];
					}
				}
				else {
					push_text("\nRun time error! Unknown register number!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				break;
			}
			case OP_STR_EQ: {
				state.program_c++;
				
				unsigned int il1 = 0;
				unsigned int il2 = 0;
				
				unsigned char str1[256];
				unsigned char str2[256];
				
				unsigned int equal;
				
				unsigned int sp = 0;
				while (state.program[state.program_c] != '\0' && state.program[state.program_c] != '\n' && state.program[state.program_c] != ' '){
					str1[il1] = state.program[state.program_c++];
					il1++;
				}
				str1[il1] = '\0';
				while (state.work_stack[sp] != '\0' && state.work_stack[sp] != '\n'){
					str2[il2] = state.work_stack[sp];
					state.work_stack[sp++] = '\0';
					il2++;
				}
				str2[il2] = '\0';
				state.work_stack[sp] = '\0';
				if (il1 == il2){
					if (streq(str1, str2) == 0){
						equal = 1;
					}
					else {
						equal = 0;
					}
				}
				else {
					equal = 0;
				}
				state.stack_pointer = 0;
				
				state.work_stack[state.stack_pointer++] = equal ? '1' : '0';
				
				state.program_c++;
				break;
			}
			case OP_RANDOM: {
				unsigned int rand = seeding_rnd(state.program_c, 0xFF);
				
				char rand_str[4];
				digtostr(rand, rand_str);
				
				for (int i = 0; rand_str[i] != '\0'; i++){
					state.work_stack[state.stack_pointer++] = rand_str[i];
				}
				
				break;
			}
			default:
				push_text("\nRun time error! Unknown operation code!\nExit the program...\n");
				input_mode = 0;
				return;
		}
		if (state.stack_pointer >= sizeof(state.work_stack)) {
			push_text("\nRun time error! Stack overflow!\nExit the program...\n");
			input_mode = 0;
			return;
		}
	}
}