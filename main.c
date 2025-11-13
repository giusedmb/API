#include <stdio.h>
#include <stdlib.h>
#include <string.h>
double load_factor_limite = 0.5;
int numero_ricette_max = 25006;
int numero_ricette_presenti = 0;
int storage_magazzino_max = 25006;
int storage_magazzino_attuale=0;
float load_factor = 0.5;
int wait_list_qta = 0;
int wait_pickup_qta = 0;
typedef struct nodo_lista_magazzino_tipo nodo_lista_magazzino_tipo;

typedef struct corriere_ordine_tipo {
    int istante_arrivo_ordine;
    char* nome_ricetta;
    int numero_elementi_ordinati;
    int peso;
    struct corriere_ordine_tipo* next;
}corriere_ordine_tipo;

struct nodo_lista_magazzino_tipo{
    struct nodo_lista_magazzino_tipo* next_node;
    //char* nome_ingrediente;
    int qta;
    int scad;
};

typedef struct magazzino_tipo{
    char* nome_ingrediente;
    nodo_lista_magazzino_tipo* node_magazzino;
    int qta_tot;
    char tomb_magazzino;
}magazzino_tipo;

typedef struct ingrediente_tipo{ //ingrediente
    //char* nome_ingrediente; //uso un puntatore così posso usare meno memoria, allocando solo lo spazio che serve effettivamente al nome
    int qta; //int a 32 bit
    magazzino_tipo* punt_magazz;
    struct ingrediente_tipo* next_ingredient; //ogni ingrediente punta al successivo
    struct ingrediente_tipo* prev_ingredient; //ogni ingrediente punta al successivo
}ingrediente_tipo;

typedef struct ricetta_tipo{ //ricetta
    char* nome_ricetta; //uso un puntatore così posso usare meno memoria, allocando solo lo spazio che serve effettivamente al nome
    ingrediente_tipo* ingredienti_rec; //ogni ricetta punta alla lista dei suoi ingredienti
    int waiting; //segno se ho ordini in attesa per quella ricetta
}ricetta_tipo;

typedef struct ricettario_tipo{ //ricettario. base per la tabella hash
    ricetta_tipo* rec; //il campo principale del ricettario è il puntatore alla ricetta
    char tomb_recipe; //uso il closed hashing, quello in cui ho le tombe
}ricettario_tipo;

typedef struct nodo_wait_list_tipo{
    //char* ricetta_ordine;
    int tempo_ordine;
    int qta_ordinata;
    struct nodo_wait_list_tipo* next_node_w;
    struct nodo_wait_list_tipo* prev_node_w;
    ricettario_tipo* punt_ricettario;
}nodo_wait_list_tipo;

typedef struct wait_pickup_list_tipo{
    //char* ordine;
    int peso;
    int arrival_time;
    int numero_elementi_ordinati;
    struct wait_pickup_list_tipo* next_node_wp;
    struct wait_pickup_list_tipo* prev_node_wp;
    ricettario_tipo* punt_ricettario;
}wait_pickup_list_tipo;

// Funzioni di hash
long int hash_funct(char *str);
// Funzioni di gestione della wait list
//void stampa_wait_list(nodo_wait_list_tipo* wait_list);
void rimuovi_nodo_wait_list(nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, nodo_wait_list_tipo* nodo_da_rimuovere);
void inserisci_in_wait_list(nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, int time, int qta_ordine, ricettario_tipo* punt_ricettario);
void check_wait_list(nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, wait_pickup_list_tipo** wait_pickup_list, ricettario_tipo* ricettario, magazzino_tipo* magazzino, int time);
//void freeWaitList(nodo_wait_list_tipo** wait_list);
// Funzioni di gestione della wait pickup list
void inserisci_in_wait_pickup_list(wait_pickup_list_tipo** wait_pickup_list, int peso_ordine, int time,int qta_ordine, ricettario_tipo* punt_ricettario);
void rimuovi_nodo_wait_pickup_list(wait_pickup_list_tipo** wait_pickup_list, wait_pickup_list_tipo* nodo_da_rimuovere);
//void freeWaitPickUpList(wait_pickup_list_tipo** wait_pickup_list);
//void stampa_wait_pickup_list(wait_pickup_list_tipo* wait_pickup_list);
// Funzioni di gestione del magazzino
void rifornimento_magazzino(magazzino_tipo** magazzino, nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, wait_pickup_list_tipo** wait_pickup_list, ricettario_tipo* ricettario, int time, char* token);
//void stampa_magazzino_lista(magazzino_tipo* magazzino);
//void freeMagazzino_full(magazzino_tipo** magazzino);
void cancella_nodo_magazzino(nodo_lista_magazzino_tipo** nodo);
nodo_lista_magazzino_tipo* check_expired_batch(int time, nodo_lista_magazzino_tipo** nodo, magazzino_tipo* magazzino);
// Funzioni di gestione degli ingredienti
ingrediente_tipo* add_ingredients(ingrediente_tipo* var, char* token, magazzino_tipo* magazzino);
void free_ingredients(ingrediente_tipo* var);
// Funzioni di gestione del ricettario
void rimuovi_ricetta(ricettario_tipo* ricettario, char* recipe_name);
void aggiungi_ricetta(ricettario_tipo* ricettario, char* token, magazzino_tipo* magazzino);
ricettario_tipo* rehashing(ricettario_tipo* ricettario);
//void free_ricettario(ricettario_tipo* ricettario);
//void stampa_ricette(ricettario_tipo* ricettario, int numero_ricette_max);
//Gestione Ordini
int produci_ordine2(int qta_ordine, ricettario_tipo* ricettario, magazzino_tipo* magazzino);
int fattibility_ricetta2(int qta_ordine, ricettario_tipo* ricettario, magazzino_tipo* magazzino, int time);
void ordine(ricettario_tipo* ricettario, nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, wait_pickup_list_tipo** wait_pickup_list, magazzino_tipo* magazzino, int time, char* ordine_ricevuto, int qta_ordine);
//Gestione corriere
void pesi_uguali(corriere_ordine_tipo* curr, corriere_ordine_tipo* temp);
void aggiungi_lista_corriere(corriere_ordine_tipo** lista_corriere, int istante_arrivo, char* ordine_ricevuto, int numero_elementi_ordinati, int peso);
void check_corriere(wait_pickup_list_tipo** wait_pickup_list, int capacity_corriere, ricettario_tipo* ricettario);
void load_camioncino(corriere_ordine_tipo* lista_corriere);

