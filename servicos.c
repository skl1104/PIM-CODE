#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "servicos.h"

// Protótipo da função auxiliar de ordenação (necessária para qsort ou bubble sort)
void trocar_alunos(Aluno *a, Aluno *b); 
int buscar_aluno_por_ra(const DadosSistema *sistema, const char *ra);
int buscar_turma_por_id(const DadosSistema *sistema, int id_turma);
void calcular_media(Aluno *aluno);

// --- 1. Autenticação ---

/**
 * @brief Tenta autenticar o usuário no sistema.
 * * Verifica se o login e senha correspondem a um usuário fixo predefinido e, 
 * se sim, define o nível de acesso do usuário.
 * * @param nivel_acesso Ponteiro para armazenar o nível de acesso (0=Aluno, 1=Prof, 2=Admin).
 * @return int 1 se o login for bem-sucedido, 0 caso contrário.
 */
int realizar_login(int *nivel_acesso) {
    // Usuários fixos do sistema (você pode customizar isso)
    Usuario usuarios_fixos[] = {
        {"admin", "master", NIVEL_ADMIN},
        {"222", "senha222", NIVEL_PROFESSOR},
        {"111", "senha111", NIVEL_ALUNO}
    };
    int total_usuarios = sizeof(usuarios_fixos) / sizeof(Usuario);

    char login[TAM_RA];
    char senha[TAM_SENHA];

    printf("\n--- LOGIN ---\n");
    printf("Login: ");
    fgets(login, TAM_RA, stdin);
    login[strcspn(login, "\n")] = 0;

    printf("Senha: ");
    fgets(senha, TAM_SENHA, stdin);
    senha[strcspn(senha, "\n")] = 0;

    // Busca o usuário na lista
    for (int i = 0; i < total_usuarios; i++) {
        if (strcmp(login, usuarios_fixos[i].login) == 0 && 
            strcmp(senha, usuarios_fixos[i].senha) == 0) {
            
            *nivel_acesso = usuarios_fixos[i].nivel_acesso;
            printf("\nLogin SUCESSO! Nivel de Acesso: %d.\n", *nivel_acesso);
            return 1; // Sucesso
        }
    }

    *nivel_acesso = -1; // Falha
    return 0; 
}


// --- 2. Persistência de Dados (I/O) ---

/**
 * @brief Carrega a estrutura de dados (alunos, turmas) de um arquivo binário.
 * Se o arquivo não existir ou for inválido, inicializa a estrutura do sistema.
 * @param sistema Ponteiro para a estrutura DadosSistema a ser carregada.
 */
void carregar_dados(DadosSistema *sistema) {
    FILE *f = fopen(NOME_ARQUIVO, "rb");

    if (f == NULL || fread(sistema, sizeof(DadosSistema), 1, f) != 1) {
        printf("AVISO: Arquivo de dados nao encontrado ou invalido. Inicializando o sistema...\n");
        // Inicializa o sistema se a leitura falhar
        sistema->total_turmas = 0;
        sistema->total_alunos = 0;
        // Marca todos como inativos (limpeza de memória)
        for (int i = 0; i < MAX_TURMAS; i++) sistema->turmas[i].ativo = 0;
        for (int i = 0; i < MAX_ALUNOS; i++) sistema->alunos[i].ativo = 0;
    } else {
        printf("SUCESSO: Dados carregados do arquivo '%s'.\n", NOME_ARQUIVO);
        fclose(f);
    }
}

/**
 * @brief Salva a estrutura de dados (alunos, turmas) em um arquivo binário.
 * @param sistema Ponteiro para a estrutura DadosSistema a ser salva.
 */
void salvar_dados(const DadosSistema *sistema) {
    FILE *f = fopen(NOME_ARQUIVO, "wb");
    if (f == NULL) {
        printf("ERRO: Nao foi possivel abrir o arquivo para salvar os dados.\n");
        return;
    }

    if (fwrite(sistema, sizeof(DadosSistema), 1, f) != 1) {
        printf("ERRO: Falha ao escrever os dados no arquivo.\n");
    } else {
        // printf("SUCESSO: Dados salvos em '%s'.\n", NOME_ARQUIVO); // Comentado para evitar poluição no console
    }

    fclose(f);
}


