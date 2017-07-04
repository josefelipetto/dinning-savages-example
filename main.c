#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>


#define TRIBE_MEMBERS 30
#define NUM_PORCOES 12
#define PERIOD 1000.*(1./60.)

int numRefeicoes = 0; // Numero de refeições disponíveis 
int Ref = 0;

pthread_mutex_t mut; // Controla o acesso ao caldeirão
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Cria as threads, 29 selvagens e 1 cozinheiro
pthread_t Selvagens[TRIBE_MEMBERS - 1];
pthread_t Cozinheiro;


long long milisegundos(){
    struct timeval ml;
    gettimeofday(&ml, NULL);
    long long milliseconds = ml.tv_sec * 1000LL + ml.tv_usec / 1000;
    return milliseconds;
}
void pulaTempo(long long pula){
    long long ms = milisegundos();
    while(milisegundos() - ms < pula)
        ;
}
void *selvagem(void *arg){
    while(1){
        pthread_mutex_lock(&mut);        
            // Verifica se o caldeirão está vazio
            if(numRefeicoes == 0){
                // Avisa o cozinheiro que o caldeirão está vazio
                pthread_cond_signal(&cond);
                // Manda todos Esperarem o cozinheiro cozinhar
                while(numRefeicoes != 12)
                    pthread_cond_wait(&cond,&mut);
                //numRefeicoes = 12;
            }
            // Selvagem se serve
            numRefeicoes--;
            pulaTempo(1*PERIOD);
        pthread_mutex_unlock(&mut);
        // Comer
        Ref++;
        // Selvagem come(entre 1 e 3 segundos)
        pulaTempo((rand()%(3)+1) * PERIOD);
    }
}
void *cozinheiro(void *arg){
    while(1){
        if(numRefeicoes == 0){
            // Preparar a comida
            pthread_mutex_lock(&mut);
                numRefeicoes = 12;
                pulaTempo(7*PERIOD);
                // Encher caldeirão
                pulaTempo(1*PERIOD);
                // Avisar a tribo que o caldeirão está cheio de novo
                pthread_cond_broadcast(&cond);
                // Dormir: POe a thread pra dormir. Ela será atividada de novo quando o selvagem der um signal
                pthread_cond_wait(&cond,&mut);
            pthread_mutex_unlock(&mut);
        }
    }
}
int main(){
    // Inicia os Mutex
    pthread_mutex_init(&mut,NULL);
    // Cria o cozinheiro
    pthread_create(&Cozinheiro, NULL, cozinheiro,NULL);
    // Cria os selvagens
    for(unsigned int i=0; i < TRIBE_MEMBERS -1; i++){
        pthread_create(&Selvagens[i],NULL,selvagem,NULL);
    }
    while(1){
        sleep(1);
        pthread_mutex_lock(&mut);
        printf("Refeicoes por segundo: %d\n",Ref);
        Ref = 0;
        pthread_mutex_unlock(&mut);
        
    }
}