//Funzione di Hash
long int hash_funct(char *str) { //funzione di hash per char. Mi restituisce un intero, poi fai il % grandezza della tabella per avere un indice.
    long int hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) ^ *str++;
    }
   // printf("%ld",hash);
    if(hash<0)
    return ((-1)*hash);
    else return hash;
}

/*void stampa_wait_list(nodo_wait_list_tipo* wait_list) {
    nodo_wait_list_tipo* curr= wait_list;
    printf("STO STAMPANDO WAITLIST\n");
    int cont = 0;
    while (curr != NULL && cont<18) {
        printf("%s, numero elem: %d, t.arrivo: %d \n", curr->ricetta_ordine, curr->qta_ordinata,curr->tempo_ordine);
        curr = curr->next_node_w;
        cont++;
    }
    printf("FINE WAITLIST\n");
}*/

void inserisci_in_wait_list(nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, int time, int qta_ordine, ricettario_tipo* punt_ricettario) {
    nodo_wait_list_tipo* temp = (nodo_wait_list_tipo*)malloc(sizeof(nodo_wait_list_tipo));
    temp->tempo_ordine = time;
    temp->qta_ordinata = qta_ordine;
    /*temp->ricetta_ordine = (char*)malloc((strlen(ordine_ricevuto) + 1) * sizeof(char)); //aggiungi +1 per '\0'
    strcpy(temp->ricetta_ordine, ordine_ricevuto);*/
    temp->next_node_w = NULL;
    temp->prev_node_w = NULL;
    temp->punt_ricettario = punt_ricettario;
    if (*coda_wait_list == NULL) { //se la coda è vuota, il nuovo nodo è sia la testa che la coda
        *wait_list = temp;
        *coda_wait_list = temp;
    } else {
        temp->prev_node_w = *coda_wait_list;
        (*coda_wait_list)->next_node_w = temp;
        *coda_wait_list = temp;  //aggiorna il puntatore alla coda
    }
    wait_list_qta++;
}

void rimuovi_nodo_wait_list(nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, nodo_wait_list_tipo* nodo_da_rimuovere) {
    if (nodo_da_rimuovere == NULL) {
        return;
    }
    if (nodo_da_rimuovere == *wait_list) {
        *wait_list = nodo_da_rimuovere->next_node_w;
        if (*wait_list != NULL) {
            (*wait_list)->prev_node_w = NULL;
        }
    } else {
        nodo_da_rimuovere->prev_node_w->next_node_w = nodo_da_rimuovere->next_node_w;
    }
    if (nodo_da_rimuovere == *coda_wait_list) {
        *coda_wait_list = nodo_da_rimuovere->prev_node_w;
        if (*coda_wait_list != NULL) {
            (*coda_wait_list)->next_node_w = NULL;
        }
    } else {
        nodo_da_rimuovere->next_node_w->prev_node_w = nodo_da_rimuovere->prev_node_w;
    }
    //free(nodo_da_rimuovere->ricetta_ordine);
    free(nodo_da_rimuovere);
    wait_list_qta--;
}

void check_wait_list(nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, wait_pickup_list_tipo** wait_pickup_list, ricettario_tipo* ricettario, magazzino_tipo* magazzino, int time) {
    if (wait_list_qta == 0 || *wait_list == NULL) return;  //lista vuota
    nodo_wait_list_tipo* curr = *wait_list;
    nodo_wait_list_tipo* prev = NULL;
    int cont = wait_list_qta;
    while (cont > 0) {
        if (fattibility_ricetta2(curr->qta_ordinata, curr->punt_ricettario, magazzino, time)) {
            nodo_wait_list_tipo* temp = curr;
            int peso_ordine = produci_ordine2(temp->qta_ordinata, curr->punt_ricettario, magazzino);
            inserisci_in_wait_pickup_list(wait_pickup_list, peso_ordine, temp->tempo_ordine, temp->qta_ordinata, curr->punt_ricettario);
            curr = curr->next_node_w;
            rimuovi_nodo_wait_list(wait_list, coda_wait_list, temp);
            if (prev == NULL) {//sto togliendo la testa
                *wait_list = curr;
            } else { //attacca il precedente al correnteri
                prev->next_node_w = curr;
            }
        } else {
            prev = curr;
            curr = curr->next_node_w;
        }
        cont--;
    }
}


/*void freeWaitList(nodo_wait_list_tipo** wait_list) {
    nodo_wait_list_tipo* curr = *wait_list;
    while (curr != NULL) {
        nodo_wait_list_tipo* temp = curr;
        curr = curr->next_node_w;
        free(temp->ricetta_ordine);
        free(temp);
    }
}*/

//Gestione Wait Pickup List
void inserisci_in_wait_pickup_list(wait_pickup_list_tipo** wait_pickup_list, int peso_ordine, int time, int qta_ordine, ricettario_tipo* punt_ricettario) {
    wait_pickup_list_tipo* temp = (wait_pickup_list_tipo*)malloc(sizeof(wait_pickup_list_tipo));
    temp->arrival_time = time;
    temp->numero_elementi_ordinati = qta_ordine;
    temp->peso = peso_ordine;
    /*temp->ordine = (char*)malloc((strlen(ordine_ricevuto) + 1) * sizeof(char));
    strcpy(temp->ordine, ordine_ricevuto);*/
    temp->next_node_wp = NULL;
    temp->prev_node_wp = NULL;
    temp->punt_ricettario = punt_ricettario;
       if (*wait_pickup_list == NULL || (*wait_pickup_list)->arrival_time >= time) {
           temp->next_node_wp = *wait_pickup_list;
           if (*wait_pickup_list != NULL) {
               (*wait_pickup_list)->prev_node_wp = temp;
           }
           *wait_pickup_list = temp;
       } else {
           wait_pickup_list_tipo* curr = *wait_pickup_list;
           while (curr->next_node_wp != NULL && curr->next_node_wp->arrival_time < time) {
               curr = curr->next_node_wp;
           }
           temp->next_node_wp = curr->next_node_wp;
           if (curr->next_node_wp != NULL) {
               curr->next_node_wp->prev_node_wp = temp;
           }
           curr->next_node_wp = temp;
           temp->prev_node_wp = curr;
       }
    wait_pickup_qta++;
}

