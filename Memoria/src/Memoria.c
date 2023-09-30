/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "../include/Memoria.h"

int main(int argc, char** argv) {
	memoria_logger = log_create("Memoria.log", "[Memoria]", 1, LOG_LEVEL_INFO);
	memoria_log_obligatorio = log_create("Memoria_log_obligatorio.log", "[Memoria- Log obligatorio]", 1, LOG_LEVEL_INFO);

	memoria_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//char* config_path = "../memoria.config";
	//memoria_config = config_create(config_path);

	if(memoria_config == NULL){
		log_error(memoria_logger, "No se encontro el path \n.");
		config_destroy(memoria_config);
		log_destroy(memoria_logger);
		exit(1);
	}

	leer_config(memoria_config);
	//leer_log();
	const char *path_instrucciones = "/home/utnso/tp-2023-2c-En_esta_la_firmamOS/prueba_instrucciones/psudocodigos.txt";
	t_list* instrucciones_pseudo = lista_instrucciones(memoria_logger, PATH_INSTRUCCIONES);

	//TODO: verificar como inicializar memoria

	inicializar_memoria();

	//inicializar_memoria();


	server_fd_memoria = iniciar_servidor(memoria_logger, IP_MEMORIA, PUERTO_ESCUCHA);

	//TODO: ¿Conectar a filesystem antes o después de la escucha?
	//fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);

	//TODO: Se que esta la función de server_escucha pero dejo esto comentado para tener una guía
	//fd_kernel = esperar_cliente(memoria_logger, "Kernel", server_fd_memoria);
	//fd_kernel = esperar_cliente(memoria_logger, "File System", server_fd_memoria);
	//fd_kernel = esperar_cliente(memoria_logger, "CPU", server_fd_memoria);

	while(server_escucha()){
		//log_info(memoria_logger, "Se abre servidor de Memoria");
	}
	liberar_lista_instrucciones(instrucciones_pseudo);
	finalizar_memoria();

	return EXIT_SUCCESS;
}
void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(config, "PUERTO_FILESYSTEM");
	TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
	PATH_INSTRUCCIONES = config_get_string_value(config, "PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_int_value(config, "RETARDO_RESPUESTA");
	ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
}

void leer_log(){
	log_info(memoria_logger, "IP_MEMORIA: %s", IP_MEMORIA);
	log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
	log_info(memoria_logger, "IP_FILESYSTEM: %s", IP_FILESYSTEM);
	log_info(memoria_logger, "PUERTO_FILESYSTEM: %s", PUERTO_FILESYSTEM);
	log_info(memoria_logger, "TAM_MEMORIA: %d",TAM_MEMORIA);
	log_info(memoria_logger, "TAM_PAGINA: %d",TAM_PAGINA);
	log_info(memoria_logger, "PATH_INSTRUCCIONES: %s",PATH_INSTRUCCIONES);
	log_info(memoria_logger, "RETARDO_RESPUESTA: %d",RETARDO_RESPUESTA);
	log_info(memoria_logger, "ALGORITMO DE ASIGNACION: %s \n",ALGORITMO_REEMPLAZO);

}
void inicializar_memoria(){
	log_info(memoria_logger, "Inicianlizando memoria");
	espacio_usuario = malloc(TAM_MEMORIA);
	lst_marco = list_create();
	int cant_marcos = TAM_MEMORIA/TAM_PAGINA;


	for(int i=0;i<= cant_marcos;i++){
		Marco* nuevo_marco  = crear_marco(TAM_PAGINA*i, true);

		list_add(lst_marco,nuevo_marco);

	}

}


Marco* crear_marco(int base, bool presente){
	Marco *nuevo_marco = malloc(sizeof(Marco));
	nuevo_marco->base = base;
	nuevo_marco->presente = presente;
	return nuevo_marco;
}
/*
tabla_paginas* crear_tabla_paginas(int pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
	log_debug(memoria_log_obligatorio,"[PAG]: Creo tabla de paginas PID %d", pid);
	char* spid[4];
	string_from_format(spid, "%d", pid);
	nueva_tabla->page = list_create();

	//es una variable que deberia de bloquear
	dictionary_put(tablas,spid, nueva_tabla);
	// aca deberia de desbloquear este recurso
	return nueva_tabla;


}
*/
void finalizar_memoria(){
	log_destroy(memoria_logger);
	log_destroy(memoria_log_obligatorio);
	config_destroy(memoria_config);
}

void atender_mensajes_kernel(t_buffer* buffer){
	char* mensaje = recibir_string_del_buffer(buffer);
	log_info(memoria_logger, "[KERNEL]> %s", mensaje);
	free(mensaje);
}

/*----------------TODO COMUNICACION SOCKETS --------*/

