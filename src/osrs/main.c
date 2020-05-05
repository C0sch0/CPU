#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// #include "process.c"


// ############################################################################
// First, structures we will use to simulate queues, queue nodes and processes
struct process{
	int pid;
	char name[33];
	int start_time;
	char current_state[10];
	int bursts_count;
	int burst_idx;
	int* burst_sequence;
  int turnaround;
	int running_time;
	int interrumped;
	int cpu_select;
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

struct queue{
	int total_nodes;
	Node* first;
	Node* last;
};
typedef struct queue Queue;


// END STRUCTURES
// ############################################################################
// Initialize

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

Process* new_process(int pid, char* name, int start_time, int bursts_count, int* burst_sequence){
	Process* process = malloc(sizeof(Process));
	strcpy(process -> name, name);
	process -> start_time = start_time;
	process -> bursts_count = bursts_count;
	process -> burst_sequence = burst_sequence;
	strcpy(process -> current_state, "NULL");
	return process;
}


// End initialization
// ############################################################################
// Functions


Node* get_node(int index, Queue* queue)
	{
		Node* node_ = queue -> first;
		int counter = 0;
		while (counter != index){
			node_ = node_ -> next_process;
			counter ++;
		}
		//printf("I'm returning: %s\n", node_ -> process -> name);
		return node_;
	}

Process* remove_node_by_idx(int index, Queue* queue){
	if (index == 0)
	{
		Process* dequeued_process = queue -> first -> process;
		if (queue -> first == queue -> last){
			free(queue -> first);
			queue -> first = NULL;
			queue -> last = NULL;
		}
		else {
			Node* n = queue -> first;
			queue -> first = queue -> first -> next_process;
			free(n);
	}
	queue -> total_nodes -= 1;
	return dequeued_process;
	}
	Node* parent = NULL;
	Node* selected_node = queue -> first;
	Node* child = selected_node -> next_process;
	Process* dequeued_process;
	int i = 0;
	while(i < index){
		parent = selected_node;
		selected_node = child;
		child =  child -> next_process;
		i++;
	}
	if (!child){
		parent -> next_process = NULL;
		queue -> last = parent;
	}
	else{
		parent -> next_process = selected_node -> next_process;
	}
	dequeued_process = selected_node -> process;
	selected_node -> next_process = NULL;
	free(selected_node);
	queue -> total_nodes -= 1;
	return dequeued_process;
}


Process *_pop(Queue *queue)
{
	Process* deleted = queue -> first -> process;
	if (queue -> first == queue -> last){
		free(queue -> first);
		queue -> first = NULL;
		queue -> last = NULL;
	}
	else {
		Node* first = queue -> first;
		queue -> first = queue -> first -> next_process;
		free(first);
	}
	queue -> total_nodes -= 1;
	return deleted;
}


int isQEmpty(Queue* queue){
	if(queue -> last == NULL && queue -> first == NULL){
		return 1;
	}
	else{
		return 0;
	}
}

void set_next_node(Node* node, Node* new_node){
	node -> next_process = new_node;
}

void _insert_node_in_queue(Process* process, Queue* queue){
	Node* new_node = node_init(process);
	if (isQEmpty(queue)){
		queue -> first = new_node;
		queue -> last = new_node;
	}
	else{
		set_next_node(queue -> last, new_node);
		queue -> last = new_node;
	}
	queue -> total_nodes += 1;
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
	char version[3];
	strcpy(version, argv[3]);
  fscanf(input_file, "%d", &process_count);
  printf("version: %s,  quantum: %d \n", version, quantum);
  printf("Number of processes = %d \n", process_count);

  // Read every line, create all processes with burst information
	Process** all_processes;
	all_processes = malloc(sizeof(Process*)*process_count);

  for (int process_n = 0; process_n < process_count; process_n++)
  {
    char name[33];
    int init_time;
    int bursts_count;
    fscanf(input_file, "%s %d %d ", name, &init_time, &bursts_count);
    printf("My name is: %s\n", name);
  	printf("My init time is: %d\n", init_time);
    printf("My bursts_count is: %d\n", bursts_count);

		int total_bursts;
    total_bursts = (bursts_count * 2) - 1;

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

		Process* _process = new_process(process_n, name, init_time, bursts_count, burst_sequence);
		all_processes[process_n] = _process;

  }


	Queue* ready_queue = queue_init();
	Queue* waiting_queue = queue_init();
	Queue* finished_queue = queue_init();
	//ready_queue -> total_nodes = 0;
	//waiting_queue -> total_nodes = 0;
	//finished_queue -> total_nodes = 0;
	int simulation_complete = 0;
	int simulation_time = 0;
	Process* cpu = NULL;
	printf("--------------------INIT SIM--------------------\n");

	while (!simulation_complete) {
		// -----------------------------------------------------------------------
		// CHECK PROCESS CREATION BY START_TIME
		for (int process_i = 0; process_i < process_count; process_i++) {
			if (all_processes[process_i] -> start_time == simulation_time)
			{
				_insert_node_in_queue(all_processes[process_i], ready_queue);
				printf("(t=%d) %s ha sido creado con estado READY\n", simulation_time, all_processes[process_i]-> name);
			}
		}
		// -----------------------------------------------------------------------
		// Check if any process is done with WAITING
		if (waiting_queue -> total_nodes > 0) {
			for(int process_idx = 0; process_idx < waiting_queue -> total_nodes; process_idx++)
			{
				Node* checking_node = get_node(process_idx, waiting_queue);
				if (checking_node -> process-> burst_sequence[checking_node -> process -> burst_idx] == 0)
				{
					checking_node -> process -> burst_idx += 1;
					strcpy(checking_node -> process -> current_state, "READY");
					_insert_node_in_queue(checking_node -> process, ready_queue);
					remove_node_by_idx(process_idx, waiting_queue);
					printf("(t=%d) %s termino su I/O. Pasa de WAITING a READY)\n", simulation_time, checking_node -> process -> name);
				}

			}
		}
		// -----------------------------------------------------------------------
		// CPU handling
		if (!cpu)
		{
			if (!isQEmpty(ready_queue))
			{
				// Shortest Time Remaining First
				// We check who meets the criteria within the READY processes
				// Look for the STM and put in on CPU
				cpu = remove_node_by_idx(0, ready_queue);
				printf("(t = %i) %s ingresa a CPU\n", simulation_time, cpu -> name);
				cpu -> cpu_select += 1;
			}
		}
		else
		{
			// CPU is handling a process. should we remove it?
			if (strcmp(version, "np") == 0) {
				//printf("Non-preemptive\n");
				// Has the CPU process finished its burst ?
				if (cpu -> burst_sequence[cpu -> burst_idx] == 0)
				{
					// The process is done with its current burst
					// Is it the last burst though?
					if (cpu -> burst_idx == ((cpu -> bursts_count) * 2) - 2)
					{
						cpu -> finished = simulation_time;
						strcpy(cpu -> current_state, "FINISHED");
						_insert_node_in_queue(cpu, finished_queue);
						printf("(t = %i) %s finalizo. Pasa de READY a FINISHED.\n", simulation_time, cpu -> name);
						cpu = NULL;
					}
					else
					{
						// Current CPU burst is over, yet not done. moving to WAITING
						cpu -> interrumped += 1;
						cpu -> burst_idx += 1;
						cpu -> running_time = 0;
						strcpy(cpu -> current_state, "WAITING");
						_insert_node_in_queue(cpu, waiting_queue);
						printf("(t = %i) %s termino CPU burst. Pasa a WAITING. \n", simulation_time, cpu -> name);
						cpu = NULL;
					}
				}
			}
			else{
				// Has it achieved quantum or finished ?
				//printf("preemptive\n");
				if (cpu -> burst_sequence[cpu -> burst_idx] == 0)
				{
					// The process is done with its current burst
					// Is it the last burst ?
					if (cpu -> burst_idx == ((cpu -> bursts_count) * 2) - 2)
					{
						// Last burst !
						cpu -> finished = simulation_time;
						strcpy(cpu -> current_state, "FINISHED");
						_insert_node_in_queue(cpu, finished_queue);
						printf("(t = %i) %s finalizo \n", simulation_time, cpu -> name);
						cpu = NULL;
					}
					else
						{
						// Current burst is over, yet not done, moving over to I/O WAITING
						cpu -> interrumped += 1;
						cpu -> burst_idx += 1;
						cpu -> running_time = 0;
						strcpy(cpu -> current_state, "WAITING");
						_insert_node_in_queue(cpu, waiting_queue);
						printf("(t = %i) %s termino CPU burst. Pasa a WAITING. \n", simulation_time, cpu -> name);
						cpu = NULL;
					}
				}
				else if (cpu -> running_time == quantum) {
					cpu -> interrumped += 1;
					cpu -> running_time = 0;
					strcpy(cpu -> current_state, "READY");
					_insert_node_in_queue(cpu, ready_queue);
					printf("(t = %i) %s alcanzo quantum. Pasa de RUNNING a READY)\n", simulation_time, cpu -> name);
					cpu = NULL;
				}


			}
		}

		// What is time ?
		// Add time to waiting queue
		if (waiting_queue -> total_nodes > 0)
		{
			for(int process_id = 0; process_id < waiting_queue -> total_nodes; process_id++)
			{
				Node* checking_node = get_node(process_id, waiting_queue);
				checking_node -> process -> waiting_time += 1;
				checking_node -> process -> burst_sequence[checking_node -> process -> burst_idx] -= 1;
			}
		}

		// Add time to ready queue
		if (ready_queue -> total_nodes > 0)
		{
			for(int process_id = 0; process_id < ready_queue -> total_nodes; process_id++)
			{
				Node* checking_node = get_node(process_id, ready_queue);
				checking_node -> process -> waiting_time += 1;
			}
		}
		// Add time to CPU process
		if (cpu) {
			cpu -> running_time += 1;
			cpu -> burst_sequence[cpu -> burst_idx] -= 1;
		}

		if (ready_queue -> total_nodes == 0 && waiting_queue -> total_nodes == 0 && finished_queue -> total_nodes == process_count) {
			simulation_complete = 1;
			printf("Simulación terminada. %i procesos - tiempo %i\n", finished_queue -> total_nodes, simulation_time - 1);
			printf("A continuación, las estadísticas:\n");
		}


		simulation_time ++;
	}

  // fprintf(output_file, "%d, %d, SIGNAL\n", cells, iteracion_inicial - 1);
  fclose(input_file);
  fclose(output_file);

  return 0;
}