void rimuovi_nodo_wait_pickup_list(wait_pickup_list_tipo** wait_pickup_list, wait_pickup_list_tipo* nodo_da_rimuovere) {
    if (nodo_da_rimuovere == NULL) {
        return;
    }
    if (*wait_pickup_list == nodo_da_rimuovere) {
        *wait_pickup_list = nodo_da_rimuovere->next_node_wp;
        if (*wait_pickup_list != NULL) {
            (*wait_pickup_list)->prev_node_wp = NULL;
        }
    } else {
        if (nodo_da_rimuovere->prev_node_wp != NULL) {
            nodo_da_rimuovere->prev_node_wp->next_node_wp = nodo_da_rimuovere->next_node_wp;
        }
        if (nodo_da_rimuovere->next_node_wp != NULL) {
            nodo_da_rimuovere->next_node_wp->prev_node_wp = nodo_da_rimuovere->prev_node_wp;
        }
    }
    //free(nodo_da_rimuovere->ordine);
    free(nodo_da_rimuovere);
}


/*void freeWaitPickUpList(wait_pickup_list_tipo** wait_pickup_list){
    wait_pickup_list_tipo* curr = *wait_pickup_list;
        while (curr != NULL) {
            wait_pickup_list_tipo* temp = curr;
            curr = curr->next_node_wp;
            free(temp->ordine);
            free(temp);
        }
}

void stampa_wait_pickup_list(wait_pickup_list_tipo* wait_pickup_list) {
    wait_pickup_list_tipo* curr= wait_pickup_list;
    int cont=0;
    while (curr != NULL && cont <18) {
        printf("%s, numero elem: %d, peso: %d, t.arrivo: %d \n", curr->ordine, curr->numero_elementi_ordinati, curr->peso, curr->arrival_time);
        curr = curr->next_node_wp;
        cont++;
    }
    printf("\n");
}*/

//Funzioni Magazzino
void rifornimento_magazzino(magazzino_tipo** magazzino, nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list,wait_pickup_list_tipo** wait_pickup_list, ricettario_tipo* ricettario, int time, char* token) {
    while (token!=NULL) {
        char* ing = (char*)malloc((sizeof(char) * strlen(token)) + 1);
        strcpy(ing, token);
        token = strtok(NULL, " ");
        int qta = atoi(token);
        token = strtok(NULL, " ");
        int scad = atoi(token);
        token = strtok(NULL, " ");
        if (scad > time) {
            int pos = hash_funct(ing) % storage_magazzino_max;
            int cont = 0;
            while (cont < storage_magazzino_max) {
                    if ((*magazzino)[pos].node_magazzino == NULL) { //ho subito il posto vuoto
                        storage_magazzino_attuale++;
                        (*magazzino)[pos].nome_ingrediente = (char*)malloc((strlen(ing)+1)* sizeof(char));
                        strcpy((*magazzino)[pos].nome_ingrediente, ing);
                        nodo_lista_magazzino_tipo* temp = (nodo_lista_magazzino_tipo*)malloc(sizeof(nodo_lista_magazzino_tipo));
                        /*temp->nome_ingrediente = (char*)malloc((strlen(ing)+1)* sizeof(char));
                        strcpy(temp->nome_ingrediente, ing);*/
                        temp->qta = qta;
                        temp->scad = scad;
                        temp->next_node = NULL;
                        (*magazzino)[pos].node_magazzino = temp;
                        (*magazzino)[pos].qta_tot += qta;
                        (*magazzino)[pos].tomb_magazzino = '0';
                        break;
                    } else if (strcmp((*magazzino)[pos].nome_ingrediente, ing) == 0) { //è già presente l'ingrediente
                        nodo_lista_magazzino_tipo* nodo_curr = (*magazzino)[pos].node_magazzino;
                        nodo_lista_magazzino_tipo* prev = NULL;
                        //nodo_curr= check_expired_batch(time, &nodo_curr, *magazzino, pos);
                        while (nodo_curr != NULL && nodo_curr->scad < scad) {
                            prev = nodo_curr;
                            nodo_curr = nodo_curr->next_node;
                        }
                        if (nodo_curr != NULL && nodo_curr->scad == scad) {
                            nodo_curr->qta += qta;
                            (*magazzino)[pos].qta_tot += qta;
                        } else {
                            nodo_lista_magazzino_tipo* temp = (nodo_lista_magazzino_tipo*)malloc(sizeof(nodo_lista_magazzino_tipo));
                            /*temp->nome_ingrediente = (char*)malloc((strlen(ing)+1)* sizeof(char));
                            strcpy(temp->nome_ingrediente, ing);*/
                            temp->qta = qta;
                            temp->scad = scad;
                            temp->next_node = nodo_curr;

                            if (prev == NULL) {
                                (*magazzino)[pos].node_magazzino = temp;
                            } else {
                                prev->next_node = temp;
                            }
                            (*magazzino)[pos].qta_tot += qta;
                        }
                        break;
                    } else {
                        pos = (pos + 1) % storage_magazzino_max;
                        cont++;
                    }
            }//fine while principale
            if (((float)storage_magazzino_attuale / (float)storage_magazzino_max) >= load_factor) {
                int new_max_magazzino = storage_magazzino_max * 2;
                magazzino_tipo* new_magazzino = (magazzino_tipo*)malloc(sizeof(magazzino_tipo) * new_max_magazzino);
                for (int i = 0; i < new_max_magazzino; i++) {
                    new_magazzino[i].nome_ingrediente = NULL;
                    new_magazzino[i].node_magazzino = NULL;
                    new_magazzino[i].tomb_magazzino = '0';
                    new_magazzino[i].qta_tot = 0;
                }
                for (int i = 0; i < storage_magazzino_max; i++) {
                    if ((*magazzino)[i].tomb_magazzino == '0' && (*magazzino)[i].node_magazzino != NULL) {
                        int pos = hash_funct((*magazzino)[i].nome_ingrediente) % new_max_magazzino;
                        while (new_magazzino[pos].node_magazzino != NULL) {
                            pos = (pos + 1) % new_max_magazzino;
                        }
                        new_magazzino[pos].node_magazzino = (*magazzino)[i].node_magazzino;
                        new_magazzino[pos].nome_ingrediente = (*magazzino)[i].nome_ingrediente;
                        new_magazzino[pos].qta_tot = (*magazzino)[i].qta_tot;
                    }
                }
                storage_magazzino_max = new_max_magazzino;
                free(*magazzino);
                *magazzino = new_magazzino;
            }
            free(ing);
        } else {
            free(ing); //liberare la memoria allocata per 'ing' se la data di scadenza è scaduta
        }
    }
    check_wait_list(wait_list, coda_wait_list, wait_pickup_list, ricettario, *magazzino, time);
}


