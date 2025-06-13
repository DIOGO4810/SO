# Trabalho Prático de Sistemas Operativos

## Grupo

- **Diogo Ribeiro** — A106906  
- **Carolina Martins** — A107285  
- **Filipa Gonçalves** — A107329  

## 📊 Final Grade

19.5 Valores

---

## 📌 Resumo

Este projeto tem como objetivo implementar um serviço para **indexação e pesquisa de documentos de texto** armazenados localmente utilizando apenas System calls. O sistema é composto por dois componentes principais:

- **Servidor**: responsável por manter um registo com meta-informações dos documentos (como título, autor, ano e caminho).
- **Cliente**: interage com o servidor para adicionar, remover, pesquisar documentos ou obter detalhes sobre documentos indexados. Apenas uma operação pode ser executada por vez.

---

## ⚙️ Funcionalidades

- Indexação de novos documentos com título, autor, ano e caminho.
- Remoção de documentos indexados.
- Consulta de documentos por critérios definidos.
- Obtenção de informação detalhada de documentos específicos.
- Suporte para múltiplos clientes, sem bloquear o servidor.
- Cache para acesso mais rápido aos documentos frequentemente consultados.
- Persistência dos documentos mesmo após reinício do servidor.

---

## 🛠️ Tecnologias e Conceitos

- Linguagem: **C**
- Comunicação entre processos: **FIFO (named pipes)**
- Processos: **`fork()` e `exec()`**
- Redirecionamento de I/O: **`dup`**
- Manipulação de ficheiros: **`open()`, `read()`, `write()`, `close()`**
- Estruturas de dados auxiliares: **GLib (GArray, Hash Tables, etc.)**
- Sistema de cache implementado com política **LRU (Least Recently Used)**

---

## 🔒 Restrições

- Apenas **System Calls** do sistema operativo são utilizadas para toda a comunicação e manipulação de ficheiros.
- O servidor deve continuar funcional mesmo com múltiplos pedidos concorrentes.
- O sistema não deve perder dados ao ser reiniciado.

---