void identificar_modulo(t_buffer* unBuffer, int conexion){
	int modulo_id = recibir_int_del_buffer(unBuffer);
	switch (modulo_id) {
		case KERNEL:
			fd_kernel = conexion;
			log_info(memoria_logger, "!!!!! KERNEL CONECTADO !!!!!");
			break;
		case CPU:
			fd_cpu = conexion;
			log_info(memoria_logger, "!!!!! CPU CONECTADO !!!!!");
			break;
		case FILESYSTEM:
			fd_filesystem = conexion;
			log_info(memoria_logger, "!!!!! FILESYSTEM CONECTADO !!!!!");
			break;
		default:
			log_error(memoria_logger, "[%d]Error al identificar modulo",modulo_id);
			exit(EXIT_FAILURE);
			break;
	}
}

static void procesar_conexion(void *void_args){
	int* args = (int*) void_args;
	int cliente_socket = *args;
//	int valor1, valor2;
//	char* myString;
//	char* unChoclo;
//	t_buffer* myBuffer = malloc(sizeof(t_buffer));
//	int size;
//	t_list* paquete_recibido;
	//op_code cod_op;
	int control_key = 1;
	printf("Procesando conexion\n");
	while(control_key){
		int cod_op = recibir_operacion(cliente_socket);
		t_buffer* unBuffer;
		switch(cod_op){
		case MENSAJE:
			recibir_mensaje(memoria_logger, cliente_socket);
			break;
		case PAQUETE:
			//int* numero_xd = recibir_int(logger, coso)

			t_list* paquete_recibido = recibir_paquete_int(cliente_socket);
			//t_list* paquete_recibido = recibir_paquete(cliente_socket);
			log_info(memoria_logger, "Se reciben los siguientes paqubetes: ");
			list_iterate(paquete_recibido, (void*)iterator);

			break;
		case ADMINISTRAR_PAGINA_MEMORIA:

			break;
		case IDENTIFICACION:
			unBuffer = recibiendo_super_paquete(cliente_socket);
			identificar_modulo(unBuffer, cliente_socket);
			break;
		//======= KERNEL ===========================
		case INICIAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			//
			break;
		case LIBERAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			//
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			atender_mensajes_kernel(unBuffer);
			break;
		//======= CPU ===========================
		case PETICION_INFO_RELEVANTE_CM:
			unBuffer = recibiendo_super_paquete(fd_cpu);
			//
			break;
		case PETICION_DE_INSTRUCCIONES_CM:
			unBuffer = recibiendo_super_paquete(fd_cpu);
			//
			break;
		case PETICION_DE_EJECUCION_CM:
			unBuffer = recibiendo_super_paquete(fd_cpu);
			//
			break;
		case CONSULTA_DE_PAGINA_CM:
			unBuffer = recibiendo_super_paquete(fd_cpu);
			//
			break;
		//======= FILESYSTEM ===========================
		case PETICION_ASIGNACION_BLOQUE_SWAP_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case LIBERAR_PAGINAS_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case PETICION_PAGE_FAULT_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case CARGAR_INFO_DE_LECTURA_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case GUARDAR_INFO_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		//==================================================
		case PRUEBAS:

			break;

		case -1:
			control_key = 0;
			log_error(memoria_logger, "CLIENTE DESCONCETADO");
			break;
		default:
			log_error(memoria_logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}

	}
}


void iterator(int *value) {
	log_info(memoria_logger, "%d", *value);
}

void saludar_cliente(void *void_args){
	int* conexion = (int*) void_args;
	//int cliente_socket = *args;
	printf("\nGestionando saludo\n");

	int code_op = recibir_operacion(*conexion);
	switch (code_op) {
		case HANDSHAKE:
			printf("Se detecto HANDSHAKE\n");
			void* coso_a_enviar = malloc(sizeof(int));
			int respuesta = 1;
			memcpy(coso_a_enviar, &respuesta, sizeof(int));
			send(*conexion, coso_a_enviar, sizeof(int),0);
			free(coso_a_enviar);

			printf("Rpta enviada\n");
			procesar_conexion(conexion);
			break;
		case -1:
			log_error(memoria_logger, "Deseconexion en HANDSHAKE");
			break;
		default:
			log_error(memoria_logger, "ERROR EN HANDSHAKE: Operacion desconocida");
			break;
	}
}


int server_escucha(){
	server_name = "Memoria";
	int cliente_socket = esperar_cliente(memoria_logger, server_name, server_fd_memoria );
	if(cliente_socket != -1){
		pthread_t hilo_cliente;
		int *args = malloc(sizeof(int));
		//args = &cliente_socket;
		*args = cliente_socket;
		pthread_create(&hilo_cliente, NULL, (void*) saludar_cliente, (void*)args);
		log_info(memoria_logger, "[THREAD] Creo hilo para atender");
		pthread_detach(hilo_cliente);
		return 1;
	}
	log_info(memoria_logger, "Se activa el servidor %s ", server_name);
	return 0;
}

void enviar_instrucciones_a_cpu(){
	lista_instrucciones(memoria_logger, PATH_INSTRUCCIONES);
}