magazzino_tipo* trova_nel_magazzino(magazzino_tipo** magazzino, char* ing) {
            int pos = hash_funct(ing) % storage_magazzino_max;
            int cont = 0;
            while (cont < storage_magazzino_max) {
                    if ((*magazzino)[pos].node_magazzino == NULL) { //ho subito il posto vuoto
                        storage_magazzino_attuale++;
                        (*magazzino)[pos].nome_ingrediente = (char*)malloc((strlen(ing)+1)* sizeof(char));
                        strcpy((*magazzino)[pos].nome_ingrediente, ing);
                        nodo_lista_magazzino_tipo* temp = (nodo_lista_magazzino_tipo*)malloc(sizeof(nodo_lista_magazzino_tipo));
                        /*temp->nome_ingrediente = (char*)malloc((strlen(ing)+1)* sizeof(char));
                        strcpy(temp->nome_ingrediente, ing);*/
                        temp->qta = 0;
                        temp->scad = 0;
                        temp->next_node = NULL;
                        (*magazzino)[pos].node_magazzino = temp;
                        (*magazzino)[pos].tomb_magazzino = '0';
                        return &(*magazzino)[pos];
                    } else if (strcmp((*magazzino)[pos].nome_ingrediente, ing) == 0) { //è già presente l'ingrediente
                        return &(*magazzino)[pos];
                        break;
                    } else {
                        pos = (pos + 1) % storage_magazzino_max;
                        cont++;
                    }
            }//fine while principale
    return *magazzino;
}

void cancella_nodo_magazzino(nodo_lista_magazzino_tipo** nodo) {
    if (*nodo == NULL) {
        return;
    }
    nodo_lista_magazzino_tipo* temp = *nodo;
    *nodo = (*nodo)->next_node; //aggiorna il puntatore al nodo successivo
    //free(temp->nome_ingrediente);
    free(temp);
}

/*void stampa_magazzino_lista(magazzino_tipo* magazzino) {
     //printf("Contenuto del magazzino (lista):\n");
    printf("            stato scorte ingredienti, in tabella\n");
     for (int i = 0; i < storage_magazzino_max; i++) {
         if (magazzino[i].node_magazzino != NULL) {
             printf("quantita cumulata di %s: %d\n",magazzino[i].nome_ingrediente, magazzino[i].qta_tot);
             //printf("Posizione: %d, Ingrediente: %s, Quantità Totale: %d\n", i, magazzino[i].nome_ingrediente, magazzino[i].qta_tot);
             //nodo_lista_magazzino_tipo* curr = magazzino[i].node_magazzino;
             //while (curr != NULL) {
               //  printf("    -> Scadenza: %d, Quantità: %d\n", curr->scad, curr->qta);
                 //curr = curr->next_node;
            //}
        }
    }
 }

void freeMagazzino_full(magazzino_tipo** magazzino) {
    if (magazzino == NULL || *magazzino == NULL) {
        return;
    }
    for (int i = 0; i < storage_magazzino_max; i++) {
        if ((*magazzino)[i].nome_ingrediente != NULL && (*magazzino)[i].node_magazzino != NULL ) {
            nodo_lista_magazzino_tipo* curr = (*magazzino)[i].node_magazzino;
            while (curr != NULL) {
                nodo_lista_magazzino_tipo* next = curr->next_node;
                free(curr->nome_ingrediente); //libera la memoria per il nome dell'ingrediente
                free(curr); //libera la memoria per il nodo
                curr = next;
            }
            free((*magazzino)[i].nome_ingrediente); //libera la memoria per il nome dell'ingrediente nel magazzino
        }
    }
    free(*magazzino);  //libera la memoria per l'array di magazzino
    *magazzino = NULL; //reimposta il puntatore a NULL
}*/

void free_ingredients(ingrediente_tipo* var) { //rimuove gli ingredienti
    ingrediente_tipo* temp;
    while (var != NULL) { //finchè ci sono ingredienti, rimuovili
        temp = var;
        var = var->next_ingredient;
        //free(temp->nome_ingrediente);
        free(temp);
    }
}

