#ifndef INDEX_H
#define INDEX_H

#include <glib.h>

/**
 * @brief Tipo opaco para representar um índice.
 */
typedef struct index Index; 

/**
 * @brief Obtém o tamanho da estrutura Index.
 * 
 * @return Tamanho em bytes da estrutura.
 */
int getStructSize();

/**
 * @brief Obtém um ponteiro para uma estrutura Index marcada como apagada (ou reutilizável).
 * 
 * @return Ponteiro para a estrutura Index.
 */
Index* getDeletedIndex();

/**
 * @brief Liberta a memória associada a uma GArray de estruturas Index.
 * 
 * @param estrutura Ponteiro para a GArray a ser libertada.
 */
void freeEstrutura(GArray *estrutura);

/**
 * @brief Imprime os dados de um índice.
 * 
 * @param indice Ponteiro para a estrutura Index a ser impressa.
 */
void printIndice(Index *indice);


/// @name Setters
/// Funções para definir campos da estrutura Index.
/// @{

/**
 * @brief Define o título no índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param title String com o título.
 */
void setTitle(Index *idx, char *title);

/**
 * @brief Define o autor no índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param author String com o autor.
 */
void setAuthor(Index *idx, char *author);

/**
 * @brief Define o ano no índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param year Valor do ano.
 */
void setYear(Index *idx, int year);

/**
 * @brief Define o caminho do ficheiro no índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param path String com o caminho.
 */
void setPath(Index *idx, char *path);

/**
 * @brief Define o PID do processo zombie.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param pidZombie Valor do PID.
 */
void setPidZombie(Index *idx, int pidZombie);

/**
 * @brief Define a ordem do índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param order Valor da ordem.
 */
void setOrder(Index *idx, int order);

/**
 * @brief Define o tipo de mensagem.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param messageType Carácter representando o tipo.
 */
void setMessageType(Index *idx, char messageType);

/**
 * @brief Define a chave do índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param key Valor da chave.
 */
void setKey(Index *idx, int key);

/**
 * @brief Define a palavra-chave.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param keyWord String com a palavra-chave.
 */
void setKeyWord(Index *idx, char *keyWord);

/**
 * @brief Define o PID do cliente.
 * 
 * @param index Ponteiro para a estrutura Index.
 * @param pidCliente Valor do PID do cliente.
 */
void setPidCliente(Index *index, int pidCliente);

/**
 * @brief Define o número de processos associados.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @param numProcessos Número de processos.
 */
void setNumProcessos(Index *idx, int numProcessos);

/// @}


/// @name Getters
/// Funções para aceder aos campos da estrutura Index.
/// @{

/**
 * @brief Obtém o título.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return String com o título.
 */
char* getTitle(Index *idx);

/**
 * @brief Obtém o PID do cliente.
 * 
 * @param index Ponteiro para a estrutura Index.
 * @return Valor do PID do cliente.
 */
int getPidCliente(Index* index);

/**
 * @brief Obtém o autor.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return String com o autor.
 */
char* getAuthor(Index *idx);

/**
 * @brief Obtém o ano.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return Valor do ano.
 */
int getYear(Index *idx);

/**
 * @brief Obtém o caminho do ficheiro.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return String com o caminho.
 */
char* getPath(Index *idx);

/**
 * @brief Obtém o PID do processo zombie.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return Valor do PID.
 */
int getPidZombie(Index *idx);

/**
 * @brief Obtém a ordem do índice.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return Valor da ordem.
 */
int getOrder(Index *idx);

/**
 * @brief Obtém o tipo de mensagem.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return Carácter representando o tipo.
 */
char getMessageType(Index *idx);

/**
 * @brief Obtém a chave.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return Valor da chave.
 */
int getKey(Index *idx);

/**
 * @brief Obtém a palavra-chave.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return String com a palavra-chave.
 */
char* getKeyWord(Index *idx);

/**
 * @brief Obtém o número de processos associados.
 * 
 * @param idx Ponteiro para a estrutura Index.
 * @return Número de processos.
 */
int getNumProcessos(Index *idx);

/// @}

#endif
