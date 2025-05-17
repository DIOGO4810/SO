#ifndef PARSER_H
#define PARSER_H

/**
 * @brief Tipo opaco que representa um parser de linhas em tokens.
 */
typedef struct parser Parser;

/**
 * @brief Cria uma nova estrutura de parser.
 * 
 * @param numTokensMax Número máximo de tokens que o parser poderá armazenar.
 * @return Ponteiro para a nova estrutura Parser.
 */
Parser *newParser(int numTokensMax);

/**
 * @brief Processa uma linha de texto, dividindo-a em tokens com base num delimitador.
 * 
 * @param parserE Ponteiro para a estrutura Parser.
 * @param line Linha de texto a ser dividida.
 * @param limitador Carácter delimitador (ex: espaço, vírgula, etc.).
 * @return Ponteiro atualizado para a estrutura Parser com os novos tokens.
 */
Parser *parser(Parser *parserE, char* line, char limitador);

/**
 * @brief Liberta a memória associada ao parser.
 * 
 * @param parserE Ponteiro para a estrutura Parser.
 */
void freeParser(Parser *parserE);

/**
 * @brief Obtém o array de tokens gerado pelo parser.
 * 
 * @param parserE Ponteiro para a estrutura Parser.
 * @return Array de strings com os tokens (char**).
 */
char** getTokens(Parser *parserE);

/**
 * @brief Imprime os tokens para o output standard.
 * 
 * @param tokens Array de strings contendo os tokens a imprimir.
 */
void printTokens(char **tokens);

/**
 * @brief Obtém o número de tokens atualmente armazenados no parser.
 * 
 * @param parserE Ponteiro para a estrutura Parser.
 * @return Número de tokens.
 */
int getNumTokens(Parser *parserE);

#endif