//Funzioni di gestione Ingredienti
ingrediente_tipo* add_ingredients(ingrediente_tipo* var, char* token, magazzino_tipo* magazzino) { //funzione per aggiungere ingredienti alla ricetta
    ingrediente_tipo* head = var; //creo la base per la ricetta. Var se la ricetta è nuova sarà null, per come funziona la funzione aggiungi ricetta
    ingrediente_tipo* prec = NULL;
    while (token!=NULL) { //per come è dato l'input, alla fine della stringa finiscono gli ingredienti
        ingrediente_tipo* temp = (ingrediente_tipo*)malloc(sizeof(ingrediente_tipo)); //creo lo spazio per il nuovo ingrediente
        magazzino_tipo* pos_magazz = trova_nel_magazzino(&magazzino, token);
        temp->punt_magazz = pos_magazz;
        /*temp->nome_ingrediente = (char*)malloc((strlen(token) + 1) * sizeof(char)); //creo lo spazio per il nome. + IL TERMINATOREEEE
        strcpy(temp->nome_ingrediente, token);*/
        token = strtok(NULL, " ");
        temp->qta = atoi(token);
        token = strtok(NULL, " ");
        temp->next_ingredient = NULL; //inizializzo il puntatore del prossimo ingrediente
        temp->prev_ingredient = prec; //collega al nodo precedente
        if (prec != NULL) { //se non è il primo nodo, il punt al prissimo ingrediente è il nodo corrente
            prec->next_ingredient = temp;
        } else { //è il primo nodo della lista, quindi la testa si deve attaccare a lui
            head = temp;
        }
        prec = temp;
    }
    return head; //inserisco gli ingredienti in ordine inverso, quindi restituisco la testa
}

 
ingrediente_tipo* swap_ingredienti(ingrediente_tipo* testa, ingrediente_tipo* nodo_nuova_testa) { //controlli iniziali per casi particolari
    if (testa == NULL || nodo_nuova_testa == NULL || testa == nodo_nuova_testa) { //se la lista è vuota, il nodo da scambiare è già la testa, o il nodo da scambiare è NULL non faccio nulla e restituiamo la testa originale
        return testa;
    }
    ingrediente_tipo* vecchia_testa = testa;
    ingrediente_tipo* nuova_testa = nodo_nuova_testa;
    if (vecchia_testa->next_ingredient == nuova_testa) { //caso particolare: il nodo da scambiare è adiacente alla testa
        vecchia_testa->next_ingredient = nuova_testa->next_ingredient;  //aggiorna i puntatori della vecchia testa
        if (vecchia_testa->next_ingredient != NULL) {
            vecchia_testa->next_ingredient->prev_ingredient = vecchia_testa;
        }
        nuova_testa->prev_ingredient = vecchia_testa->prev_ingredient; //aggiorna i puntatori della nuova testa
        nuova_testa->next_ingredient = vecchia_testa;
        if (vecchia_testa->prev_ingredient != NULL) {  //aggiorna il puntatore del nodo precedente alla vecchia testa, se esiste
            vecchia_testa->prev_ingredient->next_ingredient = nuova_testa;
        }
        vecchia_testa->prev_ingredient = nuova_testa;  //aggiorna il puntatore prev della vecchia testa
    } else { //caso generale: i nodi non sono adiacenti
        ingrediente_tipo* nodo_prec = nuova_testa->prev_ingredient; //salva i nodi adiacenti al nodo da scambiare
        ingrediente_tipo* nodo_succ = nuova_testa->next_ingredient;
        if (nodo_prec != NULL) { //rimuove il nodo da scambiare dalla sua posizione originale
            nodo_prec->next_ingredient = nodo_succ;
        }
        if (nodo_succ != NULL) {
            nodo_succ->prev_ingredient = nodo_prec;
        }
        nuova_testa->next_ingredient = vecchia_testa->next_ingredient; //inserisce il nodo da scambiare nella posizione della vecchia testa
        nuova_testa->prev_ingredient = vecchia_testa->prev_ingredient;
        if (vecchia_testa->next_ingredient != NULL) {//ggiorna i puntatori dei nodi vicini alla vecchia testa
            vecchia_testa->next_ingredient->prev_ingredient = nuova_testa;
        }
        if (vecchia_testa->prev_ingredient != NULL) {
            vecchia_testa->prev_ingredient->next_ingredient = nuova_testa;
        }
        vecchia_testa->next_ingredient = nodo_succ; //inserisce la vecchia testa nella posizione originale del nodo scambiato
        if (nodo_succ != NULL) {
            nodo_succ->prev_ingredient = vecchia_testa;
        }
        vecchia_testa->prev_ingredient = nodo_prec;
        if (nodo_prec != NULL) {
            nodo_prec->next_ingredient = vecchia_testa;
        }
    }
    if (testa == vecchia_testa) {//se il nodo scambiato era effettivamente la testa, aggiorna il puntatore alla testa
        testa = nuova_testa;
    }
    return testa; //restituisce il nuovo puntatore alla testa della lista
}

//Gestione del Ricettario
void aggiungi_ricetta(ricettario_tipo* ricettario, char* token, magazzino_tipo* magazzino) {
    char *recipe_name = strtok(NULL, " ");
    int pos = hash_funct((char*)recipe_name) % numero_ricette_max;
    int cont = 0;
    while (cont < numero_ricette_max) {
        if (ricettario[pos].rec == NULL) { //trovato un buco, aggiungo la ricetta
            ricettario[pos].rec = (ricetta_tipo*)malloc(sizeof(ricetta_tipo));
            ricettario[pos].rec->nome_ricetta = (char*)malloc((strlen(recipe_name) + 1) * sizeof(char));
            strcpy(ricettario[pos].rec->nome_ricetta, recipe_name);
            ricettario[pos].rec->waiting = 0;
            ricettario[pos].rec->ingredienti_rec = NULL;
            token = strtok(NULL, " ");
            ricettario[pos].rec->ingredienti_rec = add_ingredients(ricettario[pos].rec->ingredienti_rec, token, magazzino);
            numero_ricette_presenti++;
            printf("aggiunta\n");
            return;
        } else if (strcmp(ricettario[pos].rec->nome_ricetta, recipe_name) == 0) { //la ricetta è già presente
            printf("ignorato\n");
            return;
        }
        pos = (pos + 1) % numero_ricette_max;
        cont++;
    }
}

