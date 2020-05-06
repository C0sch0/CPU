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
	int created_time;
	int start_time;
	char current_state[10];
	int bursts_count; // Total bursts
	int burst_idx; // Which burst are we currently reading
	int* burst_sequence;
  int turnaround;
	int running_time;
	int interrumped;
	int total_remaining_time;
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

Process* new_process(int pid, char* name, int created_time, int bursts_count, int* burst_sequence, int total_remaining_time){
	Process* process = malloc(sizeof(Process));
	strcpy(process -> name, name);
	process -> created_time = created_time;
	process -> start_time = -1;
	process -> bursts_count = bursts_count;
	process -> burst_idx = 0;
	process -> total_remaining_time = total_remaining_time;
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
		else
		{
			Node* n = queue -> first;
			queue -> first = queue -> first -> next_process;
			free(n);
		}
		queue -> total_nodes -= 1;
		return dequeued_process;
	}
	else
	{
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
		if (!child)
		{
			parent -> next_process = NULL;
			queue -> last = parent;
		}
		else
		{
			parent -> next_process = selected_node -> next_process;
		}
		dequeued_process = selected_node -> process;
		selected_node -> next_process = NULL;
		free(selected_node);
		queue -> total_nodes -= 1;
		return dequeued_process;
	}
}

int minimun_in_array(int array[], int lenght)
	{
		int min_in_array = array[0];
		for (int index = 0; index < lenght; index++)
		{
			if (array[index] < min_in_array)
			{
				min_in_array = array[index];
			}
		}
		return min_in_array;
	}


Process* shortest_remaining_time(Queue* queue)
	{
		int array_total_remainings[queue -> total_nodes];
		for (int idx = 0; idx < queue -> total_nodes; idx++)
		{
			Node* checking_node = get_node(idx, queue);
			array_total_remainings[idx] = checking_node -> process -> total_remaining_time;
			printf("%s has total remaining of : %i\n", checking_node -> process -> name,checking_node -> process -> total_remaining_time);
		}
		// Min value in the array
		int min_remaining_time;
		min_remaining_time = minimun_in_array(array_total_remainings, queue -> total_nodes);

		int repeated = 0;
		for (int _idx = 0; _idx < queue -> total_nodes; _idx++)
		{
			if (array_total_remainings[_idx] == min_remaining_time)
			{
				repeated += 1;
			}
		}

		if (repeated == 1)
		{
			for (int idx = 0; idx < queue -> total_nodes; idx++)
			{
				Node* checking_node = get_node(idx, queue);
				if (checking_node -> process -> total_remaining_time == min_remaining_time)
				{
					return remove_node_by_idx(idx, queue);
				}
			}
		}
		else
		{
			// Tie between processes, we must choose by current CPU burst
			int tie_breaker[repeated];
			int current_index = 0;
			for (int idx = 0; idx < queue -> total_nodes; idx++)
			{
				Node* checking_node = get_node(idx, queue);
				if (checking_node -> process -> total_remaining_time == min_remaining_time)
				{
					tie_breaker[current_index] = checking_node -> process -> burst_sequence[checking_node -> process -> burst_idx];
					current_index += 1;
				}
			}
			// We extracted all the CPU bursts from the tied processes.
			// Now, we calculate the min in the array, and extract the process.
			int min_current_cpu_burst = minimun_in_array(tie_breaker, repeated);

			// Now we have the info of the chosen one
			for (int idx = 0; idx < queue -> total_nodes; idx++)
			{
				Node* checking_node = get_node(idx, queue);
				if (checking_node -> process -> total_remaining_time == min_remaining_time && checking_node -> process -> burst_sequence[checking_node -> process -> burst_idx] == min_current_cpu_burst)
				{
					return remove_node_by_idx(idx, queue);
				}
			}
		}
	}


void _insert_node_in_queue(Process* process, Queue* queue){
	Node* new_node = node_init(process);
	if (queue -> last == NULL && queue -> first == NULL){
		queue -> first = new_node;
		queue -> last = new_node;
	}
	else{
		queue -> last -> next_process = new_node;
		queue -> last = new_node;
	}
	queue -> total_nodes += 1;
}
// End Functions
// ############################################################################
// Memory handlers

// End Memory handlers
// ############################################################################
// Simulation

int quantum;
int length;

