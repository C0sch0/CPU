#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// #include "process.c"


// ############################################################################
// First, we define the structures we will use to simulate queues and processes

struct process{
	int pid;
	char name[256];
	int start_time;
	char current_state[9];
	int bursts_count;
	int burst_idx;
	int* burst_sequence;
  int turnaround;
  int response_time;
  int waiting_time;
  int ready;
	int finished;
	int aging;
};
typedef struct process Process;

struct node{
	struct node* next_process;
	Process* process;
};
typedef struct node Node;

typedef struct queue{
	int total_nodes;
	Node* first;
	Node* last;
}Queue;

// END STRUCTURES
// ############################################################################
// Initialize

Process* new_process(int pid, char* name, int start_time, int bursts_count, int* burst_sequence){
	Process* process = malloc(sizeof(Process));
	strcpy(process -> name, name);
	process -> start_time = start_time;
	process -> bursts_count = bursts_count;
	process -> burst_sequence = burst_sequence;
	strcpy(process -> current_state, "NULL");
	return process;
}

Node* node_init(Process* process){
	Node* new_node = malloc(sizeof(Node));
	new_node -> next_process = NULL;
	new_node -> process = process;
	return new_node;
}

Queue* queue_init(){
	Queue* new_queue = malloc(sizeof(Queue));
	new_queue -> total_nodes = 0;
	new_queue -> first = NULL;
	new_queue -> last = NULL;
	return new_queue;
}

// End initialization
// ############################################################################
// Functions

Process *_pop(Queue *queue)
{
	Process* deleted = queue -> first -> process;
	if (queue -> first == queue -> last){
		free(queue -> first);
		queue -> first = NULL;
		queue -> last = NULL;
	}
	else {
		Node* head = queue -> first;
		queue -> first = queue -> first -> next_process;
		free(head);
	}
	queue -> total_nodes -= 1;
	return deleted;
}


int isQEmpty(Queue* queue){
	if((queue -> head == NULL) && (queue -> rear == NULL)){
		return 1;
	}
	else{
		return 0;
	}
}

void insert_head(Process* process, Queue* queue){
	Node* new_node = node_init(process);
	if (isQEmpty(queue)){
		queue -> head = new_node;
		queue -> rear = new_node;
	}
	else{
		set_next_node(queue -> rear, new_node);
		queue -> rear = new_node;
	}
	queue -> length += 1;
	// printf("Proceso %i ha entrado a la cola\n", process -> pid);
}



// End Functions
// ############################################################################
// Memory leak handlers

// End Memory handlers
// ############################################################################
// Simulation
int quantum;
int length;

int main(int argc, char const *argv[]) {
  if (argc < 4 || argc > 5)
  {
    printf("Modo de uso: ./osrs <input_file> <output_file> <version> [<quantum>]\n");
    printf("[<quantum>] = 5 por defecto\n");
		return 0;
  }

  else if (argc == 4)
  {
    quantum = 5;
  }

  else if (argc == 5)
  {
    quantum = atoi(argv[4]);
  }

	FILE *input_file = fopen(argv[1], "r");
	FILE *output_file = fopen(argv[2], "w");

  if (!input_file)
  {
    printf("¡El archivo %s no existe!\n", argv[1]);
    return 2;
  }

  int process_count;
  fscanf(input_file, "%d", &process_count);
  printf("version: %s,  quantum: %d \n", argv[3], quantum);
  printf("Number of processes = %d \n", process_count);

  // Read every line, create all processes with burst information
	Process** all_processes;
	all_processes = malloc(sizeof(Process*)*process_count);

  for (int process_n = 0; process_n < process_count; process_n++)
  {
    char name[256];
    int init_time;
    int burst_count;
    fscanf(input_file, "%s %d %d ", name, &init_time, &burst_count);
    printf("My name is: %s\n", name);
  	printf("My init time is: %d\n", init_time);
    printf("My burst_count is: %d\n", burst_count);

		int total_bursts;
    total_bursts = (burst_count * 2) - 1;

		int* burst_sequence = malloc(sizeof(int) * total_bursts);

		for(int burst_index = 0; burst_index < total_bursts; burst_index++)
		{
			fscanf(input_file, "%i", &burst_sequence[burst_index]);
		}


		printf("And my sequence is: \n");
		for (int i = 0; i < total_bursts; i++) {
			if (i % 2 == 0)
			{
			  printf("CPU burst %d\n", burst_sequence[i]);
			}
			else{
			printf("I/O burst %d\n", burst_sequence[i]);
		 	}
		}

		Process* init_process = new_process(process_n, name, init_time, burst_count, burst_sequence);
		all_processes[process_n] = init_process;

  }


	Queue* ready_queue = queue_init();
	Queue* waiting_queue = queue_init();
	Queue* finished_queue = queue_init();
	ready_queue -> total_nodes = 0;
	waiting_queue -> total_nodes = 0;
	finished_queue -> total_nodes = 0;

	int simulation_complete = 0;
	int simulation_time = 0;


	while (!simulation_complete) {
		// We check all processes to see if any gets created this iteration
		for (int process_i = 0; process_i < process_count; process_i++) {
			if (all_processes[process_i] -> start_time == simulation_time)
			{
				insert_process(all_processes[process_i], ready_queue);
				printf("(t=%d) %s ha sido creado con estado READY\n", simulation_time, all_processes[process_i]-> name);
			}
		}

		// Check if any process is done with WAITING
		for(int process_idx = 0; process_idx < waiting_queue -> total_nodes; process_idx++)
		{
			if(strcmp(all_processes[process_idx] -> current_state, "WAITING") == 0)
			{
				if (all_processes[process_idx]-> burst_sequence[all_processes[process_idx] -> burst_idx] == 0)
				{
					all_processes[process_idx]-> burst_idx += 1;
					insert_process(all_processes[process_idx], ready_queue);
					strcpy(all_processes[process_idx] -> current_state, "READY");
					printf("(t=%d) %i pasa de WAITING a READY)\n",simulation_time, all_processes[process_idx] -> name);
				}
			}
		}


		simulation_time ++;
	}

  // fprintf(output_file, "%d, %d, SIGNAL\n", cells, iteracion_inicial - 1);
  fclose(input_file);
  fclose(output_file);

  return 0;
}