void rimuovi_ricetta(ricettario_tipo* ricettario, char* recipe_name) {
    int posz = hash_funct((char*)recipe_name) % numero_ricette_max;
    for (int cont = 0; cont < numero_ricette_max; cont++) {
        if (ricettario[posz].rec == NULL) {
            if (ricettario[posz].tomb_recipe == 0) {
                printf("non presente\n");
                return; //la posizione è vuota e non è una tomba, quindi interrompo
            }
        } else if (strcmp(ricettario[posz].rec->nome_ricetta, recipe_name) == 0) {
            // Ricetta trovata
            if (ricettario[posz].rec->waiting == 0) { //non ho ordini in attesa
                //free_ingredients(ricettario[posz].rec->ingredienti_rec);
                free(ricettario[posz].rec->nome_ricetta);
                free(ricettario[posz].rec);
                ricettario[posz].rec = NULL;
                ricettario[posz].tomb_recipe = '1';
                numero_ricette_presenti--;
                printf("rimossa\n");
            } else { //con ordini in sospeso
                printf("ordini in sospeso\n");
            }
            return;
        }
        posz = (posz + 1) % numero_ricette_max;
    }
    printf("non presente\n"); //non trovata
}



void free_ricettario(ricettario_tipo* ricettario) { //per svuotare il ricettario alla fine del programma
    for (int i = 0; i < numero_ricette_max; i++) { //scorre per tutte le ricette
        if (ricettario[i].rec != NULL) {
            free_ingredients(ricettario[i].rec->ingredienti_rec);
            free(ricettario[i].rec->nome_ricetta);
            free(ricettario[i].rec);
            ricettario[i].rec = NULL;
        }
    }
    free(ricettario); //elimino il ricettario
}

/*void stampa_ricette(ricettario_tipo* ricettario, int numero_ricette_max) {
    for (int i = 0; i < numero_ricette_max; i++) {
        if (ricettario[i].rec != NULL && ricettario[i].tomb_recipe == '0') {
            printf("Ricetta: %s\n", ricettario[i].rec->nome_ricetta);
            ingrediente_tipo* ingrediente = ricettario[i].rec->ingredienti_rec;
            while (ingrediente != NULL) {
                printf("  Ingrediente: %s, Quantità: %d\n", ingrediente->nome_ingrediente, ingrediente->qta);
                ingrediente = ingrediente->next_ingredient;
            }
        }
    }
} */

ricettario_tipo* rehashing(ricettario_tipo* ricettario) {
    int old_max_recipes = numero_ricette_max; //salva la vecchia dimensione del ricettario
    int new_max_recipes = old_max_recipes * 2; //raddoppia il numero di ricette massimo
    //allocazione della memoria per il nuovo ricettario
    ricettario_tipo* new_ricettario = (ricettario_tipo*)malloc(sizeof(ricettario_tipo) * new_max_recipes);
    for (int i = 0; i < new_max_recipes; i++) {
        new_ricettario[i].rec = NULL;
        new_ricettario[i].tomb_recipe = '0';
    }
    for (int i = 0; i < old_max_recipes; i++) {
        if (ricettario[i].tomb_recipe == '0' && ricettario[i].rec != NULL) {
            long int pos = hash_funct((char*)ricettario[i].rec->nome_ricetta) % new_max_recipes;
            while (new_ricettario[pos].rec != NULL) {
                pos = (pos + 1) % new_max_recipes;
            }
            new_ricettario[pos].rec = ricettario[i].rec;
            new_ricettario[pos].tomb_recipe = '0';
        }
    }
    numero_ricette_max = new_max_recipes;
    free(ricettario);
    return new_ricettario;
}


//Gestione Ordini
//IN PRODUCI ORDINE DO PER ASSODATO CHE LA RICETTA SIA FATTIBILE
int produci_ordine2(int qta_ordine, ricettario_tipo* ricettario, magazzino_tipo* magazzino) {
    int peso_ordine = 0;
    ingrediente_tipo* ingrediente_ordine = ricettario->rec->ingredienti_rec;
    while (ingrediente_ordine != NULL) { //ciclo sugli ingredienti
        int qta_da_togliere = qta_ordine * ingrediente_ordine->qta; //quanto mi serve di quell'ingrediente
        nodo_lista_magazzino_tipo* temp_magazz = ingrediente_ordine->punt_magazz->node_magazzino;
        nodo_lista_magazzino_tipo* prev_magazz = NULL; //puntatore al nodo precedente
        while (qta_da_togliere > 0 && temp_magazz != NULL) {
            if (qta_da_togliere >= temp_magazz->qta) {  //se la quantità da togliere è maggiore o uguale a quella disponibile
                qta_da_togliere -= temp_magazz->qta;
                peso_ordine += temp_magazz->qta;
                ingrediente_ordine->punt_magazz->qta_tot -= temp_magazz->qta;
                if (prev_magazz == NULL) { //cancella il nodo dal magazzino
                    ingrediente_ordine->punt_magazz->node_magazzino = temp_magazz->next_node;
                } else {
                    prev_magazz->next_node = temp_magazz->next_node;
                }
                cancella_nodo_magazzino(&temp_magazz);
                if (prev_magazz == NULL) {
                    temp_magazz = ingrediente_ordine->punt_magazz->node_magazzino;
                } else {
                    temp_magazz = prev_magazz->next_node;
                }
            } else {
                temp_magazz->qta -= qta_da_togliere;
                peso_ordine += qta_da_togliere;
                ingrediente_ordine->punt_magazz->qta_tot -= qta_da_togliere;
                qta_da_togliere = 0;
            }
        }
        if (ingrediente_ordine->punt_magazz->node_magazzino == NULL) {
            ingrediente_ordine->punt_magazz->qta_tot=0;
        }
        ingrediente_ordine = ingrediente_ordine->next_ingredient;
    }
    return peso_ordine;
}