// --- 3. Auxiliares e Busca ---

/**
 * @brief Busca o índice de um aluno ativo pelo RA.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param ra O Registro Acadêmico a ser buscado.
 * @return int O índice do aluno no array, ou -1 se não for encontrado ou estiver inativo.
 */
int buscar_aluno_por_ra(const DadosSistema *sistema, const char *ra) {
    for (int i = 0; i < MAX_ALUNOS; i++) {
        // Busca apenas alunos ATIVOS
        if (sistema->alunos[i].ativo == 1 && strcmp(sistema->alunos[i].ra, ra) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Busca o índice de uma turma ativa pelo ID.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param id_turma O ID da turma a ser buscado.
 * @return int O índice da turma no array, ou -1 se não for encontrada ou estiver inativa.
 */
int buscar_turma_por_id(const DadosSistema *sistema, int id_turma) {
    for (int i = 0; i < MAX_TURMAS; i++) {
        // Busca apenas turmas ATIVAS e com ID correspondente
        if (sistema->turmas[i].ativo == 1 && sistema->turmas[i].id == id_turma) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Exibe uma lista de todas as turmas ativas no sistema.
 * Ajuda o usuário a escolher um ID.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 */
void listar_todas_turmas(const DadosSistema *sistema) {
    printf("\n--- Turmas Ativas ---\n");
    int encontrou = 0;
    for (int i = 0; i < MAX_TURMAS; i++) {
        if (sistema->turmas[i].ativo == 1) {
            printf("ID: %d | Nome: %s | Vagas: %d/%d\n", 
                   sistema->turmas[i].id, 
                   sistema->turmas[i].nome, 
                   sistema->turmas[i].vagas_ocupadas, 
                   sistema->turmas[i].vagas_maximas);
            encontrou = 1;
        }
    }
    if (!encontrou) {
        printf("Nenhuma turma ativa cadastrada.\n");
    }
    printf("---------------------\n");
}

/**
 * @brief Calcula a média aritmética das 3 notas de um aluno.
 * @param aluno Ponteiro para a estrutura Aluno.
 */
void calcular_media(Aluno *aluno) {
    aluno->media_final = (aluno->notas[0] + aluno->notas[1] + aluno->notas[2]) / 3.0f;
}

// --- 4. Gerenciamento (CREATE) ---

/**
 * @brief Adiciona uma nova turma ao sistema.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param nome Nome da nova turma.
 * @param vagas Número máximo de vagas.
 * @return int 1 se adicionada com sucesso, 0 caso contrário.
 */
int adicionar_turma(DadosSistema *sistema, const char *nome, int vagas) {
    if (sistema->total_turmas >= MAX_TURMAS) {
        // Limite máximo de turmas atingido
        return 0;
    }
    if (vagas <= 0) {
        printf("ERRO: O numero de vagas deve ser positivo.\n");
        return 0;
    }

    // Procura a primeira posição inativa (liberada)
    for (int i = 0; i < MAX_TURMAS; i++) {
        if (sistema->turmas[i].ativo == 0) {
            // Inicializa a nova turma
            sistema->turmas[i].id = i + 1; // ID baseado no índice + 1 (para IDs > 0)
            strncpy(sistema->turmas[i].nome, nome, TAM_NOME);
            sistema->turmas[i].vagas_maximas = vagas;
            sistema->turmas[i].vagas_ocupadas = 0;
            sistema->turmas[i].ativo = 1;

            sistema->total_turmas++;
            return 1;
        }
    }
    return 0; // Se não encontrou espaço ativo (embora o check inicial deve cobrir isso)
}

/**
 * @brief Adiciona um novo aluno a uma turma específica.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param nome Nome do aluno.
 * @param ra Registro Acadêmico do aluno.
 * @param id_turma ID da turma a ser matriculado.
 * @return int 1 se adicionado com sucesso, 0 caso contrário.
 */
int adicionar_aluno(DadosSistema *sistema, const char *nome, const char *ra, int id_turma) {
    if (sistema->total_alunos >= MAX_ALUNOS) {
        printf("ERRO: Limite maximo de alunos atingido.\n");
        return 0;
    }
    if (buscar_aluno_por_ra(sistema, ra) != -1) {
        printf("ERRO: RA '%s' ja cadastrado.\n", ra);
        return 0;
    }

    int idx_turma = buscar_turma_por_id(sistema, id_turma);
    if (idx_turma == -1) {
        printf("ERRO: Turma ID %d nao encontrada ou inativa.\n", id_turma);
        return 0;
    }
    if (sistema->turmas[idx_turma].vagas_ocupadas >= sistema->turmas[idx_turma].vagas_maximas) {
        printf("ERRO: Turma '%s' esta cheia.\n", sistema->turmas[idx_turma].nome);
        return 0;
    }

    // Procura a primeira posição inativa (liberada)
    for (int i = 0; i < MAX_ALUNOS; i++) {
        if (sistema->alunos[i].ativo == 0) {
            // Inicializa o novo aluno
            strncpy(sistema->alunos[i].ra, ra, TAM_RA);
            strncpy(sistema->alunos[i].nome, nome, TAM_NOME);
            sistema->alunos[i].id_turma = id_turma;
            sistema->alunos[i].notas[0] = 0.0f;
            sistema->alunos[i].notas[1] = 0.0f;
            sistema->alunos[i].notas[2] = 0.0f;
            sistema->alunos[i].media_final = 0.0f;
            sistema->alunos[i].ativo = 1;

            // Atualiza contadores
            sistema->total_alunos++;
            sistema->turmas[idx_turma].vagas_ocupadas++;
            return 1;
        }
    }
    return 0; // Falha (não deve acontecer se o total_alunos for verificado)
}


// --- 5. Gerenciamento (UPDATE) ---

/**
 * @brief Lança as notas N1, N2, N3 e atualiza a média de um aluno.
 * Verifica o nível de acesso (deve ser PROFESSOR ou ADMIN).
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param ra RA do aluno.
 * @param n1 Nota 1.
 * @param n2 Nota 2.
 * @param n3 Nota 3.
 * @param nivel_acesso Nível de acesso do usuário logado.
 * @return int 1 se as notas foram lançadas e a média atualizada, 0 caso contrário.
 */
int lancar_notas_e_atualizar_media(DadosSistema *sistema, const char *ra, float n1, float n2, float n3, int nivel_acesso) {
    if (nivel_acesso < NIVEL_PROFESSOR) {
        printf("ACESSO NEGADO: Apenas Professor ou Admin podem lancar notas.\n");
        return 0;
    }
    
    int idx_aluno = buscar_aluno_por_ra(sistema, ra);
    if (idx_aluno == -1) {
        printf("ERRO: Aluno com RA '%s' nao encontrado ou inativo.\n", ra);
        return 0;
    }
    
    // Atualiza as notas e calcula a média
    sistema->alunos[idx_aluno].notas[0] = n1;
    sistema->alunos[idx_aluno].notas[1] = n2;
    sistema->alunos[idx_aluno].notas[2] = n3;
    calcular_media(&sistema->alunos[idx_aluno]);
    
    printf("SUCESSO: Notas de '%s' lancadas (Media: %.2f).\n", 
           sistema->alunos[idx_aluno].nome, 
           sistema->alunos[idx_aluno].media_final);
    return 1;
}

/**
 * @brief Edita o nome e/ou a turma de um aluno.
 * Permite a alteração do nome e/ou a transferência de turma de um aluno ativo.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param ra_antigo RA do aluno a ser editado.
 * @param nome_novo Novo nome (se vazio, mantém o atual).
 * @param id_turma_nova Novo ID da turma (se 0, mantém a atual).
 * @return int 1 se a edição foi bem-sucedida, 0 caso contrário.
 */
int editar_dados_aluno(DadosSistema *sistema, const char *ra_antigo, const char *nome_novo, int id_turma_nova) {
    int idx_aluno = buscar_aluno_por_ra(sistema, ra_antigo);
    if (idx_aluno == -1) {
        printf("ERRO: Aluno com RA '%s' nao encontrado ou inativo.\n", ra_antigo);
        return 0;
    }
    Aluno *aluno = &sistema->alunos[idx_aluno];
    
    printf("Editando Aluno: %s (RA: %s)\n", aluno->nome, aluno->ra);
    int alterado = 0;

    // 1. Atualizar Nome
    if (strlen(nome_novo) > 0) {
        strncpy(aluno->nome, nome_novo, TAM_NOME);
        printf("Nome atualizado para: %s\n", nome_novo);
        alterado = 1;
    }

    // 2. Atualizar Turma (Transferência)
    if (id_turma_nova != 0 && id_turma_nova != aluno->id_turma) {
        int idx_turma_nova = buscar_turma_por_id(sistema, id_turma_nova);
        
        if (idx_turma_nova != -1) {
            // Verifica vagas na nova turma
            if (sistema->turmas[idx_turma_nova].vagas_ocupadas < sistema->turmas[idx_turma_nova].vagas_maximas) {
                
                // Libera vaga na turma antiga
                int idx_turma_antiga = buscar_turma_por_id(sistema, aluno->id_turma);
                if (idx_turma_antiga != -1) {
                    sistema->turmas[idx_turma_antiga].vagas_ocupadas--;
                }
                
                // Ocupa vaga na nova turma e atualiza o aluno
                sistema->turmas[idx_turma_nova].vagas_ocupadas++;
                aluno->id_turma = id_turma_nova;
                printf("Turma atualizada para ID: %d (%s)\n", id_turma_nova, sistema->turmas[idx_turma_nova].nome);
                alterado = 1;
            } else {
                printf("ERRO: Nova turma ID %d esta cheia. Turma nao alterada.\n", id_turma_nova);
                return 0; // Falha na edição
            }
        } else {
            printf("AVISO: ID de turma nova %d e invalido. Turma nao alterada.\n", id_turma_nova);
        }
    }
    
    if (!alterado) {
        printf("AVISO: Nenhum dado alterado.\n");
        return 0;
    }
    
    printf("SUCESSO: Edicao de dados concluida.\n");
    return 1;
}

// --- 6. Gerenciamento (DELETE - Exclusão Lógica) ---

/**
 * @brief Realiza a exclusão lógica de um aluno pelo RA.
 * A vaga na turma é liberada, e o aluno é marcado como inativo.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param ra RA do aluno a ser inativado.
 * @return int 1 se o aluno foi excluído logicamente, 0 caso contrário.
 */
int excluir_aluno_por_ra(DadosSistema *sistema, const char *ra) {
    int idx_aluno = buscar_aluno_por_ra(sistema, ra);
    if (idx_aluno == -1) {
        printf("ERRO: Aluno com RA '%s' nao encontrado ou ja inativo.\n", ra);
        return 0;
    }

    // Libera a vaga na turma
    int idx_turma = buscar_turma_por_id(sistema, sistema->alunos[idx_aluno].id_turma);
    if (idx_turma != -1) {
        sistema->turmas[idx_turma].vagas_ocupadas--;
    }

    // Exclusão Lógica
    sistema->alunos[idx_aluno].ativo = 0;
    sistema->total_alunos--;

    printf("SUCESSO: Aluno '%s' (RA: %s) excluido (logicamente) do sistema.\n", 
           sistema->alunos[idx_aluno].nome, ra);
    return 1;
}

/**
 * @brief Realiza a exclusão lógica de uma turma e de todos os seus alunos (Cascata).
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param id ID da turma a ser inativada.
 * @return int 1 se a turma foi excluída logicamente, 0 caso contrário.
 */
int excluir_turma_por_id(DadosSistema *sistema, int id) {
    int idx_turma = buscar_turma_por_id(sistema, id);
    if (idx_turma == -1) {
        printf("ERRO: Turma ID %d nao encontrada ou ja inativa.\n", id);
        return 0;
    }
    
    int alunos_excluidos = 0;
    // Exclusão em Cascata (inativa todos os alunos vinculados à turma)
    for (int i = 0; i < MAX_ALUNOS; i++) {
        // Verifica se o aluno está ativo E pertence a esta turma
        if (sistema->alunos[i].ativo == 1 && sistema->alunos[i].id_turma == id) {
            sistema->alunos[i].ativo = 0; // Inativa o aluno
            sistema->total_alunos--;      // Reduz o contador global
            alunos_excluidos++;
        }
    }
    
    // Exclusão Lógica da Turma
    sistema->turmas[idx_turma].ativo = 0;
    sistema->turmas[idx_turma].vagas_ocupadas = 0; // Zera as vagas ocupadas, pois todos os alunos foram inativados
    sistema->total_turmas--;

    printf("SUCESSO: Turma '%s' (ID %d) excluida (logicamente).\n", 
           sistema->turmas[idx_turma].nome, id);
    printf("AVISO: %d alunos vinculados tambem foram inativados (cascata).\n", alunos_excluidos);
    return 1;
}

// --- 7. Lógica e Relatórios (READ) ---

/**
 * @brief Função auxiliar de troca para o algoritmo de ordenação (ex: Bubble Sort).
 * @param a Ponteiro para o primeiro aluno.
 * @param b Ponteiro para o segundo aluno.
 */
void trocar_alunos(Aluno *a, Aluno *b) {
    Aluno temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * @brief Ordena a lista de alunos ativos no sistema por nome (Bubble Sort simples).
 * Nota: Uma implementação mais eficiente como QuickSort seria recomendada em um sistema real.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 */
void ordenar_alunos_por_nome(DadosSistema *sistema) {
    int i, j;
    
    // Implementação Bubble Sort simplificada
    for (i = 0; i < MAX_ALUNOS - 1; i++) {
        if (sistema->alunos[i].ativo == 0) continue; // Ignora inativos
        
        for (j = i + 1; j < MAX_ALUNOS; j++) {
             if (sistema->alunos[j].ativo == 0) continue; // Ignora inativos
            
            // Compara os nomes. Se o aluno[i] for maior que aluno[j], troca
            if (strcmp(sistema->alunos[i].nome, sistema->alunos[j].nome) > 0) {
                trocar_alunos(&sistema->alunos[i], &sistema->alunos[j]);
            }
        }
    }
    // A mensagem de sucesso é dada no main.c
}

/**
 * @brief Gera e exibe o relatório de todos os alunos ativos em uma turma.
 * @param sistema Ponteiro para a estrutura DadosSistema.
 * @param id_turma ID da turma para a qual o relatório será gerado.
 */
void gerar_relatorio_turma(const DadosSistema *sistema, int id_turma) {
    int idx_turma = buscar_turma_por_id(sistema, id_turma);
    if (idx_turma == -1) {
        printf("ERRO: Turma ID %d nao encontrada ou inativa.\n", id_turma);
        return;
    }
    
    const Turma *turma = &sistema->turmas[idx_turma];
    printf("\n--- RELATORIO: Turma %s (ID %d) ---\n", turma->nome, turma->id);
    printf("Total de Vagas: %d | Ocupadas: %d\n", turma->vagas_maximas, turma->vagas_ocupadas);
    printf("------------------------------------------------------------------------------------------------\n");
    printf("| %-10s | %-40s | %5s | %5s | %5s | %5s | %-8s |\n", 
           "RA", "Nome", "N1", "N2", "N3", "Media", "Situacao");
    printf("------------------------------------------------------------------------------------------------\n");

    int alunos_na_turma = 0;
    for (int i = 0; i < MAX_ALUNOS; i++) {
        // Verifica se o aluno está ativo E pertence a esta turma
        if (sistema->alunos[i].ativo == 1 && sistema->alunos[i].id_turma == id_turma) {
            const Aluno *aluno = &sistema->alunos[i];
            
            // Determina a Situação
            const char *situacao;
            if (aluno->media_final >= 7.0f) {
                situacao = "Aprovado";
            } else if (aluno->media_final >= 5.0f) {
                situacao = "Recup.";
            } else {
                situacao = "Reprovado";
            }

            printf("| %-10s | %-40s | %5.2f | %5.2f | %5.2f | %5.2f | %-8s |\n", 
                   aluno->ra, 
                   aluno->nome, 
                   aluno->notas[0], 
                   aluno->notas[1], 
                   aluno->notas[2], 
                   aluno->media_final,
                   situacao);
            alunos_na_turma++;
        }
    }

    if (alunos_na_turma == 0) {
        printf("|                                        Nenhum aluno ativo nesta turma.                                        |\n");
    }
    printf("------------------------------------------------------------------------------------------------\n");
}
