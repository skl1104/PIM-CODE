#ifndef SERVICOS_H
#define SERVICOS_H

#include <stdio.h> 

// --- Constantes Globais ---
#define MAX_ALUNOS 100
#define MAX_TURMAS 20
#define TAM_NOME 50
#define TAM_RA 10
#define NOME_ARQUIVO "dados_sistema.bin"

// --- Constantes e Structs de Autenticação ---
#define NIVEL_ALUNO 0
#define NIVEL_PROFESSOR 1
#define NIVEL_ADMIN 2
#define TAM_SENHA 15

typedef struct {
    char login[TAM_RA];
    char senha[TAM_SENHA];
    int nivel_acesso; // 0=Aluno, 1=Professor, 2=Admin
} Usuario;

// --- Estruturas de Dados (Sincronizadas) ---
typedef struct {
    int id;
    char nome[TAM_NOME];
    int vagas_maximas;
    int vagas_ocupadas;
    int ativo;
} Turma;

typedef struct {
    char ra[TAM_RA];
    char nome[TAM_NOME];
    int id_turma;
    float notas[3]; 
    float media_final; 
    int ativo; 
} Aluno;

typedef struct {
    Turma turmas[MAX_TURMAS];
    Aluno alunos[MAX_ALUNOS];
    int total_turmas;
    int total_alunos;
} DadosSistema;

// --- Protótipos das Funções ---

// Autenticação
int realizar_login(int *nivel_acesso);

// I/O (Persistência)
void carregar_dados(DadosSistema *sistema);
void salvar_dados(const DadosSistema *sistema);

// Auxiliares (Relatório)
void listar_todas_turmas(const DadosSistema *sistema);

// Gerenciamento (CREATE)
int adicionar_turma(DadosSistema *sistema, const char *nome, int vagas);
int adicionar_aluno(DadosSistema *sistema, const char *nome, const char *ra, int id_turma);

// Gerenciamento (UPDATE - Função mudou para aceitar nivel_acesso)
int lancar_notas_e_atualizar_media(DadosSistema *sistema, const char *ra, float n1, float n2, float n3, int nivel_acesso);
int editar_dados_aluno(DadosSistema *sistema, const char *ra_antigo, const char *nome_novo, int id_turma_nova);

// Gerenciamento (DELETE - Exclusão Lógica)
int excluir_aluno_por_ra(DadosSistema *sistema, const char *ra);
int excluir_turma_por_id(DadosSistema *sistema, int id);

// Lógica e Relatórios (READ)
void ordenar_alunos_por_nome(DadosSistema *sistema);
void gerar_relatorio_turma(const DadosSistema *sistema, int id_turma);


#endif // SERVICOS_H