nodo_lista_magazzino_tipo* check_expired_batch(int time, nodo_lista_magazzino_tipo** nodo, magazzino_tipo* magazzino){
    nodo_lista_magazzino_tipo* temp = *nodo;
    nodo_lista_magazzino_tipo* prev = NULL;
    while (temp != NULL && temp->scad<=time) { //scorro sugli ingredienti
            if (prev == NULL) {
                magazzino->node_magazzino = temp->next_node;
            } else {
                prev->next_node = temp->next_node;
            }
            magazzino->qta_tot -= temp->qta;
            cancella_nodo_magazzino(&temp);
            if (prev == NULL) {
                temp = magazzino->node_magazzino;
            } else {
                temp = prev->next_node;
            }
    }
    return temp;
}

int fattibility_ricetta2(int qta_ordine, ricettario_tipo* ricettario, magazzino_tipo* magazzino, int time) {
    ingrediente_tipo* ingrediente_ordine = ricettario->rec->ingredienti_rec;
    while (ingrediente_ordine != NULL) {
        int qta_richiesta = qta_ordine * ingrediente_ordine->qta; //calcolo quantità richiesta per l'ordine
        nodo_lista_magazzino_tipo* nodo_magazzino = ingrediente_ordine->punt_magazz->node_magazzino;
        nodo_magazzino = check_expired_batch(time, &nodo_magazzino, ingrediente_ordine->punt_magazz);
        if (ingrediente_ordine->punt_magazz->qta_tot < qta_richiesta) {
            if (ricettario->rec->ingredienti_rec != ingrediente_ordine) {
                ricettario->rec->ingredienti_rec = swap_ingredienti(ricettario->rec->ingredienti_rec, ingrediente_ordine);
            }
            return 0;
        }
        ingrediente_ordine = ingrediente_ordine->next_ingredient;
    }
    return 1; //fattibile
}


void ordine(ricettario_tipo* ricettario, nodo_wait_list_tipo** wait_list, nodo_wait_list_tipo** coda_wait_list, wait_pickup_list_tipo** wait_pickup_list, magazzino_tipo* magazzino, int time, char* ordine_ricevuto, int qta_ordine) {
    int posz = hash_funct((char*)ordine_ricevuto) % numero_ricette_max;
    for (int cont = 0; cont < numero_ricette_max; cont++) {
        if (ricettario[posz].rec == NULL) {
            if (ricettario[posz].tomb_recipe == 0) {
                printf("rifiutato\n");
                return; //vuota e non è tomba
            }
        } else if (strcmp(ricettario[posz].rec->nome_ricetta, ordine_ricevuto) == 0) { //ricetta trovata
            printf("accettato\n");
            if (fattibility_ricetta2(qta_ordine, &ricettario[posz], magazzino, time)) { //fattibile
                int peso_ordine = produci_ordine2(qta_ordine, &ricettario[posz], magazzino);
                inserisci_in_wait_pickup_list(wait_pickup_list, peso_ordine, time, qta_ordine, &ricettario[posz]);
            } else { //non fattibile
                inserisci_in_wait_list(wait_list, coda_wait_list, time, qta_ordine, &ricettario[posz]);
            }
            ricettario[posz].rec->waiting++;
            return;
        }
        posz = (posz + 1) % numero_ricette_max;
    }
    printf("rifiutato\n"); //non trovata
}


//Gestione corriere
void pesi_uguali(corriere_ordine_tipo* curr_ref, corriere_ordine_tipo* temp) {
    corriere_ordine_tipo* curr = curr_ref;
    while (curr->next != NULL && curr->next->peso == temp->peso) {
        if (temp->istante_arrivo_ordine < curr->next->istante_arrivo_ordine) {
            temp->next = curr->next;
            curr->next = temp;
            return;
        }
        curr = curr->next;
    }
    temp->next = curr->next;  //se siamo alla fine della lista o abbiamo trovato un peso diverso
    curr->next = temp;
}

void aggiungi_lista_corriere(corriere_ordine_tipo** lista_corriere_ref, int istante_arrivo, char* ordine_ricevuto, int numero_elementi_ordinati, int peso) {
    corriere_ordine_tipo* temp = (corriere_ordine_tipo*)malloc(sizeof(corriere_ordine_tipo));
    temp->istante_arrivo_ordine = istante_arrivo;
    temp->numero_elementi_ordinati = numero_elementi_ordinati;
    temp->peso = peso;
    temp->nome_ricetta = (char*)malloc((strlen(ordine_ricevuto) + 1) * sizeof(char));
    strcpy(temp->nome_ricetta, ordine_ricevuto);
    temp->next = NULL;
    if (*lista_corriere_ref == NULL || peso > (*lista_corriere_ref)->peso) {
        temp->next = *lista_corriere_ref;
        *lista_corriere_ref = temp;
        return;
    }
    corriere_ordine_tipo* curr = *lista_corriere_ref;
    while (curr->next != NULL && curr->next->peso >= peso) {
        curr = curr->next;
    }
    if (curr->peso == peso) {
        pesi_uguali(curr, temp);
    } else {
        temp->next = curr->next;
        curr->next = temp;
    }
}

void load_camioncino(corriere_ordine_tipo* lista_corriere) {//carica il camioncino e stampa come richiesto da specifica
    corriere_ordine_tipo* curr = lista_corriere;
    while (curr != NULL) {
        printf("%d %s %d\n", curr->istante_arrivo_ordine, curr->nome_ricetta, curr->numero_elementi_ordinati);
        corriere_ordine_tipo* tmp = curr;
        curr = curr->next;
        free(tmp->nome_ricetta);
        free(tmp);
    }
}