int main(int argc, char const *argv[])
{
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
    fscanf(input_file, "%s %d %d", name, &init_time, &bursts_count);
		printf("\n");
    printf("My name is: %s\n", name);
  	printf("My init time is: %d\n", init_time);
    printf("My bursts_count is: %d\n", bursts_count);

		int total_bursts;
		int total_remaining_time = 0;
    total_bursts = (bursts_count * 2) - 1;
		int* burst_sequence = malloc(sizeof(int) * total_bursts);

		for(int burst_index = 0; burst_index < total_bursts; burst_index++)
		{
			fscanf(input_file, "%i", &burst_sequence[burst_index]);
		}

		printf("My sequence is: \n");
		for (int i = 0; i < total_bursts; i++)
		{
			int idx = i % 2;
			if (idx == 0)
			{
				total_remaining_time += burst_sequence[i];
			  printf("CPU: %d ", burst_sequence[i]);
			}
			else
			{
				printf("I/O: %d ", burst_sequence[i]);
		 	}
		}
		printf("\n");
		Process* _process = new_process(process_n, name, init_time, bursts_count, burst_sequence, total_remaining_time);
		all_processes[process_n] = _process;
  }

	Queue* ready_queue = queue_init();
	Queue* waiting_queue = queue_init();
	Queue* finished_queue = queue_init();

	int simulation_complete = 0;
	int simulation_time = 0;
	Process* cpu = NULL;
	printf("--------------------INIT SIM--------------------\n");

	while (!simulation_complete)
	{
		// -----------------------------------------------------------------------
		// CHECK PROCESS CREATION BY created_time
		for (int process_i = 0; process_i < process_count; process_i++)
		{
			if (all_processes[process_i] -> created_time == simulation_time)
			{
				_insert_node_in_queue(all_processes[process_i], ready_queue);
				printf("(t = %d) %s ha sido creado con estado READY\n", simulation_time, all_processes[process_i]-> name);
			}
		}
		// -----------------------------------------------------------------------
		// Check if any process is done with WAITING
		if (waiting_queue -> total_nodes > 0)
		{
			for(int process_idx = 0; process_idx < waiting_queue -> total_nodes; process_idx++)
			{
				Node* checking_node = get_node(process_idx, waiting_queue);
				if (checking_node -> process-> burst_sequence[checking_node -> process -> burst_idx] == 0)
				{
					checking_node -> process -> burst_idx += 1;
					strcpy(checking_node -> process -> current_state, "READY");
					_insert_node_in_queue(checking_node -> process, ready_queue);
					remove_node_by_idx(process_idx, waiting_queue);
					printf("(t = %d) %s termino su I/O. Pasa de WAITING a READY)\n", simulation_time, checking_node -> process -> name);
					printf("Su waiting time actual: %i\n", checking_node -> process -> waiting_time);

				}
			}
		}

		if (cpu)
		{
			cpu -> running_time += 1;
			cpu -> burst_sequence[cpu -> burst_idx] -= 1;
			cpu -> total_remaining_time -= 1;
			printf("(t= %d) %s is running, with %d left\n", simulation_time, cpu -> name, cpu -> burst_sequence[cpu -> burst_idx]);
		}

		// -----------------------------------------------------------------------
		// CPU handling

		if (cpu)
		{
			// CPU is handling a process. should we remove it?
			if (strcmp(version, "np") == 0)
			{
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
						cpu -> turnaround = cpu -> finished;
						cpu -> turnaround -= cpu -> start_time;
						printf("%s finished at %d and started on %d\n",cpu ->name, cpu -> finished, cpu -> start_time );
						_insert_node_in_queue(cpu, finished_queue);
						printf("(t = %i) %s finalizo. Pasa de READY a FINISHED.\n", simulation_time, cpu -> name);
						cpu = NULL;
					}
					else
					{
						// Current CPU burst is over, yet not done. moving to WAITING
						//cpu -> interrumped += 1;
						cpu -> burst_idx += 1;
						strcpy(cpu -> current_state, "WAITING");
						_insert_node_in_queue(cpu, waiting_queue);
						printf("(t = %i) %s termino CPU burst. Pasa a WAITING. \n", simulation_time, cpu -> name);
						printf("running time %i\n", cpu -> running_time);
						cpu = NULL;
					}
				}
			}
			else
			{
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
				else if (cpu -> running_time == quantum)
				{
					cpu -> interrumped += 1;
					cpu -> running_time = 0;
					strcpy(cpu -> current_state, "READY");
					_insert_node_in_queue(cpu, ready_queue);
					printf("(t = %i) %s alcanzo quantum. Pasa de RUNNING a READY)\n", simulation_time, cpu -> name);
					cpu = NULL;
				}
			}
		}

		if (!cpu)
		{
			if (!(ready_queue -> last == NULL && ready_queue -> first == NULL))
			{
				// Shortest Time Remaining First
				// We check who meets the criteria within the READY processes
				cpu = shortest_remaining_time(ready_queue);
				printf("(t = %i) %s ingresa a CPU\n", simulation_time, cpu -> name);
				if (cpu -> start_time == -1) {
					cpu -> start_time = simulation_time;
					cpu -> response_time = cpu -> start_time;
					cpu -> response_time -= cpu -> created_time;
				}
				cpu -> cpu_select += 1;

				printf("response time: %i / waiting time: %i\n", cpu->response_time, cpu->waiting_time);
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


		// Are we done ?
		if (ready_queue -> total_nodes == 0 && waiting_queue -> total_nodes == 0 && finished_queue -> total_nodes == process_count)
		{
			simulation_complete = 1;
		}

		simulation_time ++;
	}

	printf("--------------------END SIM--------------------\n");
	printf("%i procesos en tiempo %i\n", finished_queue -> total_nodes, simulation_time - 1);

	for (int idx = 0; idx < finished_queue -> total_nodes; idx ++)
	{
		Node* checking_node = get_node(idx, finished_queue);
		Process* process_ = checking_node -> process;
		fprintf(output_file, "%s,%d,%d,%d,%d,%d\n", process_->name, process_->cpu_select, process_->interrumped, process_->turnaround,process_->response_time,process_->waiting_time);
	}

  // fprintf(output_file, "%d, %d, SIGNAL\n", cells, iteracion_inicial - 1);
  fclose(input_file);
  fclose(output_file);

  return 0;
}