void check_corriere(wait_pickup_list_tipo** wait_pickup_list, int capacity_corriere, ricettario_tipo* ricettario) {
    if (wait_pickup_qta == 0 || *wait_pickup_list == NULL) {
        printf("camioncino vuoto\n");
        return;
    }
    corriere_ordine_tipo* lista_corriere = NULL;
    wait_pickup_list_tipo* curr = *wait_pickup_list;
    wait_pickup_list_tipo* prec = NULL;
    int capacity_rimasta = capacity_corriere;
    while (curr != NULL && capacity_rimasta > 0) {
        if (capacity_rimasta >= curr->peso) {
            aggiungi_lista_corriere(&lista_corriere, curr->arrival_time, curr->punt_ricettario->rec->nome_ricetta, curr->numero_elementi_ordinati, curr->peso); // aggiungi l'ordine alla lista del corriere
            curr->punt_ricettario->rec->waiting--;//aggiorna la ricetta nel ricettario
            capacity_rimasta -= curr->peso;  //aggiorna la capacità rimanente
            wait_pickup_qta--; //segno che ho un elemento in meno in attesa
            wait_pickup_list_tipo* temp = curr;
            if (prec == NULL) {
                *wait_pickup_list = curr->next_node_wp; //nuova testa delkla lista
            } else {
                prec->next_node_wp = curr->next_node_wp;
            }
            curr = curr->next_node_wp;
            //free(temp->ordine);
            free(temp);
        } else {
            break;
        }
    }
    if (lista_corriere != NULL) {
        load_camioncino(lista_corriere);
    }
}

//MAIN
int main(void){
    int periodo_corriere, capacity_corriere; //so che l'input inizia con la configurazione del corriere. salvo in due interi il valore periodicità e capienza
    int time;
    time = scanf ("%d %d%*c", &periodo_corriere, &capacity_corriere); //leggo i primi due dati
    time = 0; //azzero l'orologio
    size_t size = 0; // variabile per memorizzare la dimensione allocata
    ssize_t len; //draft c11, tipo ssize e size sono i più adatti
    ricettario_tipo* ricettario = (ricettario_tipo*)malloc(sizeof(ricettario_tipo)*numero_ricette_max);//setup ricettario, il mio ricettario ora è un puntatore a questa mega struttura dati fatta di tante ricette
    for (int i = 0; i<numero_ricette_max;i++){//inizializzo a '0' il campo tomb e a null il campo pointer
        ricettario[i].rec = NULL;
        ricettario[i].tomb_recipe = '0';
    }
    magazzino_tipo* magazzino = (magazzino_tipo*)malloc(sizeof(magazzino_tipo)*storage_magazzino_max);
    for(int i= 0; i<storage_magazzino_max; i++) { //inizializzo
        magazzino[i].nome_ingrediente = NULL;
        magazzino[i].node_magazzino = NULL;
        magazzino[i].tomb_magazzino='0';
        magazzino[i].qta_tot = 0;
    }
    nodo_wait_list_tipo* wait_list =(nodo_wait_list_tipo*)malloc(sizeof(nodo_wait_list_tipo));//alloco wait list
    wait_list = NULL; //inizializzo wait list
    nodo_wait_list_tipo* coda_wait_list=(nodo_wait_list_tipo*)malloc(sizeof(nodo_wait_list_tipo));//alloco coda wait list
    coda_wait_list = NULL; //inizializzo coda wait list
    wait_pickup_list_tipo* wait_pickup_list=(wait_pickup_list_tipo*)malloc(sizeof(wait_pickup_list_tipo));//alloco wait pickup list
    wait_pickup_list = NULL; //inizializzo
    while (1) { //esegui fino a che, il gestore dell'input, non riconosce che l'input è finito
        if(time%periodo_corriere==0){ //controllo il camioncino
            if(time!=0){
                check_corriere(&wait_pickup_list,capacity_corriere, ricettario);
            }
        }
        char *input = NULL;  //puntatore per la stringa di input
        len = getline(&input, &size, stdin);  //lettura stringa di input per righe di input
        if (len == -1) { //se getline ritorna -1, significa che c'è stato un errore o è stata raggiunta la fine del file. termino il programma
            free(input); //libera la memoria allocata
            break;
        }
        if (len > 0 && input[len - 1] == '\n')
                input[len - 1] = '\0';//rimuove il carattere \n alla fine, se presente, per non averlo nei token
        char *token = strtok(input, " ");  //inizializzo la variabile token con la funzione strtok per ottenere il primo token. separo un token ad ogni spazio
                //strcmp restituisce un intero = 0 se le stringhe sono uguali 1 in caso contrario
        if ((strcmp(token, "aggiungi_ricetta")==0)) {
            aggiungi_ricetta(ricettario,token, magazzino);
            if((double)(numero_ricette_presenti/numero_ricette_max)>load_factor_limite){//verifico eventuale rehashing
                ricettario = rehashing(ricettario); //ho ideato la funzione perchè restituisca il puntatore del nuovo ricettario
            }
        } else if (strcmp(token, "rimuovi_ricetta")==0){
            char *recipe_name = strtok(NULL, " ");
            rimuovi_ricetta(ricettario, recipe_name);
        } else if (strcmp(token, "rifornimento")==0){
            token = strtok(NULL, " ");
            rifornimento_magazzino(&magazzino, &wait_list, &coda_wait_list,&wait_pickup_list, ricettario, time ,token);
            printf("rifornito\n");
        } else if (strcmp(token, "ordine")==0){
            char* ordine_ricevuto = strtok(NULL, " ");
            token = strtok(NULL, " ");
            int qta_ordine = atoi(token);
            ordine(ricettario, &wait_list, &coda_wait_list, &wait_pickup_list, magazzino, time, ordine_ricevuto, qta_ordine);
        }
        free(input);
        time++;
        /*stampa_magazzino_lista(magazzino);
        stampa_ricette(ricettario, numero_ricette_max);
        stampa_wait_list(wait_list);
        stampa_wait_pickup_list(wait_pickup_list);*/
    }
    if(time%periodo_corriere==0){ //ultimo check del corriere prima della fine del codice, se è il tempo corretto
        check_corriere(&wait_pickup_list,capacity_corriere, ricettario);
    }
    //free per valgrind
    /*free_ricettario(ricettario);
    freeMagazzino_full(&magazzino);
    freeWaitList(&wait_list);
    freeWaitPickUpList(&wait_pickup_list);*/
    return 0;
}
